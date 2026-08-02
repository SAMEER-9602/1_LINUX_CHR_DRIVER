/*Program 2: Two kernel threads
thread_fn_1()
{
    spin_lock(&spinlock);
    value++;
    spin_unlock(&spinlock);
}

thread_fn_2()
{
    spin_lock(&spinlock);
    value++;
    spin_unlock(&spinlock);
}
Thread 1 increments value.
Thread 2 increments value.
Both threads run in process context.
Normal spin_lock() is enough because there is no IRQ or tasklet accessing the same variable.

Main point: This program uses Thread + Thread.
*********************************************************************/

#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/err.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/slab.h>
#include<linux/delay.h>
#include<linux/sched.h> //for task_struct
#include<linux/kthread.h>
#include<linux/kdev_t.h>
#include<linux/kobject.h>

DEFINE_SPINLOCK(spinlock);

int value=0;

struct task_struct *thrd_1;
struct task_struct *thrd_2;

int thread_fn_1(void *arg);
int thread_fn_2(void *arg);

int thread_fn_1(void *arg)
{
    while(!kthread_should_stop())
    {
        if(!spin_is_locked(&spinlock)) //spinlock define static method
        {
            pr_info("Spin Lock is Free, we can use it\n");
        }
        spin_lock(&spinlock);
        if(spin_is_locked(&spinlock))
        {
            pr_info("spin locked for Thread-1\n");
        }
        value++;
        pr_info("Value Increased By Thread_1:%d\n",value);
        spin_unlock(&spinlock);
         pr_info("Spin Lock is Free by THREAD-1..\n");
        msleep(1000);
    }
    return 0;
}
int thread_fn_2(void *arg)
{
    while(!kthread_should_stop())
    {
        spin_lock(&spinlock);
        pr_info("spin locked for Thread-2 \n");
        value++;
        pr_info("Value Increased By Thread_2 :%d\n",value);
        spin_unlock(&spinlock);
        pr_info("Spin Lock is Free by THREAD-2..\n");
        msleep(1000);
    }
    return 0;
}

static int __init spin_init(void)
{
    thrd_1 = kthread_run(thread_fn_1,NULL,"My_thrd_1"); //this start function 1
    if(thrd_1)
    {
        pr_info("Thread 1 Created\n");
    }
    else
    {
        pr_info("failed to create a thread\n");
        // return -1;
    }
    thrd_2 = kthread_run(thread_fn_2,NULL,"My_thrd_2");
    if(thrd_2)
    {
        pr_info("Thread 2 Created\n");
    }
    else
    {
        pr_info("failed to create a thread\n");
        // return -1;
    }   
    pr_info("**************DRiver loadded*************\n");
    return 0;
}
static void __exit spin_exit(void)
{
    kthread_stop(thrd_1);
    kthread_stop(thrd_2);
    pr_info("---------Driver removed--------\n");
}
module_init(spin_init);
module_exit(spin_exit);

MODULE_LICENSE("GPL");



/*
 *Program 3: Two tasklets
tsklet_fn_1()
{
    spin_lock(&spinlock);
    value++;
    spin_unlock(&spinlock);
}

tsklet_fn_2()
{
    spin_lock(&spinlock);
    value++;
    spin_unlock(&spinlock);
}
Tasklet 1 increments value.
Tasklet 2 increments value.
Both run in bottom-half/softirq context.
Normal spin_lock() is used.

Main point: This program uses Tasklet + Tasklet.
***************************************************
 *
#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/interrupt.h>
#include<linux/spinlock.h>
#include<linux/delay.h>

void tsklet_fn_1(struct tasklet_struct *t);
void tsklet_fn_2(struct tasklet_struct *t);


DECLARE_TASKLET(tasklet1,tsklet_fn_1);
DECLARE_TASKLET(tasklet2,tsklet_fn_2);
DEFINE_SPINLOCK(spinlock);


int value=0;

void tsklet_fn_1(struct tasklet_struct *t)
{
    if(!spin_is_locked(&spinlock))
    {
        printk(KERN_INFO "Spin Lock is free\n");
    }
    spin_lock(&spinlock);
    if(spin_is_locked(&spinlock))
    {
        printk(KERN_INFO "Spin is Locked By Tasklet - 1:\n");
    }
    value++;
    printk(KERN_INFO "Value is Increased by Taskle_1 : %d\n",value);
    spin_unlock(&spinlock);
    printk(KERN_INFO "Spin Locked is Freed by Tasklet-1 \n");
    // return 0;
}
void tsklet_fn_2(struct tasklet_struct *t)
{
    if(!spin_is_locked(&spinlock))
    {
        printk(KERN_INFO "Spin is Lock is Free\n");
    }
    spin_lock(&spinlock);
    if(spin_is_locked(&spinlock))
    {
        printk(KERN_INFO "Spin is Locked by Tasklet - 2:\n");
    }
    value++;
    printk(KERN_INFO "Value is Increased by Tasklet_2 : %d\n", value);
    spin_unlock(&spinlock);
    printk(KERN_INFO "Spin Locked is Freed by Tasklet -2\n");
    // return 0;
}
static int __init taskle_init(void)
{
    tasklet_schedule(&tasklet1);
    tasklet_schedule(&tasklet2);
    printk(KERN_INFO "******* Driver Loadded Successfully.****.\n");
    return 0;
}
static void __exit tasklet_exit(void)
{
    tasklet_kill(&tasklet1);
    tasklet_kill(&tasklet2);
    printk(KERN_INFO "--------Driver Unloadded-------\n");
}
module_init(taskle_init);
module_exit(tasklet_exit);
MODULE_LICENSE("GPL");
 * */


