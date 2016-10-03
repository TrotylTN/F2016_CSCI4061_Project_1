make4061: main.o util.o
	gcc -o make4061 main.o util.o

main.o: main.c util.h
	gcc -c main.c

util.o: util.c util.h
	gcc -c util.c

clean:
	rm make4061 main.o util.o
