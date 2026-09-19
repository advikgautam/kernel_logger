#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

static unsigned long start_jiffies;

/*
 * Walks the kernel's live process list (the same linked list that `ps`,
 * Task Manager, and security tools read from) and logs the PID and name
 * of every currently running process.
 *
 * for_each_process() is a kernel macro that iterates over every
 * task_struct in the system's process list.
 */
static void list_processes(void) {
    struct task_struct *task;

    printk(KERN_INFO "LOGGER: --- Active Process List ---\n");

    for_each_process(task) {
        printk(KERN_INFO "LOGGER: PID: %d | Name: %s\n", task->pid, task->comm);
    }

    printk(KERN_INFO "LOGGER: --- End of Process List ---\n");
}

static int __init logger_init(void) {
    unsigned long uptime_seconds = jiffies / HZ;
    start_jiffies = jiffies;

    printk(KERN_INFO "LOGGER: Module loaded.\n");
    printk(KERN_INFO "LOGGER: System uptime is %lu seconds.\n", uptime_seconds);

    list_processes();

    return 0;
}

static void __exit logger_exit(void) {
    unsigned long active_jiffies = jiffies - start_jiffies;
    unsigned long active_seconds = active_jiffies / HZ;

    printk(KERN_INFO "LOGGER: Module unloaded.\n");
    printk(KERN_INFO "LOGGER: Module was active for %lu seconds.\n", active_seconds);
}

module_init(logger_init);
module_exit(logger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advik Gautam");
MODULE_DESCRIPTION("Kernel State Logger with Process Enumeration");
