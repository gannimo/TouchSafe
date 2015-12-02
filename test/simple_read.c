/**
 * @file simple_read.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	int fd, rd;
	char buf[81];
	if (argc != 2) {
		printf("Wrong number of arguments. Use: %s filename\n", argv[0]);
		return -1;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		printf("Could not read file '%s'\n", argv[1]);
		return -1;
	}

  //int f2 = open("./foo", O_WRONLY);
	while ((rd = read(fd, buf, 80)) != 0) {
		buf[rd] = 0;
		//write(f2, buf, rd);
		printf("Data: '%s'\n", buf);
	}

  //close(f2);
	close(fd);
	return 0;
}
