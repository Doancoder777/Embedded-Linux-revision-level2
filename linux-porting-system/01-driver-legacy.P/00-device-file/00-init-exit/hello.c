/*linux kernel module*/
#include<linux/module.h>

#define DRIVER_AUTHOR "sigma"
#define DRIVER_DESC "hello world kernel module"
#define DRIVER_VERS "1.0"

//constructor
static int __init hello_world_init(void){
    printk(KERN_INFO "Hello world kernel module\n");
    return 0;
}


//destructor
static void __exit hello_world_exit(void){
    printk(KERN_INFO "Good bye");
}

module_init(hello_world_init);
module_exit(hello_world_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERS);