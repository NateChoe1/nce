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
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <nce/maps.h>
#include <nce/proc.h>

static int search(pid_t pid, int argc, char **argv);

int main(int argc, char **argv) {
	pid_t pid;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s [pid] [command]\n", argv[0]);
		fputs("List of valid commands:\n"
			"search\n"
			"prune\n"
			"set\n", stderr);
		return 1;
	}

	pid = atoi(argv[1]);

	if (strcmp(argv[2], "search") == 0)
		return search(pid, argc - 2, argv + 2);
	return 1;
}

static int search(pid_t pid, int argc, char **argv) {
	Address *addrs;
	int addrslen;
	struct Program *program;
	int needle;
	int i;
	FILE *scratch;

	if (argc < 2) {
		printf("Usage: %s [needle] (scratch file)\n", argv[0]);
		return 1;
	}
	if (argc >= 3)
		scratch = fopen(argv[2], "w");
	else
		scratch = NULL;

	needle = atoi(argv[1]);

	program = newprogram(pid);
	if (program == NULL) {
		fprintf(stderr, "Couldn't create program\n");
		return 1;
	}
	addrs = searchprogram(program, &addrslen, &needle, sizeof needle);

	for (i = 0; i < addrslen; ++i) {
		printf("%lx\n", addrs[i]);
		if (scratch != NULL)
			fprintf(scratch, "%lx\n", addrs[i]);
	}

	free(addrs);
	freeprogram(program);
	return 0;
}