/*
 *Program 4: Kernel thread + Tasklet
my_thread_fn()
{
    spin_lock_bh(&spinlock);
    value++;
    spin_unlock_bh(&spinlock);
}

my_tsklet_fn()
{
    spin_lock_bh(&spinlock);
    value++;
    spin_unlock_bh(&spinlock);
}
Kernel thread increments value.
Tasklet increments value.
The thread runs in process context.
The tasklet runs in bottom-half context.
spin_lock_bh() disables local bottom halves while taking the lock.

Main point: This program uses Thread + Tasklet.
****************************************************************

#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/interrupt.h>
#include<linux/slab.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/spinlock.h>

struct tasklet_struct *tasklet = NULL;
struct task_struct *mythread;
DEFINE_SPINLOCK(spinlock);
int value=0;

int my_thread_fn(void *arg);
void my_tsklet_fn(unsigned long data);

void my_tsklet_fn(unsigned long data)
{
    spin_lock_bh(&spinlock);
    printk(KERN_INFO "tasklet function lock Spin_lock \n");
    value++;
    printk(KERN_INFO "Value is %d",value);
    spin_unlock_bh(&spinlock);
    printk(KERN_INFO "Tasklet Function removed Spin_lock");
    // return 0; tasklet function always void
}
int my_thread_fn(void *arg)
{
    while(!kthread_should_stop())
    {
        spin_lock_bh(&spinlock);
        printk(KERN_INFO "thread function lock Spin_lock \n");
        value++;
        printk(KERN_INFO "Value is %d",value);
        spin_unlock_bh(&spinlock);
        printk(KERN_INFO "Thread Function removed Spin_lock");
        msleep(1000);
    }
    return 0;
}
static int __init main_init(void)
{
    tasklet = kmalloc(sizeof(struct tasklet_struct),GFP_KERNEL);
    if(!tasklet)
    {
        return -ENOMEM;
    }
    tasklet_init(tasklet,&my_tsklet_fn,0);
    tasklet_schedule(tasklet);
    mythread = kthread_run(my_thread_fn,NULL,"MY_THREAD");
    if(IS_ERR(mythread))
    {
        printk(KERN_INFO "Faile to Create thread\n");
    }
    printk(KERN_INFO "******Module Inserted*******\n");
    return 0;
}
static void __exit main_exit(void)
{
    tasklet_kill(tasklet);
    if(tasklet!=NULL)
    {
        kfree(tasklet);
    }
    kthread_stop(mythread);
    printk(KERN_INFO "-----Module_removed-----------\n");
}
module_init(main_init);
module_exit(main_exit);
MODULE_LICENSE("GPL");

 *
 * */



