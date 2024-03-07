#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>

#define __NR_ftrace 336

void **syscall_table;

static asmlinkage int (*real_ptrace)(const struct pt_regs *);

static asmlinkage int file_varea(const struct pt_regs *regs)
{
    struct task_struct *task;
    struct mm_struct *mm;
    struct vm_area_struct *vma;

    char buffer[PATH_MAX];

    pid_t pid = (pid_t)regs->di;

    task = get_pid_task(find_vpid(pid), PIDTYPE_PID);

    mm = task->mm;

    printk("######## Loaded files of a process '%s(%d)' in VM ########", task->comm, pid);

    if (mm)
    {
        down_read(&mm->mmap_sem);

        for (vma = mm->mmap; vma; vma = vma->vm_next)
        {
            printk("mem[%lx-%lx] code[%lx-%lx] data[%lx-%lx] heap[%lx-%lx] %s",
                   vma->vm_start, vma->vm_end,
                   mm->start_code, mm->end_code,
                   mm->start_data, mm->end_data,
                   mm->brk, mm->start_brk,
                   vma->vm_file ? d_path(&vma->vm_file->f_path, buffer, PATH_MAX) : "");
        }

        up_read(&mm->mmap_sem);
    }

    printk("##############################################################\n");

    return 0;
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
    real_ptrace = syscall_table[__NR_ftrace];
    syscall_table[__NR_ftrace] = file_varea;
    return 0;
}

static void __exit hooking_exit(void)
{
    syscall_table[__NR_ftrace] = real_ptrace;
    make_ro(syscall_table);
}

module_init(hooking_init);
module_exit(hooking_exit);
MODULE_LICENSE("GPL");
