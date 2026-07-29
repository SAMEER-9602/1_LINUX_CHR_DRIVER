#include <linux/module.h>   // For Module_init & Module_exit
#include <linux/kernel.h>   // For Printk() 
#include <linux/fs.h>       // For dev_t alloc_chrdev_region, MAJOR, MINOR, MKDEV
#include <linux/cdev.h>     // cdev_init, cdev_add, cdev_del

#define DRIVER_NAME "my_cdev"

dev_t dev;
struct cdev my_cdev;

static int __init cdev_init_driver(void)
{
    int ret;

    /* Allocate Major and Minor number */
    ret = alloc_chrdev_region(&dev, 0, 1, DRIVER_NAME);

    if (ret < 0) 
    {
        printk(KERN_ERR "Failed to allocate device number\n");
        return ret;
    }

    printk(KERN_INFO "Major = %d\n", MAJOR(dev));
    printk(KERN_INFO "Minor = %d\n", MINOR(dev));

    /* Initialize cdev */
    cdev_init(&my_cdev, NULL);
    
    my_cdev.owner = THIS_MODULE;

    /* Add cdev to kernel */
    ret = cdev_add(&my_cdev, dev, 1);
    
    if (ret < 0) 
    {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "cdev_add failed\n");
        return ret;
    }

    printk(KERN_INFO "cdev added successfully\n");
    return 0;
}

static void __exit cdev_exit_driver(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Driver removed\n");
}

module_init(cdev_init_driver);
module_exit(cdev_exit_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOHD SAMI");
MODULE_DESCRIPTION("Basic cdev example");