#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

/* Wait queue */
static wait_queue_head_t waitqueue_head;

/* Wait condition */
static int wait_flag = 0;

/* Kernel thread pointer */
static struct task_struct *my_thread;

/* Kernel timer */
static struct timer_list my_timer;

/* Event count */
static unsigned int event_count = 0;

/* This function executes after 5 seconds. */
static void timer_callback(struct timer_list *timer)
{
    pr_info("Timer expired\n");
    wait_flag = 1;    /* Make the wait condition true */
    wake_up_interruptible(&waitqueue_head);    /* Wake the kernel thread */
    mod_timer(&my_timer,jiffies + msecs_to_jiffies(5000));     // restart timer again
}

static int wait_function(void *data)
{
    while (!kthread_should_stop())
    {
        pr_info("Kernel thread is waiting...\n");

        /*
         * The kernel thread sleeps here until:
         *
         * 1. wait_flag becomes 1, OR
         * 2. kthread_stop() is called
         */
        wait_event_interruptible(waitqueue_head,wait_flag == 1 ||kthread_should_stop());

        if (kthread_should_stop())
        {
            pr_info("Kernel thread is stopping\n");
            break;
        }
        if (wait_flag == 1)
        {
            event_count++;

            pr_info("Kernel thread woke up. Event count = %u\n",event_count);

            wait_flag = 0;
        }
    }

    return 0;
}

static int __init wait_timer_init(void)
{
    /* Initialize wait queue.*/
    init_waitqueue_head(&waitqueue_head);

    my_thread = kthread_run(wait_function,NULL,"WAIT_THREAD");

    if (IS_ERR(my_thread))
    {
        pr_err("Failed to create kernel thread\n");
        return PTR_ERR(my_thread);
    }

    pr_info("Kernel thread created successfully\n");

    timer_setup(&my_timer,timer_callback,0);

    mod_timer(&my_timer,jiffies +msecs_to_jiffies(5000));

    pr_info("Module inserted successfully\n");

    return 0;
}

static void __exit wait_timer_exit(void)
{
    /** Delete timer safely.*/
    timer_delete_sync(&my_timer);

    if (!IS_ERR_OR_NULL(my_thread))
    {
        kthread_stop(my_thread);
    }

    pr_info("Module removed successfully\n");
}

module_init(wait_timer_init);
module_exit(wait_timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sameer");
MODULE_DESCRIPTION("Kernel Thread + Wait Queue + Timer");
MODULE_VERSION("1.0");