/**

Program 1: IRQ handler + Tasklet
my_irq_handler()
{
    spin_lock_irqsave(&spinlock, flags);
    value++;
    spin_unlock_irqrestore(&spinlock, flags);

    tasklet_schedule(tasklet);
}

my_tasklet_fn()
{
    spin_lock_irqsave(&spinlock, flags);
    value++;
    spin_unlock_irqrestore(&spinlock, flags);
}
Button interrupt occurs.
IRQ handler increments value.
IRQ handler schedules the tasklet.
Tasklet also increments value.
spin_lock_irqsave() disables local interrupts and saves the previous interrupt state.

Main point: This program uses Hard IRQ + Tasklet.
*********************************************************************


#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/kthread.h>
#include<linux/spinlock.h>
#include<linux/delay.h>
#include<linux/gpio.h>
#include<linux/slab.h>

#define BTN_PIN 547
#define LED_PIN 540
void my_tasklet_fn(unsigned long arg);
static irqreturn_t my_irq_handler(int irq,void *arg);
DEFINE_SPINLOCK(spinlock);
int IRQ_NO,value;
struct tasklet_struct *tasklet=NULL;
static irqreturn_t my_irq_handler(int irq,void *arg)
{
    unsigned long flags;
    gpio_set_value(LED_PIN,1);
    printk(KERN_INFO "Interrupt Trigger***\n");
    spin_lock_irqsave(&spinlock,flags);
    value++;
    printk(KERN_INFO"value increased by INterrupt %d\n",value);
    spin_unlock_irqrestore(&spinlock,flags);
    tasklet_schedule(tasklet);
    gpio_set_value(LED_PIN,0);
    return IRQ_HANDLED;
}
void my_tasklet_fn(unsigned long arg)
{
    unsigned long flags;
    spin_lock_irqsave(&spinlock,flags);
    value++;
    printk(KERN_INFO "Valued Increased by Tasklet_fn:%d\n",value);
    spin_unlock_irqrestore(&spinlock,flags);
}
static int __init main_init(void)
{
    gpio_request(BTN_PIN,"BBB_BUTTON");
    gpio_request(LED_PIN,"BBB_LED");
    gpio_direction_output(LED_PIN,0);
    gpio_direction_input(BTN_PIN);
    IRQ_NO = gpio_to_irq(BTN_PIN);
    printk(KERN_INFO "IRQ_NO is : %d\n",IRQ_NO);
    int ret= request_irq(IRQ_NO,my_irq_handler,IRQF_TRIGGER_FALLING,"My_IRQ",NULL);
    if(ret)
    {
        printk(KERN_INFO"Failed to Allocated Request\n");
        return ret;
    }
    tasklet = kmalloc(sizeof(struct tasklet_struct),GFP_KERNEL);
    if(tasklet == NULL)
    {
        printk(KERN_INFO "Failed to Allocated memory to Tasklet\n");
        return -ENOMEM;
    }
    tasklet_init(tasklet,my_tasklet_fn,0);
    // tasklet_schedule(&my_tasklet_fn);
    return 0;

}
static void __exit main_exit(void)
{
    gpio_free(BTN_PIN);
    gpio_free(LED_PIN);
    tasklet_kill(tasklet);
    if(tasklet != NULL)
    {
        kfree(tasklet);
        tasklet = NULL;
    }
    free_irq(IRQ_NO,NULL);
}
module_init(main_init);
module_exit(main_exit);

MODULE_LICENSE("GPL");


// #include<linux/init.h>
// #include<linux/interrupt.h>
// #include<linux/module.h>
// #include<linux/kernel.h>
// #include<linux/kthread.h>
// #include<linux/spinlock.h>
// #include<linux/delay.h>
// #include<linux/gpio.h>
// #include<linux/slab.h>

// #define BTN_PIN 547
// #define LED_PIN 540
// void my_tasklet_fn(unsigned long arg);
// static irqreturn_t my_irq_handler(int irq,void *arg);
// DEFINE_SPINLOCK(spinlock);
// int IRQ_NO,value;
// struct tasklet_struct *tasklet=NULL;
// static irqreturn_t my_irq_handler(int irq,void *arg)
// {
//     gpio_set_value(LED_PIN,1);
//     printk(KERN_INFO "Interrupt Trigger***\n");
//     spin_lock_irq(&spinlock);
//     value++;
//     printk(KERN_INFO"value increased by INterrupt %d\n",value);
//     spin_unlock_irq(&spinlock);
//     tasklet_schedule(tasklet);
//     gpio_set_value(LED_PIN,0);
//     return IRQ_HANDLED;
// }
// void my_tasklet_fn(unsigned long arg)
// {
//     spin_lock_irq(&spinlock);
//     value++;
//     printk(KERN_INFO "Valued Increased by Tasklet_fn:%d\n",value);
//     spin_unlock_irq(&spinlock);
// }
// static int __init main_init(void)
// {
//     gpio_request(BTN_PIN,"BBB_BUTTON");
//     gpio_request(LED_PIN,"BBB_LED");
//     gpio_direction_output(LED_PIN,0);
//     gpio_direction_input(BTN_PIN);
//     IRQ_NO = gpio_to_irq(BTN_PIN);
//     printk(KERN_INFO "IRQ_NO is : %d\n",IRQ_NO);
//     int ret= request_irq(IRQ_NO,my_irq_handler,IRQF_TRIGGER_FALLING,"My_IRQ",NULL);
//     if(ret)
//     {
//         printk(KERN_INFO"Failed to Allocated Request\n");
//         return ret;
//     }
//     tasklet = kmalloc(sizeof(struct tasklet_struct),GFP_KERNEL);
//     if(tasklet == NULL)
//     {
//         printk(KERN_INFO "Failed to Allocated memory to Tasklet\n");
//         return -ENOMEM;
//     }
//     tasklet_init(tasklet,my_tasklet_fn,0);
//     // tasklet_schedule(&my_tasklet_fn);
//     return 0;

// }
// static void __exit main_exit(void)
// {
//     tasklet_kill(tasklet);
//     if(tasklet != NULL)
//     {
//         kfree(&tasklet);
//         tasklet = NULL;
//     }
//     free_irq(IRQ_NO,NULL);
// }
// module_init(main_init);
// module_exit(main_exit);

// MODULE_LICENSE("GPL");
  *
  * **/
