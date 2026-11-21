#include <linux/module.h>  /* Defines macros such as module_init and module_exit */
#include <linux/fs.h>      /* Defines functions to allocate major and minor numbers */
#include <linux/device.h>  /* Defines class_create, class_destroy, device_create, device_destroy */
#include <linux/cdev.h>    /* Defines struct cdev, cdev_init, cdev_add, cdev_del */


#define DRIVER_AUTHOR "sigma"
#define DRIVER_DESC "hello world kernel moudle"


struct  m_foo_dev
{
    dev_t dev_num; /*dev_t = MKDEV(17,0)*/
    struct class *m_class;
    struct cdev m_cdev;
}mdev;


/*  Function Prototypes */
static int      __init hello_world_init(void);
static void     __exit hello_world_exit(void);
static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *user_buf, size_t size,loff_t * offset);
static ssize_t  m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset);

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .read = m_read,
    .write = m_write,
    .open = m_open,
    .release = m_release
};


/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    pr_info("System call open() called...!!!\n");
    return 0;
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    pr_info("System call close() called...!!!\n");
    return 0;
}

/* This function will be called when we read the Device file */
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
    pr_info("System call read() called...!!!\n");
    return 0;
}

/* This function will be called when we write the Device file */
static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size, loff_t *offset)
{
    pr_info("System call write() called...!!!\n");
    return size;
}


static int __init hello_world_init(void){

    /*1 Dynamic allocating device number (cat /proc/device)*/
    if(alloc_chrdev_region(&mdev.dev_num,0,1,"m-cdev")<0){
        pr_err("Failled to alloc chrdev region\n");
    }

    pr_info("Major = %d Minor = %d\n",MAJOR(mdev.dev_num),MINOR(mdev.dev_num));

    /*2 Create struct class */
    if((mdev.m_class = class_create("m_class2"))==NULL){
        pr_err("Create class falled\n");
        goto rm_device_numb;
    }

    /*2.1 Create cdev struct*/
    cdev_init(&mdev.m_cdev,&fops);

    /*2.2 Adding character device to the system*/
    if((cdev_add(&mdev.m_cdev,mdev.dev_num,1))<0){
        pr_err("Cannot add the device to the system\n");
        goto rm_device_numb;
    }

    /*create device*/
    if((device_create(mdev.m_class,NULL,mdev.dev_num,NULL,"m_device"))==NULL){
        pr_err("Create device failed");
        goto rm_class;
    }

    rm_class: 
        class_destroy(mdev.m_class);
    rm_device_numb:
        unregister_chrdev_region(mdev.dev_num,1);

    return 0;
}





static void __exit hello_world_exit(void){
    device_destroy(mdev.m_class,mdev.dev_num);  //04
    class_destroy(mdev.m_class);                //03
    cdev_del(&mdev.m_cdev);                     //02
    unregister_chrdev_region(mdev.dev_num,1);   //01

    pr_info("Goodbye\n");
}



module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);