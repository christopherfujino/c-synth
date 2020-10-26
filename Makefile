objects = main.o
binary = main

main: main.o
	cc -o $(binary) $(objects) -lasound

debug: main.o
	cc -g -o $(binary) $(objects) -lasound

main.o: main.c

clean:
	rm $(binary) $(objects)
