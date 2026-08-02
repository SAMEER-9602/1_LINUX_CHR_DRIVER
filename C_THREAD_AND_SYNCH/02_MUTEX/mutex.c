#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/err.h>

struct task_struct *thread1_task;
struct task_struct *thread2_task;

/* Shared resource */
int shared_counter = 0;

/* Dynamically initialized mutex */
struct mutex my_mutex;

int __init mutex_dynamic_init(void);
void __exit mutex_dynamic_exit(void);
int thread1_function(void *data);
int thread2_function(void *data);

/* Kernel Thread 1 */
int thread1_function(void *data)
{
    while(!kthread_should_stop())
    {
        /* Lock the mutex */
        mutex_lock(&my_mutex);

        /* Critical section starts */
        shared_counter++;

        pr_info("Thread 1: Counter = %d\n", shared_counter);

        /* Critical section ends */

        /* Unlock the mutex */
        mutex_unlock(&my_mutex);

        ssleep(1);
    }
    pr_info("Thread 1 completed\n");

    return 0;
}

/* Kernel Thread 2 */
int thread2_function(void *data)
{   
    while(!kthread_should_stop())
    {

        /* Lock the mutex */
        mutex_lock(&my_mutex);

        /* Critical section starts */
        shared_counter++;

        pr_info("Thread 2: Counter = %d\n", shared_counter);

        /* Critical section ends */

        /* Unlock the mutex */
        mutex_unlock(&my_mutex);

        ssleep(1);
    }
    pr_info("Thread 2 completed\n");

    return 0;
}

/* Module initialization */
int __init mutex_dynamic_init(void)
{
    pr_info("Mutex dynamic initialization module loaded\n");

    /** Dynamically initialize the mutex.*/
    mutex_init(&my_mutex);

    /** Create and start Thread 1.*/
    thread1_task = kthread_run(thread1_function, NULL, "mutex_thread1");

    if (IS_ERR(thread1_task)) {
        pr_err("Failed to create Thread 1\n");
        return PTR_ERR(thread1_task);
    }

    /** Create and start Thread 2.*/
    thread2_task = kthread_run(thread2_function, NULL, "mutex_thread2");

    if (IS_ERR(thread2_task)) {
        pr_err("Failed to create Thread 2\n");

        /** Stop Thread 1 because Thread 2 creation failed.
         */
        kthread_stop(thread1_task);

        return PTR_ERR(thread2_task);
    }

    pr_info("Both threads created successfully\n");

    return 0;
}

/* Module exit */
void __exit mutex_dynamic_exit(void)
{
    if (thread1_task)
        kthread_stop(thread1_task);

    if (thread2_task)
        kthread_stop(thread2_task);

    pr_info("Final counter value = %d\n", shared_counter);

    pr_info("Mutex dynamic module removed\n");
}

module_init(mutex_dynamic_init);
module_exit(mutex_dynamic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MOHD SAMEER");
MODULE_DESCRIPTION("Mutex Program with Dynamic Initialization");