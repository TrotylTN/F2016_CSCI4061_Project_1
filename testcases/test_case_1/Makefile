all: a.o b.o
	gcc -o all a.c

b.o: c.o b.c
	gcc -o b.o b.c

c.o: d.o c.c
	gcc -o c.o c.c

a.o: d.o a.c
	gcc -o a.o a.c

d.o: e.o d.c
	gcc -o d.o d.c

e.o: e.c
	gcc -o e.o e.c
