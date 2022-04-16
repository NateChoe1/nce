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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <nce/maps.h>

static int getmap(FILE *mapfile, struct Map *ret);

struct Map *getmaps(pid_t pid, int *lenret) {
	size_t alloc, len;
	struct Map *ret;
	FILE *mapfile;
	char path[30];
	alloc = 5;
	ret = malloc(sizeof(struct Map) * alloc);

	sprintf(path, "/proc/%d/maps", pid);

	path[sizeof path - 1] = '\0';
	mapfile = fopen(path, "r");

	if (mapfile == NULL) {
		free(ret);
		return NULL;
	}

	for (len = 0;; ++len) {
		if (len >= alloc) {
			struct Map *newret;
			alloc *= 2;
			newret = realloc(ret, sizeof(struct Map) * alloc);
			if (newret == NULL) {
				free(ret);
				return NULL;
			}
			ret = newret;
		}
		if (getmap(mapfile, ret + len))
			break;
	}
	*lenret = len;
	return ret;
}

void freemaps(struct Map *maps, int len) {
	int i;
	if (maps == NULL)
		return;
	for (i = 0; i < len; ++i)
		free(maps[i].path);
	free(maps);
}

static int ctohex(int c) {
	if (isdigit(c))
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static unsigned long gethexval(FILE *file, char stop, int *status) {
	register unsigned long ret;
	int c;
	ret = 0;
	while ((c = fgetc(file)) != stop) {
		int val;
		val = ctohex(c);
		if (val < 0) {
			*status = 1;
			return 0;
		}
		ret <<= 4;
		ret |= val;
	}
	*status = 0;
	return ret;
}

static unsigned long getdecval(FILE *file, char stop, int *status) {
	register unsigned long ret;
	int c;
	ret = 0;
	while ((c = fgetc(file)) != stop) {
		if (!isdigit(c)) {
			*status = 1;
			return 0;
		}

		ret <<= 4;
		ret += c - '0';
	}
	*status = 0;
	return ret;
}

static int getmap(FILE *mapfile, struct Map *ret) {
	int status;
	int c;

	ret->start = gethexval(mapfile, '-', &status);
	if (status)
		return 1;
	ret->end = gethexval(mapfile, ' ', &status);
	if (status)
		return 1;

	ret->permissions = 0;
	ret->permissions |= fgetc(mapfile) == 'r' ? READ   : 0;
	ret->permissions |= fgetc(mapfile) == 'w' ? WRITE  : 0;
	ret->permissions |= fgetc(mapfile) == 'x' ? EXEC   : 0;
	ret->permissions |= fgetc(mapfile) == 's' ? SHARED : 0;
	/* TODO: Make getting permissions check if the map file is valid */
	if (fgetc(mapfile) != ' ')
		return 1;

	ret->offset = gethexval(mapfile, ' ', &status);
	if (status)
		return 1;

	ret->device.major = gethexval(mapfile, ':', &status);
	if (status)
		return 1;
	ret->device.minor = gethexval(mapfile, ' ', &status);
	if (status)
		return 1;

	ret->inode = getdecval(mapfile, ' ', &status);
	if (status)
		return 1;

	for (;;) {
		c = fgetc(mapfile);
		if (c == '\n') {
			ret->path = NULL;
			return 0;
		}
		if (c == EOF)
			return 1;
		if (c != ' ') {
			ungetc(c, mapfile);
			break;
		}
	}

	{
		size_t alloc, len;
		alloc = 30;
		ret->path = malloc(alloc);
		len = 0;
		for (;;) {
			c = fgetc(mapfile);
			if (c == EOF) {
				free(ret->path);
				return 1;
			}

			if (len >= alloc) {
				char *newpath;
				alloc *= 2;
				newpath = realloc(ret->path, alloc);
				if (newpath == NULL) {
					free(ret->path);
					return 1;
				}
				ret->path = newpath;
			}
			if (c == '\n') {
				ret->path[len] = '\0';
				break;
			}
			ret->path[len++] = c;
		}
	}
	return 0;

}
