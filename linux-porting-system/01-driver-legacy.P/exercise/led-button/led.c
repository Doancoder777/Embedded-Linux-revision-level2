/* LED + button driver for BeagleBone Black (AM335x), GPIO0
 * Press the button (GPIO0_26) -> toggle the LED (GPIO0_31).
 * Button is ACTIVE LOW (pressed = 0): wire it between the pin and GND, with a
 * pull-up resistor to 3.3V (or use the internal pull-up in pinmux) so the pin
 * reads 1 when released.
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include "lib/led.h"

#define DRIVER_AUTHOR  "Sigma"
#define DRIVER_DESC    "LED + button driver for BeagleBone Black (AM335x), GPIO0"
#define DRIVER_VERSION "1.1"

static void __iomem *base_addr;   /* pointer to GPIO0 register block */
static struct timer_list poll_timer;
static bool last_pressed;         /* previous button state, for edge detect */

/* Read a 32-bit register; offset is in BYTES from base_addr.
 * Returns the register's content, not its address. */
static inline u32 gpio_read(u32 offset)
{
    return ioread32(base_addr + offset);
}

/* Write val to the 32-bit register at base_addr + offset. */
static inline void gpio_write(u32 offset, u32 val)
{
    iowrite32(val, base_addr + offset);
}

/* Write 1 to bit 31 of SETDATAOUT: pin goes high, other pins untouched. */
static inline void led_on(void)  { gpio_write(GPIO0_SET_DATAOUT_OFFSET, LED0_PIN); }

/* Write 1 to bit 31 of CLEARDATAOUT: pin goes low, other pins untouched. */
static inline void led_off(void) { gpio_write(GPIO0_CLEAR_DATAOUT_OFFSET, LED0_PIN); }

/* DATAOUT stores the output value; mask keeps only bit 31. Non-zero = on. */
static inline bool led_is_on(void)
{
    return gpio_read(GPIO0_DATAOUT_OFFSET) & LED0_PIN;
}

/* DATAIN is the real pin level; mask keeps only bit 26.
 * Active low (pull-up): released = 1, pressed = 0, so invert with !. */
static inline bool button_pressed(void)
{
    return !(gpio_read(GPIO0_DATAIN_OFFSET) & BUTTON_PIN);
}


/* Runs every POLL_PERIOD_MS. Toggle LED only on the "not pressed -> pressed" edge. */
static void poll_timer_fn(struct timer_list *t)
{
    bool pressed = button_pressed();

    if (pressed && !last_pressed) {
        if (led_is_on())
            led_off();
        else
            led_on();
        pr_info("button pressed, LED is now %s\n", led_is_on() ? "ON" : "OFF");
    }
    last_pressed = pressed;

    mod_timer(&poll_timer, jiffies + msecs_to_jiffies(POLL_PERIOD_MS));
    /* Re-arm the one-shot timer: call this function again after POLL_PERIOD_MS.
     * Without this line the button would be read only once.
     * The loop is stopped by del_timer_sync() in led_exit(). */
}
/*
 * Edge detect: condition = pressed && !last_pressed
 *
 * pressed | last_pressed | condition | Meaning        | LED
 * --------+--------------+-----------+----------------+----------
 *    0    |      0       |     0     | released       | no change
 *    1    |      0       |     1     | just pressed   | TOGGLE
 *    1    |      1       |     0     | held down      | no change
 *    0    |      1       |     0     | just released  | no change
 */


/*Constructor*/
static int __init led_init(void)
{
    u32 val;

    base_addr = ioremap(GPIO_0_ADDRESS_BASE, GPIO_0_ADDR_SIZE);
    if (!base_addr) {
        pr_err("Failed to map GPIO0 register block\n");
        return -ENOMEM;
    }

    /* GPIO_OE: bit = 0 -> output, bit = 1 -> input.
     * Read-modify-write so other pins are untouched. */
    val = gpio_read(GPIO0_OE0_OFFSET);
    val &= ~LED0_PIN;     /* GPIO0_31 = output */
    val |= BUTTON_PIN;    /* GPIO0_26 = input  */
    gpio_write(GPIO0_OE0_OFFSET, val);

    led_off();

    last_pressed = button_pressed();

    /* timer_setup: bind the callback (poll_timer_fn) to the timer. Done once.
     * It does NOT start the timer. */
    timer_setup(&poll_timer, poll_timer_fn, 0);

    /* Kick-start: arm the timer for the FIRST time.
     * Without this the timer never fires, so poll_timer_fn never runs
     * and the re-arm inside it is never reached. After this first shot,
     * poll_timer_fn keeps itself running by re-arming. */
    mod_timer(&poll_timer, jiffies + msecs_to_jiffies(POLL_PERIOD_MS));
    pr_info("LED + button driver initialized\n");
    return 0;
}

/*Destructor*/
static void __exit led_exit(void)
{
    del_timer_sync(&poll_timer);  /* stop polling before unmapping! */
    led_off();                    /* write 1 directly, do NOT use |= */
    iounmap(base_addr);
    pr_info("LED + button driver exited\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);