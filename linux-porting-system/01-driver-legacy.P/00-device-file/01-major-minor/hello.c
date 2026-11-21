#include<linux/module.h> /*Thu vien nay dinh nghia cac macro nhu module_init va module_exit*/
#include<linux/fs.h>  /*Thu vien nay dinh nghia cac ham allocate major va minor number */


#define DRIVER_AUTHOR "sigma"
#define DRIVER_DESC "hello world kernel moudle"


struct  m_foo_dev
{
    dev_t dev_num; /*dev_t = MKDEV(17,0)*/
}mdev;


static int __init hello_world_init(void){

    /*1.1 Dynamic allocating device number (cat /proc/device)*/
    if(alloc_chrdev_region(&mdev.dev_num,0,1,"m-cdev")<0){
        pr_err("Failled to alloc chrdev region\n");
    }


    /*1.2 Static allocating device number (cat /proc/device)*/
    // register_chrdev_retion(&mdev.deev_num,1,"m-cdev");
    pr_info("Major = %d Minor = %d\n",MAJOR(mdev.dev_num),MINOR(mdev.dev_num));
    return 0;
}



static void __exit hello_world_exit(void){
    unregister_chrdev_region(mdev.dev_num,1);
    pr_info("Goodbye\n");
}



module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);