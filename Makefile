SRC = $(wildcard src/*.c)
OBJ = $(subst .c,.o,$(subst src,work,$(SRC)))
LDFLAGS =
CFLAGS := -O2 -pipe -Wall -Wpedantic -Wshadow -ansi
CFLAGS += -Isrc/
INSTALLDIR := /usr/sbin/
OUT = nce

build/$(OUT): $(OBJ)
	$(CC) $(OBJ) -o build/$(OUT) $(LDFLAGS)

work/%.o: src/%.c $(wildcard src/include/*.h)
	$(CC) $(CFLAGS) $< -c -o $@

install: build/$(OUT)
	cp build/$(OUT) $(INSTALLDIR)/$(OUT)

uninstall: $(INSTALLDIR)/$(OUT)
	rm $(INSTALLDIR)/$(OUT)
