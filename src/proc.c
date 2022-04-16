/*
 * nce - A program to change memory values of linux programs
 * Copyright (C) 2022  Nate Choe
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <nce/maps.h>
#include <nce/proc.h>

static int circularequal(char *buff, char *comp, int start, int len);

struct Program *newprogram(pid_t pid) {
	struct Program *ret;
	char mempath[30];
	ret = malloc(sizeof(struct Program));
	if ((ret->maps = getmaps(pid, &ret->maplen)) == NULL) {
		free(ret);
		return NULL;
	}
	sprintf(mempath, "/proc/%d/mem", pid);
	if ((ret->memfd = open(mempath, O_RDWR)) < 0) {
		freemaps(ret->maps, ret->maplen);
		free(ret);
		return NULL;
	}
	return ret;
}

void freeprogram(struct Program *program) {
	close(program->memfd);
	freemaps(program->maps, program->maplen);
	free(program);
}

Address *searchprogram(struct Program *program, int *lenret,
		void *needle, size_t needlelen) {
	size_t alloc;
	int len;
	Address *ret;

	char *buff;
	int buffstart;
	/* buff is a circular memory buffer containing the current thing */

	int i;

	len = 0;
	alloc = 5;
	ret = malloc(sizeof(Address) * alloc);
	if (ret == NULL)
		return NULL;

	buff = malloc(needlelen);
	if (buff == NULL) {
		free(ret);
		return NULL;
	}

	for (i = 0; i < program->maplen; ++i) {
		size_t readlen;
		size_t maplen;
		if (program->maps[i].path == NULL)
			continue;
		if (strcmp(program->maps[i].path, "[heap]") &&
		    strcmp(program->maps[i].path, "[stack]"))
			continue;
		/* We only want heaps and stacks, everything else is probably
		 * only used by people making games in assembly. */

		if (lseek(program->memfd, program->maps[i].start, SEEK_SET)
				== -1)
			goto error;

		readlen = needlelen - 1;
		if (read(program->memfd, buff, readlen) < readlen)
			goto error;

		buffstart = readlen;
		maplen = program->maps[i].end - program->maps[i].start;

		while (readlen < maplen) {
			if (read(program->memfd, buff + buffstart, 1) < 1)
				goto error;
			if (++buffstart >= needlelen)
				buffstart = 0;
			++readlen;
			if (circularequal(buff, (char *) needle,
						buffstart, needlelen)) {
				if (len >= alloc) {
					Address *newret;
					alloc *= 2;
					newret = realloc(ret,
						sizeof(Address) * alloc);
					if (newret == NULL)
						goto error;
					ret = newret;
				}
				ret[len++] = program->maps[i].start +
					readlen - needlelen;
			}
		}
	}
	free(buff);
	*lenret = len;
	return ret;
error:
	free(ret);
	free(buff);
	return NULL;
}

int setaddr(struct Program *program, Address addr, void *data, size_t len) {
	if (lseek(program->memfd, addr, SEEK_SET) == -1)
		return 1;
	if (write(program->memfd, data, len) < len)
		return 1;
	return 0;
}

static int circularequal(char *buff, char *comp, int start, int len) {
	int i;
	if (start >= len)
		return 0;
	for (i = 0; i < len; ++i) {
		if (buff[start] != comp[i])
			return 0;
		if (++start == len)
			start = 0;
	}
	return 1;
}
