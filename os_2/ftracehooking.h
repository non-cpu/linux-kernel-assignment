#ifndef FTRACEHOOKING_H
#define FTRACEHOOKING_H

#include <linux/module.h>

extern asmlinkage int ftrace_io(const struct pt_regs *regs);

#endif
