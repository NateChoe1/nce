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
#ifndef HAVE_MAPS
#define HAVE_MAPS

#include <sys/types.h>

#include <nce/types.h>

struct Device {
	unsigned char major;
	unsigned char minor;
};

#define READ   (1 << 0)
#define WRITE  (1 << 1)
#define EXEC   (1 << 2)
#define SHARED (1 << 3)

struct Map {
	Address start;
	Address end;
	/* It's far easier to use integer types than pointers for start and end
	 * locations. */
	unsigned char permissions;
	unsigned long offset;
	struct Device device;
	unsigned long inode;
	char *path;
};

struct Map *getmaps(pid_t pid, int *lenret);
void freemaps(struct Map *maps, int len);

#endif
