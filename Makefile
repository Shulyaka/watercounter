all: watercounter

watercounter: main.o
	$(CC) -o watercounter main.o

main.o: main.c
	$(CC) -c main.c

clean:
	rm -rf *.o watercounter
