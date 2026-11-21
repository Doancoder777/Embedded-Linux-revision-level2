/* LED blink driver for BeagleBone Black (AM335x), GPIO0 */
#include <linux/module.h> /*This module define functions such as module_init and module_exit*/
#include <linux/io.h>  /*This header define functions such as ioremap and iounmap*/
#include "led.h"

#define DRIVER_AUTHOR "Sigma"
#define DRIVER_DESC   "LED blink driver for BeagleBone Black (AM335x), GPIO0"
#define DRIVER_VERSION "1.0"

uint32_t __iomem *base_addr;  /* pointer to GPIO0 register block */


/*Constructor*/
static int __init led_init(void)
{
    base_addr = ioremap(GPIO_0_ADDRESS_BASE, GPIO_0_ADDR_SIZE);
    if (!base_addr) {
        pr_err("Failed to map GPIO0 register block\n");
        return -ENOMEM;
    }

    /* Each register is 32 bits wide (bit 0..31), each bit = one GPIO0 pin */
    /* Register GPIO_OE (base + 0x134): clear bit 31 -> GPIO0_31 is output */
    *(base_addr + GPIO0_OE0_OFFSET / 4) &= ~LED0_PIN;

    /* Register GPIO_SETDATAOUT (base + 0x194), a DIFFERENT register: write 1 to bit 31 -> pin goes high, LED on */
    *(base_addr + GPIO0_SET_DATAOUT_OFFSET / 4) |= LED0_PIN;

    /*
    * Device offset by 4 because each integer register is 4 bytes.
    * (base + 1) = base + 0x04, so a byte offset used directly is wrong.
    * Divide by 4 to get the correct index in the array of registers.
    * Example: 0x134 / 4 = 77 -> (base + 77) = base + 0x134
    * Wrong:   (base + 0x134) = base + 0x4D0 (308 * 4 = 1232 bytes),
    *          which is NOT the GPIO_OE register.
    */

    printk("LED blink driver initialized\n");
    return 0;
}

/*Destructor*/
static void __exit led_exit(void){
    *(base_addr + GPIO0_CLEAR_DATAOUT_OFFSET / 4) |= LED0_PIN;  /* turn off LED0 */
    iounmap(base_addr); //
    printk("LED blink driver exited\n");
}


module_init(led_init);
module_exit(led_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
