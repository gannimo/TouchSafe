/**
 * @file main.c
 * Starts the domain manager and prepares the application.
 * Forwards system calls to the domain manager as well.
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
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "main.h"

#include "helper.h"
#include "io.h"
#include "meta.h"

void tracee(char *cmd, char *argv[]) {
    // prepare traced process
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    // stop process so that domain manager can attach
    kill(getpid(), SIGSTOP);
    
    DEBUG("Starting target process '%s'\n", cmd);
    
    // execute our target process
    execv(cmd, argv);

    printf("Unfortunately our 'execve' failed. Next time try a real program.\n");
    
    // this location is never reached.
}

void tracer() {
    int status;
    struct user_regs_struct regs;
    long syscall;
    long enter_syscall = 1;
    long init_tracee = 0;
    long nrprocs = 1;

    // list of all traced processes
    struct process *all_procs = NULL;
    
    // loop until the application starts
    // initializes steady state
    while (1) {
        pid_t program = wait(&status);
        if (!init_tracee) {
            // get information for first process
            all_procs = proc_new(program);
            
            // handle forks, vforks, and thread creation
            ptrace(PTRACE_SETOPTIONS, program, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK |
                   PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT);
            init_tracee = 1;
        }
        if (WIFEXITED(status)) {
            INFO("Child (%d) exit with status %d.\n", program, WEXITSTATUS(status));
            exit(-1);
        }
        if (WSTOPSIG(status) == (SIGTRAP|0x80)) {
            status = ptrace(PTRACE_GETREGS, program, NULL, &regs);
            if (status == 0) {
                syscall = regs.orig_rax;

                if (enter_syscall) {
                    enter_syscall = 0;
                } else {
                    enter_syscall = 1;
                    // successfully executed execve
                    if (syscall == 59 && regs.rax == 0) {
                        ptrace(PTRACE_SYSCALL, program, NULL, NULL);
                        break;
                    }
                }
            }
        }
        ptrace(PTRACE_SYSCALL, program, NULL, NULL);
    }

    // trace all system calls of the target application
    while (1) {
        pid_t program = waitpid(-1, &status, __WALL);
        
        if (WIFEXITED(status)) {
            INFO("Child (%d) exit with status %d.\n", program, WEXITSTATUS(status));
            struct process *proc = proc_find(program, all_procs);
            assert(proc != NULL);
            all_procs = proc_delete(proc, all_procs);
            nrprocs--;
            if (nrprocs == 0)
                break;
            else
                continue;
        }
        if (WIFSIGNALED(status)) {
            INFO("Child (%d) exit due to signal %d.\n", program, WTERMSIG(status));
            struct process *proc = proc_find(program, all_procs);
            assert(proc != NULL);
            all_procs = proc_delete(proc, all_procs);
            nrprocs--;
            if (nrprocs == 0)
                break;
            else
                continue;
        }
        if (!WIFSTOPPED(status)) {
            INFO("wait() returned unhandled status 0x%x for child (%d).\n", status, program);
            ptrace(PTRACE_SYSCALL, program, NULL, NULL);
            continue;
        }

        // tracee is forking
        if (((status >> 8) == (SIGTRAP | (PTRACE_EVENT_FORK<<8))) ||
            ((status >> 8) == (SIGTRAP | (PTRACE_EVENT_VFORK<<8))) ||
            ((status >> 8) == (SIGTRAP | (PTRACE_EVENT_CLONE<<8)))) {
            pid_t newpid;
            ptrace(PTRACE_GETEVENTMSG, program, NULL, &newpid);
            ptrace(PTRACE_SYSCALL, newpid, NULL, NULL);
            struct process *new = proc_new(newpid);
            all_procs = proc_insert(new, all_procs);
            nrprocs++;
            INFO("Child (%d) started new thread/process pid: %d total procs: %ld\n", program, newpid, nrprocs);
        } else
        // execve
        if (status>>8 == (SIGTRAP | (PTRACE_EVENT_EXEC<<8))) {
            INFO("Child (%d) is executing execve\n", program);
        } else
        // exit
        if (status>>8 == (SIGTRAP | (PTRACE_EVENT_EXIT<<8))) {
            INFO("Child (%d) is about to exit\n", program);
        } else
        // other system call
        if (WSTOPSIG(status) == (SIGTRAP|0x80)) {
            struct process *proc = proc_find(program, all_procs);
            assert(proc != NULL);
            status = ptrace(PTRACE_GETREGS, program, NULL, &regs);
            if (status == 0) {
                syscall = regs.orig_rax;

                // TODO: check if syscall allowed

                if (!proc->in_syscall) {
                    proc->in_syscall = 1;
                    switch (syscall) {
                        case SYS_write:
                        case SYS_pwrite64:
                            handle_write(proc, &regs);
                            break;
                    }
                } else {
                    proc->in_syscall = 0;
                    switch (syscall) {
                        case SYS_read:
                        case SYS_pread64:
                            handle_read(proc, &regs);
                            break;
                        case SYS_open:
                            handle_open(proc, &regs);
                            break;
                        case SYS_close:
                            handle_close(proc, &regs);
                            break;
                        case SYS_write:
                        case SYS_pwrite64:
                            handle_write(proc, &regs);
                            break;
                        default:
                            //DEBUG("Child (%d) completed syscall (%ld): 0x%lx.\n",
                            //      program, syscall, regs.rax);
                            break;
                    }
                }
            }
        } else {
            INFO("Child (%d) stopped due to signal %d\n", program, WSTOPSIG(status));
            // this is a new process and we don't have any information yet.
            // let's give the main thread a chance to catch the information about thread creation.
            struct process *proc = proc_find(program, all_procs);
            if (proc == NULL)
                continue;
        }
        ptrace(PTRACE_SYSCALL, program, NULL, NULL);
    }
}

int main(int argc, char* argv[]) {
	pid_t program;
	
	if (argc < 2) {
		printf("%s user_program [params]\n", argv[0]);
		return -1;
	}

	program = fork();
	if (program == 0) {
		tracee(argv[1], &argv[1]);
	} else {
        tracer();
	}
	return 0;
}
