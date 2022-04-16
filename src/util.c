#include <stdlib.h>

#include <nce/util.h>

static int trueeof(FILE *file);

Address *parseaddrs(FILE *file, int *lenret) {
	Address *ret;
	size_t alloc, len;
	alloc = 50;
	ret = malloc(sizeof(Address) * alloc);
	if (ret == NULL)
		return NULL;
	len = 0;

	for (;;) {
		if (trueeof(file))
			break;
		if (len >= alloc) {
			Address *newret;
			alloc *= 2;
			newret = realloc(ret, sizeof(Address) * alloc);
			if (newret == NULL)
				goto error;
			ret = newret;
		}
		if (fscanf(file, "%lx\n", &ret[len++]) < 1)
			goto error;
	}
	*lenret = len;
	return ret;
error:
	free(ret);
	return NULL;
}

static int trueeof(FILE *file) {
	int c;
	c = fgetc(file);
	if (c == EOF)
		return 1;
	ungetc(c, file);
	return 0;
}
