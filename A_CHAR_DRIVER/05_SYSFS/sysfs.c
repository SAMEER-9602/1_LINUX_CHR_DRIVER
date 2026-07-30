#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/err.h>

static struct kobject *obj_ref;
static int value = 0;

/******** SysFS Function Prototypes ********/
static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr,char *buff);
static ssize_t sysfs_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buff,size_t count);

/******** SysFS Attribute ********/

static struct kobj_attribute my_att = __ATTR(value, 0664, sysfs_show, sysfs_store);

/******** SysFS Show Function ********/
static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr,char *buff)
{
    pr_info("SysFS - Reading\n");
    return sprintf(buff, "%d\n", value);
}

/******** SysFS Store Function ********/
static ssize_t sysfs_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buff,size_t count)
{
    pr_info("SysFS - Writing\n");
    sscanf(buff, "%d", &value);
    return count;
}

/******** Module Init Function ********/

static int __init sys_entry_init(void)
{
    /* Create /sys/kernel/My_SysFs directory */
    obj_ref = kobject_create_and_add("My_SysFs",kernel_kobj);
    if (!obj_ref)
    {
        pr_err("Cannot Create SysFS Directory\n");
        return -ENOMEM;
    }

    /** Create: /sys/kernel/My_SysFs/value */
    if (sysfs_create_file(obj_ref,&my_att.attr))
    {
        pr_err("Cannot Create SysFS File\n");
        kobject_put(obj_ref);
        return -1;
    }
    pr_info("SysFS Driver Loaded Successfully\n");
    return 0;
}

/******** Module Exit Function ********/

static void __exit sys_exit_exit(void)
{
    sysfs_remove_file(obj_ref,&my_att.attr);
    kobject_put(obj_ref);
    pr_info("SysFS Driver Removed Successfully\n");
}

module_init(sys_entry_init);
module_exit(sys_exit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohd Sami");
MODULE_DESCRIPTION("Simple SysFS Linux Driver");
MODULE_VERSION("1.0");
