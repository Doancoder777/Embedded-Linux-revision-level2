#include <linux/module.h>  /* Defines macros such as module_init and module_exit */
#include <linux/fs.h>      /* Defines functions to allocate major and minor numbers */
#include <linux/device.h>  /* Defines class_create, class_destroy, device_create, device_destroy */


#define DRIVER_AUTHOR "sigma"
#define DRIVER_DESC "hello world kernel moudle"


struct  m_foo_dev
{
    dev_t dev_num; /*dev_t = MKDEV(17,0)*/
    struct class *m_class;
}mdev;


static int __init hello_world_init(void){

    /*1 Dynamic allocating device number (cat /proc/device)*/
    if(alloc_chrdev_region(&mdev.dev_num,0,1,"m-cdev")<0){
        pr_err("Failled to alloc chrdev region\n");
    }

    pr_info("Major = %d Minor = %d\n",MAJOR(mdev.dev_num),MINOR(mdev.dev_num));

    /*2 Create struct class */
    if((mdev.m_class = class_create("m_class"))==NULL){
        pr_err("Create class falled\n");
        goto rm_device_numb;
    }

    /*create device*/
    if(device_create(mdev.m_class,NULL,mdev.dev_num,NULL,"m_device")==NULL){
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
    device_destroy(mdev.m_class,mdev.dev_num);   //03
    class_destroy(mdev.m_class);    //02
    unregister_chrdev_region(mdev.dev_num,1);   //01

    pr_info("Goodbye\n");
}



module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);