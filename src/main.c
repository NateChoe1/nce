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
#include <nce/util.h>

static int search(pid_t pid, int argc, char **argv);
static int prune(pid_t pid, int argc, char **argv);
static int set(pid_t pid, int argc, char **argv);
static int printlegal(pid_t pid, int argc, char **argv);

static int parsedata(char *data, void **dataret, size_t *datalen);

#define COMMAND(name) {#name, name},
struct {
	char *name;
	int (*function)(pid_t pid, int argc, char **argv);
} commands[] = {
	COMMAND(search)
	COMMAND(prune)
	COMMAND(set)
	COMMAND(printlegal);
};
/* It's beautiful. I love it. */

int main(int argc, char **argv) {
	int i;
	pid_t pid;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s [pid] [command]\n", argv[0]);
		fputs("List of valid commands:\n"
			"search\n"
			"prune\n"
			"set\n", stderr);
		return 1;
	}

	pid = atoi(argv[1]);

	for (i = 0; i < sizeof commands / sizeof commands[0]; ++i) {
		if (strcmp(argv[2], commands[i].name) == 0)
			return commands[i].function(pid, argc - 2, argv + 2);
	}
	fprintf(stderr, "Invalid command %s\n", argv[1]);
	return 1;
}

static int search(pid_t pid, int argc, char **argv) {
	Address *addrs;
	int addrslen;
	struct Program *program;
	void *needle;
	size_t needlelen;
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

	if (parsedata(argv[1], &needle, &needlelen))
		return 1;

	program = newprogram(pid);
	if (program == NULL) {
		fprintf(stderr, "Couldn't create program\n");
		return 1;
	}
	addrs = searchprogram(program, &addrslen, needle, needlelen);

	for (i = 0; i < addrslen; ++i) {
		printf("%lx\n", addrs[i]);
		if (scratch != NULL)
			fprintf(scratch, "%lx\n", addrs[i]);
	}

	free(addrs);
	freeprogram(program);
	return 0;
}

static int prune(pid_t pid, int argc, char **argv) {
	struct Program *program;
	Address *addrs;
	int addrslen;
	void *needle;
	size_t needlelen;
	int i;
	FILE *infile, *outfile;
	/*
	 * fopen(path, "w") will truncate a file, so we have to open a scratch
	 * file twice when pruning it, the first time to read the contents, and
	 * the second time to write the data. Also, if there is no scratch file,
	 * we should use stdin and stdout.
	 * */

	if (argc < 2) {
		printf("Usage: %s [needle] (scratch file)\n", argv[0]);
		return 1;
	}

	if (parsedata(argv[1], &needle, &needlelen))
		return 1;

	if (argc >= 3) {
		infile = fopen(argv[2], "r");
		if (infile == NULL) {
			fprintf(stderr, "Couldn't open file %s for reading\n",
					argv[2]);
			return 1;
		}
	}
	else
		infile = stdin;

	program = newprogram(pid);

	if (program == NULL) {
		fprintf(stderr, "Couldn't create program\n");
		return 1;
	}

	addrs = parseaddrs(infile, &addrslen);

	if (infile != stdin)
		fclose(infile);

	if (argc >= 3)
		outfile = fopen(argv[2], "w");
	else
		outfile = NULL;

	for (i = 0; i < addrslen; ++i) {
		if (stillgood(program, addrs[i], needle, needlelen)) {
			if (outfile != NULL)
				fprintf(outfile, "%lx\n", addrs[i]);
			printf("%lx\n", addrs[i]);
		}
	}

	free(addrs);
	if (outfile != NULL)
		fclose(outfile);
	freeprogram(program);
	return 0;
}

static int set(pid_t pid, int argc, char **argv) {
	Address *addrs;
	int addrslen;
	int i;
	void *newval;
	size_t newvallen;
	FILE *scratch;
	struct Program *program;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s [new value] (scratch file)\n",
				argv[0]);
		return 1;
	}

	program = newprogram(pid);
	if (program == NULL) {
		fprintf(stderr, "Couldn't create program\n");
		return 1;
	}

	if (parsedata(argv[1], &newval, &newvallen))
		return 1;

	if (argc >= 3) {
		scratch = fopen(argv[2], "r");
		if (scratch == NULL) {
			fprintf(stderr, "Failed to open %s for writing\n",
					argv[2]);
			return 1;
		}
	}
	else
		scratch = stdin;

	addrs = parseaddrs(scratch, &addrslen);

	for (i = 0; i < addrslen; ++i) {
		if (setaddr(program, addrs[i], newval, newvallen)) {
			fprintf(stderr, "Failed to set memory address %lx\n",
					addrs[i]);
		}
	}
	return 0;
}

static int printlegal(pid_t pid, int argc, char **argv) {
	puts("nce - a program to change memory values of linux programs");
	puts("This program is free software, you can redistribute it and/or");
	puts("modify it under the terms of the GNU General Public License as");
	puts("published by the Free Software Foundation, either version 3 of");
	puts("the Licnese, or (at your option) any later version.");
	return 0;
}

static int parsedata(char *data, void **dataret, size_t *datalen) {
#define NUM_TYPE(func, c, type) \
	case c: \
		*dataret = malloc(sizeof(type)); \
		* ((type *) *dataret) = func(data + 1); \
		* datalen = sizeof(type); \
		return 0;
	switch (data[0]) {
	NUM_TYPE(atoi, 'b', unsigned char)  /* byte */
	NUM_TYPE(atoi, 'w', unsigned short) /* word */
	NUM_TYPE(atoi, 'l', unsigned int)   /* long */
	NUM_TYPE(atoi, 'q', unsigned long)  /* quad */
	NUM_TYPE(atof, 'f', float)          /* float */
	NUM_TYPE(atof, 'd', double)         /* double */
	case 's': {
		size_t len = strlen(data);	
		*dataret = malloc(len);
		memcpy(*dataret, data + 1, len);
		*datalen = len;
		return 0;
	}
	default:
		return 1;
	}
}
