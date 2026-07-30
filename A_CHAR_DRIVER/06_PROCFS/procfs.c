#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<linux/err.h>


#define WR_VAL _IOW('a',1,int32_t*)
#define RD_VAl _IOR('a',2,int32_t*)

int32_t value = 0;
char name_arr[1024]="This is Try_Proc_Entry";
static int len = 1;

dev_t dev;
static struct proc_dir_entry *parent;

static int __init proc_entry_init(void);
static void __exit proc_exit_exit(void);

static int file_proc_open(struct inode *inode, struct file *file);
static int file_proc_close(struct inode *inode, struct file *file);
static ssize_t file_proc_read(struct file *fileptr,char __user *buff, size_t length, loff_t* offset);
static ssize_t file_proc_write(struct file *fileptr,const char *buff, size_t length, loff_t* offset);

static struct proc_ops proc_ops = 
{
    .proc_open = file_proc_open,
    .proc_read = file_proc_read,
    .proc_write = file_proc_write,
    .proc_release = file_proc_close,
};

/*** This function will be called when we open the procfs file*/
static int file_proc_open(struct inode *inode, struct file *file)
{
    pr_info("proc file opend.....\t");
    return 0;
}
/*** This function will be called when we close the procfs file*/
static int file_proc_close(struct inode *inode, struct file *file)
{
    pr_info("proc file released.....\n");
    return 0;
}
static ssize_t file_proc_read(struct file *filptr, char __user *buff, size_t length, loff_t* offset)
{
    pr_info("Proc File Reading..\n");
    if(len)
    {
        len = 0;
    }
    else
    {
        len = 1;
        return 0;
    }
    if(copy_to_user(buff,name_arr,sizeof(name_arr)))
    {
        pr_err("Data Sending Error..\n");
        return -1;
    }
    return length;
}
static ssize_t file_proc_write(struct file *fileptr,const char __user *buff,size_t length,loff_t *offset)
{
    if(length >= sizeof(name_arr))
        length = sizeof(name_arr) - 1;

    if(copy_from_user(name_arr, buff, length))
    {
        pr_err("Failed to Write data from User..\n");
        return -EFAULT;
    }

    name_arr[length] = '\0';

    pr_info("Received Data : %s\n", name_arr);

    return length;
}
static int __init proc_entry_init(void)
{
    printk(KERN_INFO "******PROC FILE DRIVER ******\n");
    parent = proc_mkdir("MYOWN_PROC_DIR",NULL);
    if(parent == NULL)
    {
        pr_info("Error Creating a Proc Entry..\n");
    }
    proc_create("SAMI_FILE",0666,parent,&proc_ops);
    pr_info("Device Driver Inserted Successfully..\n");
    return  0;
}
static void __exit proc_exit_exit(void)
{
    proc_remove(parent);        
    pr_info("##### LEAVING PROC DRIVER MODULE..###\n");
}
module_init(proc_entry_init);
module_exit(proc_exit_exit);

MODULE_DESCRIPTION("SMALL PROC ENTRY");
MODULE_AUTHOR("MOHD SAMEER");
MODULE_LICENSE("GPL");