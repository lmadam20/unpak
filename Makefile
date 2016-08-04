# Selects the compiler used
# default: gcc
CC = gcc

# Selects the command for deleting files & folders
RM = rm

# Selects the command for creating a directory
MKDIR = mkdir


all: bin bin/unpak

bin:
	$(MKDIR) bin/

bin/unpak: bin/main.o bin/pak.o
	$(CC) -o bin/unpak bin/main.o bin/pak.o

bin/main.o: main.c
	$(CC) -o bin/main.o -c main.c

bin/pak.o: pak.c
	$(CC) -o bin/pak.o -c pak.c

clean:
	$(RM) -r bin/
