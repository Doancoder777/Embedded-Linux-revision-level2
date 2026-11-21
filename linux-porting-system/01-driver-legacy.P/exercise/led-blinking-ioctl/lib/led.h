/* LED blink driver for BeagleBone Black (AM335x), GPIO0 */

#ifndef _LED_MODULE_H_
#define _LED_MODULE_H_

/* GPIO0 register block: base address and size (4 KB) */
#define GPIO_0_ADDRESS_BASE 0x44E07000
#define GPIO_0_ADDR_SIZE    (0x44E07FFF - 0x44E07000 + 1)  /* end - start + 1 = 0x1000 */

/* Register offsets (address = base + offset) */
#define GPIO0_OE0_OFFSET            0x134  /* Output Enable: bit 0 = output, 1 = input */
#define GPIO0_CLEAR_DATAOUT_OFFSET  0x190  /* write 1 -> pin LOW (LED off), 0 = no effect */
#define GPIO0_SET_DATAOUT_OFFSET    0x194  /* write 1 -> pin HIGH (LED on), 0 = no effect */

/* LED on P9_13 = GPIO0_31 -> bit 31 mask (0x80000000) */
#define LED0_PIN (1U << 31)

#endif /* _LED_MODULE_H_ */