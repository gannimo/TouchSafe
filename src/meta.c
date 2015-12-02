/**
 * @file meta.c
 * Handles open and close system calls.
 *
 * Copyright (c) 2013 UC Berkeley
 * @author Mathias Payer <mathias.payer@nebelwelt.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "main.h"

#include "helper.h"
#include "meta.h"


long handle_open(struct process *proc, struct user_regs_struct *regs) {
    char filename[257];
    long sz;
    for (sz = 0; sz < 256; sz+=sizeof(long)) {
        long data = ptrace(PTRACE_PEEKDATA, proc->pid, regs->rdi+sz, NULL);
        memcpy(filename+sz, &data, sizeof(long));
        if (((data & 0xff) == 0) || ((data & 0xff00) == 0) ||
            ((data & 0xff0000) == 0) || ((data & 0xff000000) == 0))
            break;
    }
    if (sz == 256) filename[256] = 0;
    DEBUG("Open system call: str: %p (%s), flags %ld mode: %lx return: %ld (pid: %ld)\n",
          (void*)regs->rdi, filename, regs->rsi, regs->rdx, regs->rax, proc->pid);

    char *name = (char*)malloc(strlen(filename)+1);
    strcpy(name, filename);
    struct open_file *newfd = file_new(regs->rax, regs->rsi, name);
    file_insert(proc, newfd);
    return 0;
}

long handle_close(struct process *proc, struct user_regs_struct *regs) {
    struct open_file *fd;
    long lfd = regs->rdi;
    DEBUG("Close system call: fd %ld, return: %ld (pid: %ld)\n", lfd, regs->rax, proc->pid);

    fd = file_find(lfd, proc->files);
    if (fd != NULL) {
        proc->files = file_delete(fd, proc->files);
    } else {
        WARN("Warning: closed file w/o information (fd: %ld, pid: %ld)\n", lfd, proc->pid);
    }
    return 0;
}
