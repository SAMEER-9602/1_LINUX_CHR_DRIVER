#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/interrupt.h>
#include<linux/gpio.h>
#include<linux/err.h>
#include<linux/kobject.h>
#include<linux/slab.h>
#include<linux/delay.h>

#define BTN_PIN 547
#define LED_PIN 540

int value=0,IRQ_NO,led_status=0;

struct kobject *kobj;
struct tasklet_struct *tasklet = NULL;

void my_tasklet_fn(unsigned long data);

// DECLARE_TASKLET(tasklet,my_tasklet_fn);//this is for static method
/* When Macro Expands it assign values to structure of tasklet
tasklet.next  = NULL;
tasklet.state = TASKLET_STATE_SCHED;
tasklet.count = 0;
tasklet.funct = my_tasklet_fn;
tasklet.data  = 1;
*/

void my_tasklet_fn(unsigned long data)
{
    printk(KERN_INFO "EXECUTING TASKLET FUNCTION \n");
    gpio_set_value(LED_PIN,led_status);
    // msleep(350);
}
static irqreturn_t BTN_HNDLER_FN(int irq, void* arg)
{
    printk(KERN_INFO "Shared IRQ: INTERRUPT OCCURED\n");
    led_status = !led_status;
    tasklet_schedule(tasklet);
    return IRQ_HANDLED;
}
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

static int __init etx_driver_init(void)
{
    int ret;

    gpio_request(BTN_PIN,"BBB_BUTTON");
    gpio_request(LED_PIN,"BBB_LED");

    gpio_direction_output(LED_PIN,0);
    gpio_direction_input(BTN_PIN);

    IRQ_NO = gpio_to_irq(BTN_PIN);
    if(IRQ_NO<0)
    {
        printk(KERN_INFO " Failed to allocated Iqr No\n");
        goto irq;
    }
    printk(KERN_INFO "IRQ No is %d\n",IRQ_NO);

    ret = request_irq(IRQ_NO,BTN_HNDLER_FN,IRQF_TRIGGER_RISING,"my_handler",NULL); 
    if(ret<0)
    {
        printk(KERN_INFO "Failed to register request\n");
        return ret;
    }
    tasklet = kmalloc(sizeof(struct tasklet_struct),GFP_KERNEL);
    if(tasklet == NULL)
    {
        printk(KERN_INFO "Etx_device: Cannot Allocate Memory\n");
        goto irq;
    }
    tasklet_init(tasklet,my_tasklet_fn,0);

    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;
 
irq:
    free_irq(IRQ_NO,NULL);
    return -1;
}

static void __exit etx_driver_exit(void)
{
        /*Kill the Tasklet */ 
        gpio_free(BTN_PIN);
        tasklet_kill(tasklet);
        if(tasklet != NULL)
        {
            kfree(tasklet);
        }
        free_irq(IRQ_NO,NULL);
        printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("SIMPLE TASKLET PROGRAM");

