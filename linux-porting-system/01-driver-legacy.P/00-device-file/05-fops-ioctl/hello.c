/******************************************************************************
*  @brief      Simple Linux device driver
*
*  @author     Phonglt9 - phongxxxxx@gmail.com
*******************************************************************************/

#include <linux/module.h>  /* Thu vien nay dinh nghia cac macro nhu module_init/module_exit */
#include <linux/fs.h>      /* Thu vien nay dinh nghia cac ham allocate major/minor number */
#include <linux/device.h>  /* Thu vien nay dinh nghia cac ham class_create/device_create */
#include <linux/cdev.h>    /* Thu vien nay dinh nghia cac ham kmalloc */
#include<linux/slab.h>     /* Thu vien nay dinh nghia cac ham cdev_init/cdev_add */
#include<linux/uaccess.h>  /* Thu vien nay dinh nghia cac ham copy_to_user/copy_from_user */

#define DRIVER_AUTHOR "Sigma"
#define DRIVER_DESC   "Hello world kernel module"

#define LED_ON _IOW('a', '1', int32_t *)
#define LED_OFF _IOW('a', '2', int32_t *)

#define NPAGES  1

static int32_t value = 0;

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
            pr_info("Turn on Led\n");
            break;

        case LED_OFF:
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
    if ((mdev.m_class = class_create("m_class")) == NULL) {
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

    pr_info("Goodbye\n");;
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);  