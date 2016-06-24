all: bin bin/unpak

bin:
	mkdir bin/

bin/unpak: bin/main.o bin/pak.o
	$(CC) -o bin/unpak bin/main.o bin/pak.o

bin/main.o: main.c
	$(CC) -o bin/main.o -c main.c

bin/pak.o: pak.c
	$(CC) -o bin/pak.o -c pak.c

clean:
	rm -rf bin/
