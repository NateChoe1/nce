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
#ifndef HAVE_PROC
#define HAVE_PROC

#include <nce/maps.h>
#include <nce/types.h>

struct Program {
	int memfd;
	struct Map *maps;
	int maplen;
};

struct Program *newprogram(pid_t pid);
void freeprogram(struct Program *program);
Address *searchprogram(struct Program *program, int *lenret,
		void *needle, size_t needlelen);
int setaddr(struct Program *program, Address addr, void *data, size_t len);
int stillgood(struct Program *program, Address addr, void *data, size_t len);
/* Checks if the memory at address addr still matches data. */

#endif
