CC=gcc
CFLAGS=-I.
DEPS=
OBJ=server.o
USERID=105817312

all: server

server: server.c
	gcc -o server server.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean: // this is not right
	rm -rf *.o server *.tar.gz

dist: tarball
tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .