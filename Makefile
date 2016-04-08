all: watercounter

watercounter: main.o
	$(LD) -o watercounter main.o $(LDFLAGS) -l event_core -l c

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

clean:
	rm -rf *.o watercounter
