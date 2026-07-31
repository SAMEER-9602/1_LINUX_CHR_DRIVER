#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/poll.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/timer.h>
#include<linux/jiffies.h>

#define DEV_NAME "poll_dev"

dev_t dev;
struct cdev etx_cdev;
struct class *clsptr;
struct device *devptr;

wait_queue_head_t my_queue; 
struct timer_list timer;

int data = 0;
char kernel_buffer[] = "Hello From Poll Driver\n";

ssize_t poll_read(struct file *fileptr, char __user *buff, size_t len, loff_t *offset);
void timer_callback(struct timer_list *timer);
int poll_open(struct inode *inode, struct file *fileptr);
int poll_close(struct inode *inode, struct file *fileptr);

void timer_callback(struct timer_list *timer)
{
    printk(KERN_INFO "Timer Expired: Data is Available\n");
    data = 1;
    wake_up_interruptible(&my_queue);
}
int poll_open(struct inode *inode, struct file *fileptr)
{
    printk(KERN_INFO "Device Opened\n");
    data= 0;
    mod_timer(&timer, jiffies + msecs_to_jiffies(5000));
    pr_info("Timer started for 5 seconds\n");

    return 0;
    return 0;
}
int poll_close(struct inode *inode, struct file *fileptr)
{
    printk(KERN_INFO "Device Closed\n");
    return 0;
}
ssize_t poll_read(struct file *fileptr, char __user *buff, size_t len, loff_t *offset)
{
    int data_length = strlen(kernel_buffer);
    if(!data)
        return -EAGAIN;
    
    if(*offset >= data_length)
        return 0;
    
    if(len>data_length-*offset) //user ask 50, but kernel_buff = 30. then 50>30 true
        len = data_length-*offset; //making length (is equal to kernel_buffer length)

    if(copy_to_user(buff, kernel_buffer+*offset, len))
        return -EFAULT;
    
    *offset += len;
    printk("Data Copied to user space\n");
    return len;
}
static __poll_t my_poll_function(struct file *fileptr, poll_table *table)
{
    __poll_t mask = 0;
    printk(KERN_INFO "Driver Poll() function called..\n");

    poll_wait(fileptr, &my_queue, table); //Registering the user process with wait queue.

    if(data)
    {
        mask |= POLLIN | POLLRDNORM;
    }
    return mask; //if data available it is return to user;
}

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .open = poll_open,
    .release = poll_close,
    .read = poll_read,
    .poll = my_poll_function,
};

static int __init poll_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&dev, 0, 1, DEV_NAME);

    if (ret < 0)
    {
        pr_err("Cannot allocate major number\n");
        return ret;
    }

    pr_info("Major = %d, Minor = %d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&etx_cdev, &fops);

    ret = cdev_add(&etx_cdev, dev, 1);

    if (ret < 0)
    {
        pr_err("Cannot add cdev\n");
        goto unregister_device;
    }

    clsptr = class_create("poll_class");

    if (IS_ERR(clsptr))
    {
        pr_err("Cannot create class\n");
        ret = PTR_ERR(clsptr);
        goto delete_cdev;
    }

    if (IS_ERR(device_create(clsptr, NULL, dev, NULL, DEV_NAME)))
    {
        pr_err("Cannot create device\n");
        ret = -1;
        goto destroy_class;
    }    

    /*..Initialization Wait Queue...*/
    init_waitqueue_head(&my_queue); 

    /*Initialization timer*/
    timer_setup(&timer,timer_callback,0);

    printk(KERN_INFO "Poll Driver Inserted successfully\n");

    return 0;

destroy_class:
    class_destroy(clsptr);

delete_cdev:
    cdev_del(&etx_cdev);

unregister_device:
    unregister_chrdev_region(dev, 1);

    return ret;
}

/* Module exit */
static void __exit poll_exit(void)
{
    timer_delete_sync(&timer);

    device_destroy(clsptr, dev);
    class_destroy(clsptr);

    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);

    pr_info("Poll driver removed\n");
}

module_init(poll_init);
module_exit(poll_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sameer");
MODULE_DESCRIPTION("Simple poll driver using wait queue and timer");
MODULE_VERSION("1.0");

