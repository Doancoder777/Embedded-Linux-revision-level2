/* LED + button driver for BeagleBone Black (AM335x), GPIO0 */

#ifndef _LED_MODULE_H_
#define _LED_MODULE_H_

/* GPIO0 register block: base address and size (4 KB) */
#define GPIO_0_ADDRESS_BASE 0x44E07000
#define GPIO_0_ADDR_SIZE    0x1000

/* Register offsets (address = base + offset) */
#define GPIO0_OE0_OFFSET            0x134  /* Output Enable: bit 0 = output, 1 = input */
#define GPIO0_DATAIN_OFFSET         0x138  /* Read pin logic level */
#define GPIO0_DATAOUT_OFFSET        0x13C  /* Current output value */
#define GPIO0_CLEAR_DATAOUT_OFFSET  0x190  /* write 1 -> pin LOW,  0 = no effect */
#define GPIO0_SET_DATAOUT_OFFSET    0x194  /* write 1 -> pin HIGH, 0 = no effect */

/* LED on P9_13 = GPIO0_31, button on P8_14 = GPIO0_26 */
#define LED0_PIN   (1U << 31)
#define BUTTON_PIN (1U << 26)

/* Polling period (ms). Also acts as debounce. */
#define POLL_PERIOD_MS 20

#endif /* _LED_MODULE_H_ */ 