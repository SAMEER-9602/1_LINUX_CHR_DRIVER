#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define DEVICE_NAME "wait_queue"

 dev_t dev;
 struct cdev etx_cdev;
 struct class *class_ptr;

/* Declearation of Wait queue Variable */
 wait_queue_head_t wait_queue;

/* Wait condition */
 int data_available = 0;

/* Kernel buffer */
 char kernel_buffer[100];

/*Functions Declearations*/
ssize_t driver_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset);
int driver_open(struct inode *inode, struct file *file);
ssize_t driver_write(struct file *file, const char __user *user_buffer, size_t leng, loff_t *offset);
int driver_release(struct inode *inode, struct file *file);
int __init wait_queue_init(void);
void __exit wait_queue_exit(void);

/* Open function */
 int driver_open(struct inode *inode, struct file *file)
{
    pr_info("Device opened\n");

    return 0;
}

/* Read function */
ssize_t driver_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset)
{
    int ret;

    pr_info("Read function called\n");

    /*
     * If data_available is 0, the process sleeps.
     * It wakes up when data_available becomes 1.
     */
    ret = wait_event_interruptible(wait_queue, data_available == 1);

    if (ret)
        return ret;

    /* Copy data from kernel space to user space.   */
    if (copy_to_user(user_buffer, kernel_buffer, len))
        return -EFAULT;

    /* Data has been read. Make the condition false again.  */
    data_available = 0;

    pr_info("Data sent to user space\n");

    return len;
}

/* Write function */
ssize_t driver_write(struct file *file, const char __user *user_buffer, size_t leng, loff_t *offset)
{
    if (leng >= sizeof(kernel_buffer))
        leng = sizeof(kernel_buffer) - 1;

    /* Copy data from user space to kernel space. */
    if (copy_from_user(kernel_buffer, user_buffer, leng))
        return -EFAULT;

    kernel_buffer[leng] = '\0';

    pr_info("Data received: %s\n", kernel_buffer);

    /* Change the wait condition.*/
    data_available = 1;

    /*
     * Wake up the process waiting on wait_queue.
     */
    wake_up_interruptible(&wait_queue);

    return leng;
}

/* Release function */
int driver_release(struct inode *inode, struct file *file)
{
    pr_info("Device closed\n");

    return 0;
}

/* File operations */
 struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .read = driver_read,
    .write = driver_write,
    .release = driver_release,
};

/* Driver initialization */
int __init wait_queue_init(void)
{
    int ret;

    /* Initialize the wait queue.*/
    init_waitqueue_head(&wait_queue);

    /** Allocate one major/minor device number.*/
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);

    if (ret < 0) 
    {
        pr_err("Failed to allocate device number\n");
        return ret;
    }

    /** Initialize and add cdev.*/
    cdev_init(&etx_cdev, &fops);

    ret = cdev_add(&etx_cdev, dev, 1);

    if (ret < 0) 
    {
        pr_err("Failed to add cdev\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    /** Create device class.*/
    class_ptr = class_create(DEVICE_NAME);

    if (IS_ERR(class_ptr)) 
    {
        pr_err("Failed to create class\n");
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(class_ptr);
    }

    /** Create /dev/wait_queue.*/
    if (IS_ERR(device_create(class_ptr, NULL, dev, NULL, DEVICE_NAME)))
    {
        pr_err("Failed to create device\n");
        class_destroy(class_ptr);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    pr_info("Wait queue driver loaded\n");

    return 0;
}

/* Driver exit */
void __exit wait_queue_exit(void)
{
    device_destroy(class_ptr, dev);

    class_destroy(class_ptr);

    cdev_del(&etx_cdev);

    unregister_chrdev_region(dev, 1);

    pr_info("Wait queue driver removed\n");
}

module_init(wait_queue_init);
module_exit(wait_queue_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOHD SAMEER");
MODULE_DESCRIPTION("Simple Wait Queue Driver");