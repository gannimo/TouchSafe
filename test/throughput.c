/**
 * @file throughput.c
 * Measures read or write throughput
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 1024

int main(int argc, char *argv[]) {
	int fd, rd, i;
	char buf[BUFSIZE];
	if (argc != 3) {
		printf("Wrong number of arguments. Use: %s [read|write] filename\n", argv[0]);
		return -1;
	}

    if (strcmp(argv[1], "read") == 0) {
        fd = open(argv[2], O_RDONLY);
        if (fd == -1) {
            printf("Could not read file '%s'\n", argv[1]);
            return -1;
        }
        while ((rd = read(fd, buf, BUFSIZE)) != 0);
        close(fd);
    }

    if (strcmp(argv[1], "write") == 0) {
        for (i = 0; i < BUFSIZE; i++) buf[i] = i;
        fd = open(argv[2], O_WRONLY);
        if (fd == -1) {
            printf("Could not write file '%s'\n", argv[2]);
            return -1;
        }
        for (i = 0; i < 1024*1024*1000/BUFSIZE; i++)
            rd = write(fd, buf, BUFSIZE);
        close(fd);
    }

	return 0;
}
