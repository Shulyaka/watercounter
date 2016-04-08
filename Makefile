all: watercounter

watercounter: main.o
	$(CC) -o watercounter main.o $(LDFLAGS) -l event

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm -rf *.o watercounter
