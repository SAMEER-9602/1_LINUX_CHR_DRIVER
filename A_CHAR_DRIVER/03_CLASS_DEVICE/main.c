#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DRIVER_NAME "MY_DRIVER"
#define CLASS_NAME  "OWN_CLASS"
#define DEVICE_NAME "OWN_DEVICE"

dev_t dev;
struct cdev my_cdev;

struct class *clsptr;
struct device *devptr;

/* Module Init */
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
        return ret;
    }


    /* Create device class */
    clsptr = class_create(CLASS_NAME);

    if (IS_ERR(clsptr))
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(clsptr);
    }

    printk(KERN_INFO "Class Created..\n");
    /* Create device file */
    devptr = device_create(clsptr, NULL, dev, NULL, DEVICE_NAME);

    if (IS_ERR(devptr))
    {
        class_destroy(clsptr);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(devptr);
    }
    printk(KERN_INFO "Device created: /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "Driver Loaded Successfully.\n");
    return 0;
}


/* Module Exit */
static void __exit cdev_exit_driver(void)
{
    device_destroy(clsptr, dev);
    class_destroy(clsptr);

    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Driver removed\n");
}


module_init(cdev_init_driver);
module_exit(cdev_exit_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sameer");
MODULE_DESCRIPTION("Basic cdev driver with class and device creation");
