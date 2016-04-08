all: watercounter

watercounter: main.o
	$(CC) -o watercounter main.o -l event_core

main.o: main.c
	$(CC) -c main.c

clean:
	rm -rf *.o watercounter
