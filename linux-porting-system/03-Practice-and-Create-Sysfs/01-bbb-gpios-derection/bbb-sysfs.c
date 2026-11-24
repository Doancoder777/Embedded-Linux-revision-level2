/*****************************************************************************
*  @brief      GPIO sysfs driver (integer-based)
*
*  @author     Sigma
*
*  What it does: create a folder /sys/bbb_gpios with one file "direction".
*  Reading the file calls sysfs_show(), writing it calls sysfs_store().
*  For now both functions only print a message to the kernel log (dmesg).
*****************************************************************************/

#include <linux/module.h>   /* This module defines macros such as module_init/module_exit */
#include <linux/fs.h>       /* This module defines functions such as allocate major/minor number */
#include <linux/device.h>   /* This module defines functions such as class_create/device_create */
#include <linux/slab.h>     /* This module defines functions such as kmalloc */
#include <linux/cdev.h>     /* This module defines functions such as cdev_init/cdev_add */
#include <linux/uaccess.h>  /* This module defines functions such as copy_to_user/copy_from_user */

#define DRIVER_AUTHOR "Sigma"
#define DRIVER_DESC   "GPIO sysfs driver (integer-based)"

/* Keep the kobject (the "folder" in sysfs) of our driver here */
struct m_foo_dev {
    struct kobject *kobj_ref;
} mdev;

/***************** Function Prototypes ******************/
static int  __init hello_world_init(void);
static void __exit hello_world_exit(void);

/***************** Sysfs functions ******************/
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

/* Create the sysfs file "direction" with permission 0660 (rw-rw----).
 * __ATTR(name, mode, show_function, store_function) */
struct kobj_attribute m_attr = __ATTR(direction, 0660, sysfs_show, sysfs_store);

/* This function will be called when we read the sysfs file
 * (example: cat /sys/bbb_gpios/direction) */
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    pr_info("Sysfs read.....\n");

    return 0;   /* return 0 = nothing to show (empty file) */
}

/* This function will be called when we write the sysfs file
 * (example: echo out > /sys/bbb_gpios/direction) */
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    pr_info("Sysfs write.....\n");

    return count;   /* return count = we accepted all the bytes written */
}

static int
__init hello_world_init(void)   /* Constructor */
{
    /* 01. It will create a directory under "/sys", [firmware_kobj, kernel_kobj]
     *     The 2nd parameter is the parent. NULL = put it directly under /sys */
    mdev.kobj_ref = kobject_create_and_add("bbb_gpios", NULL);

    /* 02. Creating sys entry under "/sys/bbb_gpios/" */
    if (sysfs_create_file(mdev.kobj_ref, &m_attr.attr)) {
        pr_err("Cannot create sysfs file......\n");
        goto rm_kobj;
    }

    pr_info("Hello! Initialize successfully!\n");
    return 0;

rm_kobj:
    kobject_put(mdev.kobj_ref);     /* undo step 01 if step 02 failed */
    return -1;
}

static void
__exit hello_world_exit(void)   /* Destructor */
{
    sysfs_remove_file(mdev.kobj_ref, &m_attr.attr); /* undo 02 */
    kobject_put(mdev.kobj_ref);                     /* undo 01 */

    pr_info("Good bye my fen !!!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
