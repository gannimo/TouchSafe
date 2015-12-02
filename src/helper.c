/**
 * @file helper.c
 * Handles virtual TouchSafe processes.
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

#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "helper.h"

struct process* proc_find(pid_t pid, struct process *all) {
    struct process *cur = all;
    while (cur != NULL) {
        if (cur->pid == pid)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

struct process* proc_insert(struct process *new, struct process *all) {
    new->next = all;
    return new;
}

struct process* proc_delete(struct process *del, struct process *all) {
    struct process *cur, *prev = NULL;
    for (cur = all; cur != NULL; prev = cur, cur = cur->next) {
        if (cur == del) {
            if (prev == NULL) {
                all = cur->next;
            } else {
                prev->next = cur->next;
            }
            while (cur->files != NULL) {
                cur->files = file_delete(cur->files, cur->files);
            }
            free(cur);
            return all;
        }
    }
    return NULL;
}

struct process* proc_new(pid_t pid) {
    struct process *proc = (struct process*)malloc(sizeof(struct process));
    proc->next = NULL;
    proc->pid = pid;
    proc->in_syscall = 0;
    proc->files = NULL;
    return proc;
}

struct open_file* file_new(long fd, long flags, char* name) {
    struct open_file *newfd = (struct open_file*)malloc(sizeof(struct open_file));
    newfd->fd = fd;
    newfd->name = name;
    newfd->flags = flags;
    newfd->offset = 0;
    if (strstr(name, MAGIC_FILENAME) != NULL) {
        newfd->status = DSFILE;
    } else {
        newfd->status = REGFILE;
    }
    return newfd;
}

struct open_file* file_insert(struct process *proc, struct open_file* file) {
    file->next = proc->files;
    proc->files = file;
    return file;

}

struct open_file* file_delete(struct open_file* del, struct open_file *all) {
    struct open_file *cur, *prev = NULL;
    for (cur = all; cur != NULL; prev = cur, cur = cur->next) {
        if (cur == del) {
            if (prev == NULL) {
                all = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur->name);
            free(cur);
            return all;
        }
    }
    return NULL;
}

struct open_file* file_find(long fd, struct open_file *all) {
    struct open_file *cur = all;
    while (cur != NULL) {
        if (cur->fd == fd)
            return cur;
        cur = cur->next;
    }
    return NULL;
}
