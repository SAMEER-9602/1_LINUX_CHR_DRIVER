#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/timer.h>
#include<linux/wait.h>
#include<linux/poll.h>
#include<linux/proc_fs.h>
#include<linux/jiffies.h>


#define DEV_NAME        "MY_DEV"
#define PROC_FILE_NAME  "MY_PROC"
#define RW_DEV_NAME     "RW_DEVICE"

dev_t dev;
struct cdev etx_cdev;
struct cdev rw_cdev;
struct class *clsptr;
struct device *devptr;

static wait_queue_head_t wait;
struct timer_list timer;

struct proc_dir_entry *proc_entry;

int timer_data_available =0;

char timer_buffer[] = "Timer Event Occured after 5 Second\n";



int file_open(struct inode *inode, struct file *fileptr);
int file_close(struct inode *inode, struct file *fileptr);
ssize_t file_read(struct file *fileptr, char __user *buffer, size_t length, loff_t *offset);
ssize_t file_write(struct file *fileptr, const char __user *buffer, size_t length, loff_t *offset);


__poll_t poll_function(struct file *fileptr, poll_table *table);
void timer_callback(struct timer_list *timer);


void timer_callback(struct timer_list *timer)
{
    printk(KERN_INFO "Timer Expired : data is Now available\n");
    timer_data_available = 1;
    wake_up_interruptible(&wait);
}

int file_open(struct inode *inode, struct file *fileptr)
{
    pr_info("Device opened\n");
    return 0;
}

int file_close(struct inode *inode, struct file *fileptr)
{
    pr_info("Device closed\n");
    return 0;
}

ssize_t file_read(struct file *fileptr, char __user *buffer, size_t length, loff_t *offset)
{
    pr_info("Read function called\n");
    return 0;
}

ssize_t file_write(struct file *fileptr, const char __user *buffer, size_t length, loff_t *offset)
{
    pr_info("Write function called\n");
    return length;
}

__poll_t poll_function(struct file *fileptr, poll_table *table)
{
    pr_info("Poll function called\n");
    return 0;
}

ssize_t proc_read(struct file *fileptr, char __user *buffer, size_t length, loff_t *offset)
{
    return length;
}
struct file_operations fops = {
    .open       = file_open,
    .release    = file_close,
    .read       = file_read,
    .write      = file_write,
    .owner      = THIS_MODULE,
    .poll       = poll_function,
};
struct proc_ops proc_fops = {
    .proc_read = proc_read,
    // .proc_open = proc_open,
    // .proc_write= proc_write,
    // .proc_close= proc_close,
};

static int __init select_init(void)
{
    int ret=0;

    ret = alloc_chrdev_region(&dev, 0, 2, "DEV_NAME");
    if(ret<0)
    {
        printk(KERN_INFO "Failed to allocated Major Minor No:\n");
        return 0;
    }
    printk(KERN_INFO "Mjaor No is: %d\t Minor No is :%d\n",MAJOR(dev),MINOR(dev));
    return 0;

    cdev_init(&etx_cdev, &fops);

    ret = cdev_add(&etx_cdev,dev, 1);
    if(ret<0)
    {
        printk(KERN_INFO "Cannot ADD Cdev \n");
        goto delete_timer_cdev;   
    }

    cdev_init(&rw_cdev, &rw_fops);
    ret = cdev_add(&rw_cdev, dev, 1);
    if(ret<0)
    {
        pr_err("Cannot add read/write cdev\n");
        goto delete_timer_cdev;
    }
    clsptr = class_create("MY_CLASS");
    if(IS_ERR(clsptr))
    {
        printk(KERN_INFO "Cannot create class\n");
        return -1;
    }

    devptr = device_create(clsptr, NULL, dev, NULL, "MY_DEVICE");
    if(IS_ERR(devptr))
    {
        printk(KERN_INFO "Cannot create a device\n");
        return -1;
    }

    init_waitqueue_head(&wait);

    timer_setup(&timer,timer_callback, 0);

    mod_timer(&timer, jiffies+msecs_to_jiffies(5000));

    proc_entry = proc_create(PROC_FILE_NAME, 0666, NULL, &proc_fops);

    if(proc_entry)
    {
        printk(KERN_INFO "cannot create a Proc_Fs file\n");
        ret = -ENOMEM;
        goto destroy_rw_device;
    }
    printk(KERN_INFO "Select driver inserted successfully.\n");

    return 0;

destroy_rw_device:
    device_destroy(clsptr, dev, 1);

destroy_timer_device:
    device_destroy(clsptr, dev, 0);

destroy_class:
    class_destroy(clsptr);

delete_rw_cdev:
    cdev_del(&etx_cdev);

delete_timer_cdev:
    cdev_del(&timer_cdev);

unregister_region:
    unregister_chrdev_region(dev, 2);

    return ret;

}
static void __exit select_exit(void)
{
   timer_delete_sync(&timer);

    device_destroy(clsptr, dev);
    class_destroy(clsptr);

    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);

    pr_info("Poll driver removed\n");

}
module_init(select_init);
module_exit(select_exit);

MODULE_DESCRIPTION("SELECT PROGRAM");
MODULE_AUTHOR("MOHD SAMEER");
MODULE_LICENSE("GPL");