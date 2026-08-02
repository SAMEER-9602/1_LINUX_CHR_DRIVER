#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/err.h>

struct task_struct *thread1_task;
struct task_struct *thread2_task;

/*** Fucntion Declearations ****/
int thread1_function(void *data);
int thread2_function(void *data);
int __init two_kthread_init(void);
void __exit two_kthread_exit(void);

/* Kernel Thread 1 */
int thread1_function(void *data)
{
    int count = 0;

    while (!kthread_should_stop()) 
    {
        pr_info("Thread 1 is running: %d\n", count);

        count++;

        /* Sleep for 1 second */
        ssleep(1);
    }

    pr_info("Thread 1 is stopping\n");

    return 0;
}

/* Kernel Thread 2 */
int thread2_function(void *data)
{
    int count = 0;

    while (!kthread_should_stop()) 
    {
        pr_info("Thread 2 is running: %d\n", count);

        count++;

        /* Sleep for 2 seconds */
        ssleep(2);
    }

    pr_info("Thread 2 is stopping\n");

    return 0;
}

/* Module initialization */
int __init two_kthread_init(void)
{
    pr_info("Two kthread module loaded\n");

    /* Create and start Thread 1 */
    thread1_task = kthread_run(thread1_function, NULL, "my_thread1");

    if (IS_ERR(thread1_task)) 
    {
        pr_err("Failed to create Thread 1\n");
        return PTR_ERR(thread1_task);
    }

    pr_info("Thread 1 created successfully\n");

    /* Create and start Thread 2 */
    thread2_task = kthread_run(thread2_function, NULL, "my_thread2");

    if (IS_ERR(thread2_task)) 
    {
        pr_err("Failed to create Thread 2\n");

        /* Stop Thread 1 because Thread 2 creation failed */
        kthread_stop(thread1_task);

        return PTR_ERR(thread2_task);
    }

    pr_info("Thread 2 created successfully\n");

    return 0;
}

/* Module exit */
void __exit two_kthread_exit(void)
{
    pr_info("Stopping Thread 1\n");

    kthread_stop(thread1_task);

    pr_info("Stopping Thread 2\n");

    kthread_stop(thread2_task);

    pr_info("Two kthread module removed\n");
}

module_init(two_kthread_init);
module_exit(two_kthread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOHD SAMEER");
MODULE_DESCRIPTION("Simple Program Using Two Kernel Threads");