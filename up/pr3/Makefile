all: test1 t1 test2 t2

test1 : test1.c
	gcc -o test1 test1.c

t1: t1.c add.c sub.c mul.c div.c
	gcc -o t1 t1.c add.c sub.c mul.c div.c

test1: test1.o
	gcc -o test1 test1.o

test2.o: test1.c
	gcc -c test1.c

t2: t1.o add.o sub.o mul.o div.o
	gcc -o t2 t1.o add.o sub.o mul.o div.o

t1.o : t1.c
	gcc -c t1.c

add.o : add.c
	gcc -c add.c

sub.o: sub.c
	gcc -c sub.c

mul.o: mul.c
	gcc -c mul.c

div.o: div.c
	gcc -c div.c

clean:
	rm -f *.o test1 test2 t1 t2
