objects = main.o
binary = main

main: main.o
	gcc -o $(binary) $(objects) -lasound

main.o: main.c

clean:
	rm $(binary) $(objects)
