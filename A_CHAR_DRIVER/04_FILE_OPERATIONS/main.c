#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

dev_t dev = 0;

static struct class *cls_ptr;
static struct device *dev_ptr;
static struct cdev chr_ptr;

/* Function Prototypes */
static int file_open(struct inode *inode, struct file *file);
static int file_close(struct inode *inode, struct file *file);
static ssize_t file_read(struct file *filp,char __user *buf,size_t len,loff_t *off);

static ssize_t file_write(struct file *filp,const char __user *buf,size_t len,loff_t *off);

/* File Operations Structure */
static struct file_operations file_ops =
{
    .owner   = THIS_MODULE,
    .open    = file_open,
    .read    = file_read,
    .write   = file_write,
    .release = file_close,
};

/*** This function will be called when we open the Device file*/
static int file_open(struct inode *inode, struct file *file)
{
    pr_info("Driver Open Function Called...!!!\n");
    return 0;
}

/*** This function will be called when we close the Device file*/
static int file_close(struct inode *inode, struct file *file)
{
    pr_info("Driver Release Function Called...!!!\n");
    return 0;
}

/*** This function will be called when we read the Device file*/
static ssize_t file_read(struct file *filp,char __user *buf,size_t len,loff_t *off)
{
    pr_info("Driver Read Function Called...!!!\n");
    return 0;
}

/*** This function will be called when we write the Device file*/
static ssize_t file_write(struct file *filp,const char __user *buf,size_t len,loff_t *off)
{
    pr_info("Driver Write Function Called...!!!\n");
    return len;
}

/*** Module Init Function*/
static int __init dum_drv_init(void)
{
    /* Allocate Major and Minor Number */
    if (alloc_chrdev_region(&dev, 0, 1, "FIRST") < 0)
    {
        pr_err("Failed to Allocate Major and Minor Number\n");
        return -1;
    }

    pr_info("Major = %d Minor = %d\n",
             MAJOR(dev), MINOR(dev));

    /* Create Device Class */
    cls_ptr = class_create("CLASS_FOLDER");

    if (IS_ERR(cls_ptr))
    {
        pr_err("Failed to Create Device Class\n");
        goto r_class;
    }

    pr_info("Class Created Successfully\n");

    /* Create Device File */
    dev_ptr = device_create(cls_ptr,NULL,dev,NULL,"DEVICE_FOLDER");

    if (IS_ERR(dev_ptr))
    {
        pr_err("Failed to Create Device File\n");
        goto r_device;
    }

    pr_info("Device File Created Successfully\n");

    /* Initialize CDEV Structure */
    cdev_init(&chr_ptr, &file_ops);

    /* Add Character Device to System */
    if (cdev_add(&chr_ptr, dev, 1) < 0)
    {
        pr_err("Cannot Add the Device to the System\n");
        goto r_cdev;
    }

    pr_info("Device Driver Inserted Successfully\n");

    return 0;

/* Error Handling */
r_cdev:
    device_destroy(cls_ptr, dev);

r_device:
    class_destroy(cls_ptr);

r_class:
    unregister_chrdev_region(dev, 1);

    return -1;
}

/*
** Module Exit Function
*/
static void __exit dum_drv_exit(void)
{
    cdev_del(&chr_ptr);

    device_destroy(cls_ptr, dev);

    class_destroy(cls_ptr);

    unregister_chrdev_region(dev, 1);

    pr_info("Device Driver Removed Successfully\n");
}

module_init(dum_drv_init);
module_exit(dum_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohd Sami");
MODULE_DESCRIPTION("Simple Linux Character Device Driver");
MODULE_VERSION("1.0");