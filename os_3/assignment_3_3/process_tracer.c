#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>

#define __NR_ptrace 336

void **syscall_table;

static asmlinkage int (*real_ptrace)(const struct pt_regs *);

static const char *get_task_state_string(long state) {
    switch (state) {
        case TASK_RUNNING:
        case TASK_INTERRUPTIBLE:
        case TASK_UNINTERRUPTIBLE:
        case TASK_KILLABLE:
        case TASK_WAKEKILL:
            return "Running or ready";
        case TASK_STOPPED:
            return "Stopped";
        case TASK_TRACED:
            return "Traced";
        case EXIT_DEAD:
            return "Dead";
        case EXIT_ZOMBIE:
            return "Zombie process";
        default:
            return "ETC";
    }
}

static asmlinkage int process_tracer(const struct pt_regs *regs)
{
    pid_t pid = (pid_t)regs->di;

    struct task_struct *task;
    char parent_info[256];
    char siblings_info[256];
    char children_info[256];

    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task == NULL) {
        printk(KERN_ERR "Process with PID %d not found\n", pid);
        return;
    }

    struct task_struct *parent = task->parent;
    struct task_struct *sibling;
    struct task_struct *child;

    snprintf(parent_info, sizeof(parent_info), "it's parent process [%d] %s\n", parent->pid, parent->comm);

    snprintf(siblings_info, sizeof(siblings_info), "it's sibling process):\n");
    list_for_each_entry(sibling, &parent->children, sibling) {
        if (sibling != task) {
            char sibling_info[64];
            snprintf(sibling_info, sizeof(sibling_info), "    > [%d] %s\n", sibling->pid, sibling->comm);
            strncat(siblings_info, sibling_info, sizeof(siblings_info) - strlen(siblings_info) - 1);
        }
    }

    snprintf(children_info, sizeof(children_info), "it's child processes):\n");
    list_for_each_entry(child, &task->children, sibling) {
        char child_pid_str[16];
        snprintf(child_pid_str, sizeof(child_pid_str), "%d", child->pid);

        strncat(children_info, "    > ", sizeof(children_info) - strlen(children_info) - 1);
        strncat(children_info, child_pid_str, sizeof(children_info) - strlen(children_info) - 1);
        strncat(children_info, " ", sizeof(children_info) - strlen(children_info) - 1);
        strncat(children_info, child->comm, sizeof(children_info) - strlen(children_info) - 1);
        strncat(children_info, "\n", sizeof(children_info) - strlen(children_info) - 1);
    }

    printk("##### TASK INFORMATION of \"[%d] %s\" #####\n", task->pid, task->comm);
    printk("task state: %s\n", get_task_state_string(task->state));
    printk("Process Group Leader: [%d] %s\n", task->tgid, task->comm);
    printk("Number of context switches: %lu\n", task->nvcsw + task->nivcsw);
    printk("Number of calling fork(): %d\n", task->fork_count);
    printk("%s", parent_info);
    printk("%s", siblings_info);
    printk("%s", children_info);
    printk("##### END OF INFORMATION #####\n");

    return pid;
}

void make_rw(void *addr)
{
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    if (pte->pte & ~_PAGE_RW)
        pte->pte |= _PAGE_RW;
}

void make_ro(void *addr)
{
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    pte->pte = pte->pte & ~_PAGE_RW;
}

static int __init hooking_init(void)
{
    syscall_table = (void **)kallsyms_lookup_name("sys_call_table");
    make_rw(syscall_table);
    real_ptrace = syscall_table[__NR_ptrace];
    syscall_table[__NR_ptrace] = process_tracer;
    return 0;
}

static void __exit hooking_exit(void)
{
    syscall_table[__NR_ptrace] = real_ptrace;
    make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
