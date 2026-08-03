#include<linux/module.h>
#include<linux/param.h>
#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/kernel.h>

static int speed=10;
static int mode = 0;

static int param_get_speed(char *buffer, const struct kernel_param *kp)
{
    return sprintf(buffer, "%d",speed);
}
static int param_set_speed(const char *val, const struct kernel_param *kp)
{
    int ret = kstrtoint(val, 10,&speed);
    if(ret != 0)
    {
        return ret;
    }
    printk(KERN_INFO "Speed Changed to %d\n",speed);
    return 0;
}
static const struct kernel_param_ops Speed_ops = 
{
    .set = param_set_speed,
    .get = param_get_speed,
};
module_param(mode, int, S_IRUGO);
module_param_cb(speed,&Speed_ops,&speed, S_IRUGO | S_IWUSR);

static int __init param_init(void)
{
    printk(KERN_INFO "Driver loaded with Speed value = %d\n",speed);
    return 0;
}
static void __exit param_exit(void)
{
    printk(KERN_INFO "Driver Unloaded\n");
}
module_init(param_init);
module_exit(param_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohd Samir");
