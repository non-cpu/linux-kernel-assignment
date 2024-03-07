#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>

#include "ftracehooking.h"

#define __NR_ftrace 336

void **syscall_table;

static asmlinkage int (*real_ftrace)(const struct pt_regs *);

static pid_t target_pid = NULL;

static char *process_name = NULL;
static char file_name[256];

static unsigned long read_bytes = 0;
static unsigned long write_bytes = 0;

static int open_count = 0;
static int close_count = 0;
static int read_count = 0;
static int write_count = 0;
static int lseek_count = 0;

static asmlinkage int ftrace(const struct pt_regs *regs)
{
    pid_t pid = (pid_t)regs->di;

    if (pid == 0)
    {
        printk(KERN_INFO "[student_id] /%s file[%s] stats [x] read - %lu / written - %lu\n", process_name, file_name, read_bytes, write_bytes);
        printk(KERN_INFO "open[%d] close[%d] read[%d] write[%d] lseek[%d]\n", open_count, close_count, read_count, write_count, lseek_count);
        printk(KERN_INFO "OS Assignment 2 ftrace [%d] End\n", target_pid);
    }
    else
    {
        target_pid = pid;

        struct task_struct *task = current;
        process_name = task->comm;

        printk(KERN_INFO "OS Assignment 2 ftrace [%d] Start\n", target_pid);
    }

    return 0;
}

asmlinkage int ftrace_io(const struct pt_regs *regs)
{
    struct task_struct *task = current;
    pid_t pid = task->pid;

    int syscall_nr = regs->orig_ax;

    if (pid == target_pid)
    {
        if (syscall_nr == __NR_open)
        {
            copy_from_user(file_name, (const char __user *)regs->di, sizeof(file_name));
            open_count++;
        }
        else if (syscall_nr == __NR_close)
        {
            close_count++;
        }
        else if (syscall_nr == __NR_read)
        {
            read_bytes += regs->dx;
            read_count++;
        }
        else if (syscall_nr == __NR_write)
        {
            write_bytes += regs->dx;
            write_count++;
        }
        else if (syscall_nr == __NR_lseek)
        {
            lseek_count++;
        }
    }

    return 0;
}
EXPORT_SYMBOL(ftrace_io);

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
    real_ftrace = syscall_table[__NR_ftrace];
    syscall_table[__NR_ftrace] = ftrace;
    return 0;
}

static void __exit hooking_exit(void)
{
    syscall_table[__NR_ftrace] = real_ftrace;
    make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
