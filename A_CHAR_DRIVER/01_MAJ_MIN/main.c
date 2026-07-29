#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DRIVER_NAME "maj_min_driver"

dev_t dev;

static int __init maj_min_init(void)
{
    int Maj_no, Min_no;

    /* Allocate Major and Minor number dynamically */
    alloc_chrdev_region(&dev, 0, 1, DRIVER_NAME);

    Maj_no = MAJOR(dev);
    Min_no = MINOR(dev);

    printk(KERN_INFO "Major Number = %d\n", Maj_no);
    printk(KERN_INFO "Minor Number = %d\n", Min_no);
    printk(KERN_INFO "Major Minor Driver Loaded\n");
    return 0;
}

static void __exit maj_min_exit(void)
{
    /* Release Major and Minor number */
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Major Minor Driver Removed\n");
}


module_init(maj_min_init);
module_exit(maj_min_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOHD_SAMI");
MODULE_DESCRIPTION("Simple Major Minor Number Driver");
