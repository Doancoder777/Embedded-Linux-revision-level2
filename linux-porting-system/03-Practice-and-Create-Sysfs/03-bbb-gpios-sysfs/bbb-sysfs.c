/*****************************************************************************
*  @brief      GPIO sysfs driver (integer-based) - control a real LED
*
*  @author     Sigma
*
*  What it does: create a folder /sys/bbb_gpios with TWO files:
*      /sys/bbb_gpios/direction   "in" or "out"   (read / write)
*      /sys/bbb_gpios/value       0 or 1          (read / write)
*  Writing "value" turns the LED on GPIO31 (P9_13) off or on.
*  The two files are registered together as one attribute group.
*
*  Test:
*      echo 1 > /sys/bbb_gpios/value      -> LED on
*      echo 0 > /sys/bbb_gpios/value      -> LED off
*      cat /sys/bbb_gpios/value
*****************************************************************************/

#include <linux/module.h>   /* This module defines macros such as module_init/module_exit */
#include <linux/fs.h>       /* This module defines functions such as allocate major/minor number */
#include <linux/device.h>   /* This module defines functions such as class_create/device_create */
#include <linux/slab.h>     /* This module defines functions such as kmalloc */
#include <linux/cdev.h>     /* This module defines functions such as cdev_init/cdev_add */
#include <linux/uaccess.h>  /* This module defines functions such as copy_to_user/copy_from_user */
#include <linux/gpio.h>     /* This module defines functions such as gpio_request/gpio_set_value */

#define DRIVER_AUTHOR "Sigma"
#define DRIVER_DESC   "GPIO sysfs driver (integer-based)"

/* GPIO number of the LED. 31 = GPIO0_31 = header P9_13 on BBB */
static unsigned int GPIO_LED = 31;

static int32_t _value = 0;      /* last value written: 0 or 1 */
static char _direct[8] = {};    /* last direction written: "in" or "out" */

/* Keep the kobject (the "folder" in sysfs) of our driver here */
struct m_foo_dev {
    struct kobject *kobj_ref;
} mdev;

/***************** Function Prototypes ******************/
static int  __init hello_world_init(void);
static void __exit hello_world_exit(void);

/***************** Sysfs functions ******************/
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);
static ssize_t direction_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t direction_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

/* One attribute (= one file in sysfs) per variable.
 * __ATTR(name, mode, show_function, store_function), 0660 = rw-rw---- */
struct kobj_attribute value = __ATTR(value, 0660, value_show, value_store);
struct kobj_attribute direction = __ATTR(direction, 0660, direction_show, direction_store);

/* Called when we read /sys/bbb_gpios/value: print the saved value */
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", _value);
}

/* Called when we write /sys/bbb_gpios/value: set the LED */
static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t numb = 0;
    sscanf(buf, "%d", &numb);       /* text -> number, e.g. "1\n" -> 1 */

    switch (numb) {
        case 0: /* off */
            gpio_set_value(GPIO_LED, 0);
            break;
        case 1: /* on */
            gpio_set_value(GPIO_LED, 1);
            break;

        default:                    /* any other number: ignore it */
            return count;
    }
    sscanf(buf, "%d", &_value);     /* remember the new value */

    return count;
}

/* Called when we read /sys/bbb_gpios/direction: print the saved direction */
static ssize_t direction_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", _direct);
}

/* Called when we write /sys/bbb_gpios/direction.
 * "echo in" sends "in\n" = 3 bytes, so (count - 1) = 2 = length of "in".
 * "echo out" sends "out\n" = 4 bytes, so (count - 1) = 3 = length of "out". */
static ssize_t direction_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    /* The GPIO calls inside the cases were added by me: the video code
     * only had the comments "in" / "out" at this point. */
    switch (count - 1) {
        case 2:
            /* in */
            if (strncmp(buf, "in", 2) != 0)
                return count;
            gpio_direction_input(GPIO_LED);
            strcpy(_direct, "in");
            break;
        case 3:
            /* out */
            if (strncmp(buf, "out", 3) != 0)
                return count;
            gpio_direction_output(GPIO_LED, _value);    /* keep the last value */
            strcpy(_direct, "out");
            break;
        default:
            return count;
    }

    return count;
}

/* Initialize group attrs: a NULL-terminated list of all the files in the group */
static struct attribute *attrs[] = {
    &direction.attr,
    &value.attr,
    NULL,   /* the list MUST end with NULL */
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int
__init hello_world_init(void)   /* Constructor */
{
    /* 01. It will create a directory under "/sys", [firmware_kobj, class_kobj]
     *     The 2nd parameter is the parent. NULL = put it directly under /sys */
    mdev.kobj_ref = kobject_create_and_add("bbb_gpios", NULL);

    /* 02. Creating group sys entry under "/sys/bbb_gpios/"
     *     One call creates both files: direction and value */
    if (sysfs_create_group(mdev.kobj_ref, &attr_group)) {
        pr_err("Cannot create group attributes......\n");
        goto rm_kobj;
    }

    /* 03. Request GPIO: claim the pin from the kernel, then set it as output, LED off.
     *     (The warning below was added by me, to see the reason in dmesg
     *      if the pin is already used by someone else.) */
    if (gpio_request(GPIO_LED, "led"))
        pr_warn("gpio_request(%u) failed, the pin may be in use\n", GPIO_LED);
    gpio_direction_output(GPIO_LED, 0);
    strcpy(_direct, "out");     /* so that "cat direction" shows the real state */

    pr_info("Hello! Initialize successfully!!\n");
    return 0;

rm_kobj:
    kobject_put(mdev.kobj_ref);     /* undo step 01 if step 02 failed */
    return -1;
}

static void
__exit hello_world_exit(void)   /* Destructor */
{
    gpio_free(GPIO_LED);                            /* 03 */
    sysfs_remove_group(mdev.kobj_ref, &attr_group); /* 02 */
    kobject_put(mdev.kobj_ref);                     /* 01 */

    pr_info("Good bye my fen !!!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
