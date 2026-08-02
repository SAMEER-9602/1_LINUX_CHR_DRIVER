#include<linux/ioctl.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/param.h>


#define WR_VAL _IOW('s',1,int32_t*)
#define RD_VAL _IOR('s',2,int32_t*)

int32_t value = 0;
dev_t dev = 0;
static struct class *clsptr;
static struct cdev  etx_cdev;

static int      __init ioctal_init(void);
static void     __exit ioctal_exit(void);
static int      file_open(struct inode *inode, struct file *file);
static int      file_close(struct inode *inode, struct file *file);
static ssize_t  file_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  file_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     unlock_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


/*** This function will be called when we open the Device file*/
static int file_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*** This function will be called when we close the Device file*/
static int file_close(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
/*** This function will be called when we read the Device file*/
static ssize_t file_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        return 0;
}
/*** This function will be called when we write the Device file*/
static ssize_t file_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}

/*Within the function “ioctl” we need to implement all the commands that we defined above (WR_VALUE, RD_VALUE)*/
static long unlock_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case WR_VAL:
                if(copy_from_user(&value, (int32_t *)arg,sizeof(value)))
                {
                    pr_err("Data Write: Err!!\n");
                }
                pr_info("VAlue = %d\n",value);
                break;

        case RD_VAL:
                if(copy_to_user((int32_t *)arg, &value, sizeof(value)))
                {
                    pr_err("DAta REad  : Error..!!\n");
                }
                break;
        default    :
                pr_info("Default\n");
                break;
    }
    return 0;
}

static struct file_operations fops=
{
    .owner = THIS_MODULE,
    .open  = file_open,
    .release= file_close,
    .read   = file_read, 
    .write  = file_write,
    .unlocked_ioctl = unlock_ioctl,
};
static int __init ioctal_init(void)
{
    int res;
    res = alloc_chrdev_region(&dev,0,1,"MJR_MIN");
    if(res<0)
    {
        pr_info("Failed to Allocated Major and Minor No..\n");
    }
    printk("MAJOR NO IS : %d\t MINOR NO IS:%d\n",MAJOR(dev),MINOR(dev));

    clsptr = class_create("IOCTL_CLASS");
    if(IS_ERR(clsptr))
    {
        pr_err("Failed to Create a Class..\n");
        goto unreg;
    }
    pr_info("class created..\n");

    if(IS_ERR(device_create(clsptr,NULL,dev,NULL,"IOCTL_DEVICE")))
    {
        pr_info("Failed To Create Device Folder\n");
        goto rmcls;
    }
    pr_info("Device File Created..\n");

    cdev_init(&etx_cdev,&fops);
    if((cdev_add(&etx_cdev,dev,1))<0)
    {
        pr_err("Cannot Add the device to the system\n");
        goto rmcls;
    }
    pr_info("CDEV_struct is added..\n");
    pr_info("MODULE INSERTED SUCCESSFULLY..\n");
    return 0;

    unreg: unregister_chrdev_region(dev,1);
    rmcls: class_destroy(clsptr);
    return -1;
}
static void __exit ioctal_exit(void)
{
    cdev_del(&etx_cdev);
    device_destroy(clsptr,dev);
    class_destroy(clsptr);
    unregister_chrdev_region(dev,1);
    pr_err("MODULE UNLOADED SUCCESSFULLY..\n");
}
module_init(ioctal_init);
module_exit(ioctal_exit);

MODULE_DESCRIPTION("SImple IOCTL Control");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOHD SAMIR");




















