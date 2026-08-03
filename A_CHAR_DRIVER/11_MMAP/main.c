#include <linux/module.h>      // Module macros and module information
#include <linux/init.h>        // __init and __exit macros
#include <linux/kernel.h>      // pr_info() and pr_err()
#include <linux/fs.h>          // File operations and device functions
#include <linux/cdev.h>        // Character device functions
#include <linux/device.h>      // Device class and device creation
#include <linux/mm.h>          // mmap(), PAGE_SIZE, and memory structures
#include <linux/ioctl.h>       // ioctl command macros
#include <linux/string.h>      // Kernel string functions
#include <linux/io.h>

#define DEVICE_NAME "mmap_ioctl_device"    // Name of the device file
#define CLASS_NAME "mmap_ioctl_class"      // Name of the device class

#define BUFFER_SIZE PAGE_SIZE               // Use one complete memory page

#define MMAP_IOCTL_MAGIC 'M'                // Magic number for ioctl commands
#define MMAP_TX_READY _IO(MMAP_IOCTL_MAGIC, 1) // User notifies driver that TX data is ready

static dev_t dev_num;                       // Stores major and minor device numbers
static struct cdev mmap_cdev;               // Character device structure
static struct class *mmap_class;            // Pointer to the device class

static char *kernel_buffer;                 // One-page kernel buffer mapped to user space


/* Called when the user application opens the device. */
static int mmap_open(struct inode *inode, struct file *file)
{
    pr_info("MMAP_IOCTL: Device opened\n"); // Print device-open message

    return 0;                               // Return success
}


/* Called when the user application closes the device. */
static int mmap_release(struct inode *inode, struct file *file)
{
    pr_info("MMAP_IOCTL: Device closed\n"); // Print device-close message

    return 0;                               // Return success
}


/* Maps the kernel memory page into user-space virtual memory. */
static int mmap_driver_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long requested_size;           // Stores requested mapping size
    unsigned long pfn;                      // Stores physical page frame number
    int ret;                                // Stores function return value

    requested_size = vma->vm_end - vma->vm_start; // Calculate requested mmap size

    if (requested_size > BUFFER_SIZE)       // Check whether the request exceeds one page
    {
        pr_err("MMAP_IOCTL: Requested size is too large\n");

        return -EINVAL;                     // Return invalid-argument error
    }

    pfn = virt_to_phys(kernel_buffer) >> PAGE_SHIFT; // Convert kernel address to PFN

    ret = remap_pfn_range(vma, vma->vm_start, pfn, requested_size, vma->vm_page_prot); // Map page into user space

    if (ret)                                // Check whether memory mapping failed
    {
        pr_err("MMAP_IOCTL: remap_pfn_range failed\n");

        return ret;                         // Return the mapping error
    }

    pr_info("MMAP_IOCTL: Kernel buffer mapped to user space\n");

    return 0;                               // Return successful mapping
}


/* Handles ioctl commands sent by the user application. */
static long mmap_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)                            // Check which ioctl command was received
    {
        case MMAP_TX_READY:

            pr_info("MMAP_IOCTL: TX data received from user application\n");

            pr_info("MMAP_IOCTL: TX DATA START\n");

            pr_info("%s", kernel_buffer);  // Print data written through mmap()

            pr_info("MMAP_IOCTL: TX DATA END\n");

            break;                         // Exit the switch statement

        default:

            pr_err("MMAP_IOCTL: Invalid ioctl command\n");

            return -EINVAL;                // Return error for an unknown command
    }

    return 0;                               // Return ioctl success
}


/* Connects user-space system calls with driver functions. */
static const struct file_operations fops =
{
    .owner = THIS_MODULE,                   // Prevent module removal while in use
    .open = mmap_open,                      // Connect open() to mmap_open()
    .release = mmap_release,                // Connect close() to mmap_release()
    .mmap = mmap_driver_mmap,               // Connect mmap() to mmap_driver_mmap()
    .unlocked_ioctl = mmap_ioctl,           // Connect ioctl() to mmap_ioctl()
};


/* Runs when the driver module is inserted. */
static int __init mmap_ioctl_init(void)
{
    int ret;                                // Stores return values

    kernel_buffer = (char *)get_zeroed_page(GFP_KERNEL); // Allocate one zero-filled kernel page

    if (kernel_buffer == NULL)              // Check whether page allocation failed
    {
        pr_err("MMAP_IOCTL: Failed to allocate memory\n");

        return -ENOMEM;                     // Return out-of-memory error
    }

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME); // Dynamically allocate device numbers

    if (ret < 0)                            // Check whether device-number allocation failed
    {
        pr_err("MMAP_IOCTL: Failed to allocate device number\n");

        goto free_buffer;                   // Release the allocated kernel page
    }

    cdev_init(&mmap_cdev, &fops);           // Initialize cdev with file operations

    ret = cdev_add(&mmap_cdev, dev_num, 1); // Register the character device

    if (ret < 0)                            // Check whether cdev registration failed
    {
        pr_err("MMAP_IOCTL: Failed to add cdev\n");

        goto unregister_device;             // Release the device number and buffer
    }

    mmap_class = class_create(CLASS_NAME);  // Create a class under /sys/class/

    if (IS_ERR(mmap_class))                 // Check whether class creation failed
    {
        pr_err("MMAP_IOCTL: Failed to create class\n");

        ret = PTR_ERR(mmap_class);          // Get the actual class-creation error

        goto delete_cdev;                   // Remove cdev and previous resources
    }

    if (IS_ERR(device_create(mmap_class, NULL, dev_num, NULL, DEVICE_NAME))) // Create /dev/mmap_ioctl_device
    {
        pr_err("MMAP_IOCTL: Failed to create device\n");

        ret = -EINVAL;                      // Store the device-creation error

        goto destroy_class;                 // Destroy class and previous resources
    }

    pr_info("MMAP_IOCTL: Driver loaded\n");

    pr_info("MMAP_IOCTL: Device created as /dev/%s\n", DEVICE_NAME);

    return 0;                               // Driver initialization completed


/* Cleanup starts here when an initialization step fails. */

destroy_class:

    class_destroy(mmap_class);              // Destroy the previously created class

delete_cdev:

    cdev_del(&mmap_cdev);                   // Remove the registered character device

unregister_device:

    unregister_chrdev_region(dev_num, 1);   // Release the allocated device number

free_buffer:

    free_page((unsigned long)kernel_buffer); // Free the allocated kernel page

    return ret;                             // Return the original error code
}


/* Runs when the driver module is removed. */
static void __exit mmap_ioctl_exit(void)
{
    device_destroy(mmap_class, dev_num);    // Remove /dev/mmap_ioctl_device

    class_destroy(mmap_class);              // Destroy the device class

    cdev_del(&mmap_cdev);                   // Remove the character device

    unregister_chrdev_region(dev_num, 1);   // Release the device number

    free_page((unsigned long)kernel_buffer); // Free the kernel memory page

    pr_info("MMAP_IOCTL: Driver removed\n");
}


module_init(mmap_ioctl_init);               // Register the module initialization function
module_exit(mmap_ioctl_exit);               // Register the module exit function

MODULE_LICENSE("GPL");                      // Declare the module license
MODULE_AUTHOR("Sameer");                    // Declare the module author
MODULE_DESCRIPTION("mmap and ioctl TX notification driver"); // Module description