/**
 * @file main.h
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

//#define DEBUG(...) printf(__VA_ARGS__)
//#define INFO(...) printf(__VA_ARGS__)
//#define WARN(...) printf(__VA_ARGS__)
#define DEBUG(...) 
#define INFO(...)
#define WARN(...)

#define MAGIC_FILENAME "/OOWriter-TouchSafe"

struct process {
    long pid;
    long in_syscall;
    long dying;
    struct open_file *files;
    struct process *next;
};

struct open_file {
    long fd;
    enum {
        DSFILE,
        REGFILE,
        OTHER
    } status;
    long flags;
    long offset;
    char *name;
    struct open_file *next;
};
