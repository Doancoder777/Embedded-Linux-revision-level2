/******************************************************************************
*  @brief      Simple Linux device driver
*
*  @author     Sigma
*******************************************************************************/

#include <linux/module.h>  /* Thu vien nay dinh nghia cac macro nhu module_init/module_exit */
#include <linux/fs.h>      /* Thu vien nay dinh nghia cac ham allocate major/minor number */
#include <linux/device.h>  /* Thu vien nay dinh nghia cac ham class_create/device_create */
#include <linux/cdev.h>    /* Thu vien nay dinh nghia cac ham kmalloc */
#include<linux/slab.h>     /* Thu vien nay dinh nghia cac ham cdev_init/cdev_add */
#include<linux/uaccess.h>  /* Thu vien nay dinh nghia cac ham copy_to_user/copy_from_user */
#include "lib/led.h"
#include <linux/io.h>  /*This header define functions such as ioremap and iounmap*/

#define DRIVER_AUTHOR "Sigma"
#define DRIVER_DESC   "Hello world kernel module"

#define LED_ON _IOW('a', '1', int32_t *)
#define LED_OFF _IOW('a', '2', int32_t *)

#define NPAGES  1

static int32_t value = 0;
uint32_t __iomem *base_addr;  /* pointer to GPIO0 register block */


struct m_foo_dev {
    int size;
    char *kmalloc_ptr;
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
} mdev;

/*  Function Prototypes */
static long     m_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .unlocked_ioctl = m_ioctl,
};


static long m_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case LED_ON:
        /* Register GPIO_SETDATAOUT (base + 0x194), a DIFFERENT register: write 1 to bit 31 -> pin goes high, LED on */
            *(base_addr + GPIO0_SET_DATAOUT_OFFSET / 4) |= LED0_PIN;
            pr_info("Turn on Led\n");
            break;

        case LED_OFF:
            *(base_addr + GPIO0_CLEAR_DATAOUT_OFFSET / 4) |= LED0_PIN;  /* turn off LED0 */
            pr_info("Turn off Led\n");
            break;

        default:
                pr_info("Default\n");
                break;
    }

    return 0;
}



static int 
__init hello_world_init(void)   /* Constructor */
{   
    /* 1. Allocating device number (cat /pro/devices)*/
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "m-cdev") < 0) {
	    pr_err("Failed to alloc chrdev region\n");
	    return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    /* 02.1 Creating cdev structure */
    cdev_init(&mdev.m_cdev, &fops);

    /* 02.2 Adding character device to the system*/
    if ((cdev_add(&mdev.m_cdev, mdev.dev_num, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto rm_device_numb;
    }

    /* 03. Creating struct class */
    if ((mdev.m_class = class_create(THIS_MODULE,"m_class")) == NULL) {
        pr_err("Cannot create the struct class for my device\n");
        goto rm_device_numb;
    }

    /* 04. Creating device*/
    if ((device_create(mdev.m_class, NULL, mdev.dev_num, NULL, "m_device")) == NULL) {
        pr_err("Cannot create my device\n");
        goto rm_class;
    }

    /* 05. Creating Physical memory*/
    if((mdev.kmalloc_ptr = kmalloc(1024 , GFP_KERNEL)) == 0){
        pr_err("Cannot allocate memory in kernel\n");
        goto rm_device;
    }


    base_addr = ioremap(GPIO_0_ADDRESS_BASE, GPIO_0_ADDR_SIZE);
    if (!base_addr) {
        pr_err("Failed to map GPIO0 register block\n");
        return -ENOMEM;
    }

    /* Each register is 32 bits wide (bit 0..31), each bit = one GPIO0 pin */
    /* Register GPIO_OE (base + 0x134): clear bit 31 -> GPIO0_31 is output */
    *(base_addr + GPIO0_OE0_OFFSET / 4) &= ~LED0_PIN;
    /*
    * Device offset by 4 because each integer register is 4 bytes.
    * (base + 1) = base + 0x04, so a byte offset used directly is wrong.
    * Divide by 4 to get the correct index in the array of registers.
    * Example: 0x134 / 4 = 77 -> (base + 77) = base + 0x134
    * Wrong:   (base + 0x134) = base + 0x4D0 (308 * 4 = 1232 bytes),
    *          which is NOT the GPIO_OE register.
    */

    printk("LED blink driver initialized\n");

    pr_info("Hello world kernel module\n");
    return 0;

rm_device:
    device_destroy(mdev.m_class, mdev.dev_num);
rm_class:
    class_destroy(mdev.m_class);
rm_device_numb:
    unregister_chrdev_region(mdev.dev_num, 1);
    return -1;
}

static void 
__exit hello_world_exit(void)   /* Destructor */
{
    kfree(mdev.kmalloc_ptr);                        /* 05 */
    device_destroy(mdev.m_class, mdev.dev_num);     /* 04 */
    class_destroy(mdev.m_class);                    /* 03 */
    cdev_del(&mdev.m_cdev);                         /* 02 */
    unregister_chrdev_region(mdev.dev_num, 1);      /* 01 */

    iounmap(base_addr); //
    printk("LED blink driver exited\n");

    pr_info("Goodbye\n");;
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);