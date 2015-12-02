/**
 * @file io.c
 * Handles a read system call.
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
#include <sys/syscall.h>

#include "main.h"

#include "io.h"
#include "helper.h"

/**
 * handles both read and pread (w/ offset)
 */
long handle_read(struct process *proc, struct user_regs_struct *regs) {
    DEBUG("Read system call: fd: %ld buf: %p sz: %ld return: %ld\n", regs->rdi, (void*)regs->rsi, regs->rdx, regs->rax);
    long sz = regs->rax;
    long lfd = regs->rdi;
    char *base = (char*)(regs->rsi);
    long offset = 0;
    struct open_file *fd = file_find(lfd, proc->files);

    // no need to decrypt or handle anything if there is no data read
    if (sz <= 0) return 0;

    if (fd == NULL) {
        WARN("Warning: no information found about open fd (%ld)!\n", lfd);
        return 0;
    }
    if (fd->status != DSFILE) {
        return 0;
    }
    DEBUG("XXX-> decrypting (read)! \n");

    switch (regs->orig_rax) {
        case SYS_read:
            // update file offset
            offset = fd->offset;
            fd->offset += sz;
            break;
        case SYS_pread64:
            // adjust to given offset
            offset = regs->r10;
    }

    char *buf = (char*)malloc(((sz + sizeof(long) - 1) & (-sizeof(long))));

    // read data from tracee
    long i = 0;
    while (i < sz) {
        long data = ptrace(PTRACE_PEEKDATA, proc->pid, base+i, NULL);
        memcpy(buf+i, &data, sizeof(long));
        i+=sizeof(long);
    }

    // TODO: super secret crypto
    for (i = 0; i < sz; ++i) {
        switch ((i+offset) % 4) {
            case 0:
                buf[i] = 0xde ^ buf[i];
                break;
            case 1:
                buf[i] = 0xad ^ buf[i];
                break;
            case 2:
                buf[i] = 0xbe ^ buf[i];
                break;
            case 3:
                buf[i] = 0xef ^ buf[i];
                break;
        }
    }

    
    // write data to tracee
    i = 0;
    while (i < sz) {
        long data;
        memcpy(&data, buf+i, sizeof(long));
        ptrace(PTRACE_POKEDATA, proc->pid, base+i, data);
        i+=sizeof(long);
    }

    free(buf);
    return 0;
}

long handle_write(struct process *proc, struct user_regs_struct *regs) {
    DEBUG("Write system call: fd: %ld buf: %p sz: %ld return: %ld\n", regs->rdi, (void*)regs->rsi, regs->rdx, regs->rax);
    long sz = regs->rdx;
    long lfd = regs->rdi;
    char *base = (char*)(regs->rsi);
    long offset;
    struct open_file *fd = file_find(lfd, proc->files);

    // no need to decrypt or handle anything if there is no data read
    if (sz <= 0) return 0;

    if (fd == NULL) {
        WARN("Warning: no information found about open fd (%ld)!\n", lfd);
        return 0;
    }
    if (fd->status != DSFILE) {
        return 0;
    }

    if (proc->in_syscall == 1) {
        DEBUG("XXX-> encrypting (write)! \n");
    } else {
        DEBUG("XXX-> decrypting (write)! \n");
    }

    switch (regs->orig_rax) {
        case SYS_write:
            // update file offset
            offset = fd->offset;
            if (proc->in_syscall == 0)
                fd->offset += sz;
            break;
        case SYS_pread64:
            // adjust to given offset
            offset = regs->r10;
    }

    char *buf = (char*)malloc(((sz + sizeof(long) - 1) & (-sizeof(long))));

    // read data from tracee
    long i = 0;
    while (i < sz) {
        long data = ptrace(PTRACE_PEEKDATA, proc->pid, base+i, NULL);
        memcpy(buf+i, &data, sizeof(long));
        i+=sizeof(long);
    }

    // TODO: super secret crypto
    for (i = 0; i < sz; ++i) {
        switch ((i+offset) % 4) {
            case 0:
                buf[i] = 0xde ^ buf[i];
                break;
            case 1:
                buf[i] = 0xad ^ buf[i];
                break;
            case 2:
                buf[i] = 0xbe ^ buf[i];
                break;
            case 3:
                buf[i] = 0xef ^ buf[i];
                break;
        }
    }

    
    // write data to tracee
    i = 0;
    while (i < sz) {
        long data;
        memcpy(&data, buf+i, sizeof(long));
        ptrace(PTRACE_POKEDATA, proc->pid, base+i, data);
        i+=sizeof(long);
    }

    free(buf);
    return 0;
}
