all: helloworld

helloworld: main.o
	$(CC) -o helloworld main.o

main.o: main.c
	$(CC) -c main.c

clean:
	rm -rf *.o helloworld
