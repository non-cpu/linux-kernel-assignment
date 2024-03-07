#include <asm/unistd.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>

#include "ftracehooking.h"

void **syscall_table;

static asmlinkage long (*real_sys_open)(const struct pt_regs *);
static asmlinkage ssize_t (*real_sys_read)(const struct pt_regs *);
static asmlinkage ssize_t (*real_sys_write)(const struct pt_regs *);
static asmlinkage off_t (*real_sys_lseek)(const struct pt_regs *);
static asmlinkage int (*real_sys_close)(const struct pt_regs *);

static asmlinkage long ftrace_open(const struct pt_regs *regs)
{
    ftrace_io(regs);
    return real_sys_open(regs);
}

static asmlinkage ssize_t ftrace_read(const struct pt_regs *regs)
{
    ftrace_io(regs);
    return real_sys_read(regs);
}

static asmlinkage ssize_t ftrace_write(const struct pt_regs *regs)
{
    ftrace_io(regs);
    return real_sys_write(regs);
}

static asmlinkage off_t ftrace_lseek(const struct pt_regs *regs)
{
    ftrace_io(regs);
    return real_sys_lseek(regs);
}

static asmlinkage int ftrace_close(const struct pt_regs *regs)
{
    ftrace_io(regs);
    return real_sys_close(regs);
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

static int __init iotracehooking_init(void)
{
    syscall_table = (void **)kallsyms_lookup_name("sys_call_table");

    make_rw(syscall_table);

    real_sys_open = syscall_table[__NR_open];
    real_sys_read = syscall_table[__NR_read];
    real_sys_write = syscall_table[__NR_write];
    real_sys_lseek = syscall_table[__NR_lseek];
    real_sys_close = syscall_table[__NR_close];

    syscall_table[__NR_open] = ftrace_open;
    syscall_table[__NR_read] = ftrace_read;
    syscall_table[__NR_write] = ftrace_write;
    syscall_table[__NR_lseek] = ftrace_lseek;
    syscall_table[__NR_close] = ftrace_close;

    return 0;
}

static void __exit iotracehooking_exit(void)
{
    syscall_table[__NR_open] = real_sys_open;
    syscall_table[__NR_read] = real_sys_read;
    syscall_table[__NR_write] = real_sys_write;
    syscall_table[__NR_lseek] = real_sys_lseek;
    syscall_table[__NR_close] = real_sys_close;

    make_ro(syscall_table);
}

module_init(iotracehooking_init);
module_exit(iotracehooking_exit);
MODULE_LICENSE("GPL");
