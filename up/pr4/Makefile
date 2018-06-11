TARGETS = test1 test2 t1 t2
SOURCES = t1.c add.c sub.c mul.c div.c 
OBJECTS = t1.o add.o sub.o mul.o div.o 

all : $(TARGETS) 

test1 : test1.c
	gcc -o test1 test1.c

all : test1 t1

t1 : $(SOURCES) 
	gcc -o t1 $(SOURCES)	

all : test1 t1 test2

test2 : test1.o
	gcc -o test2 test1.o

test1.o : test1.c
	gcc -c test1.c
	
all : test1 t1 test2 t2

t2 : $(OBJECTS)
	gcc -o t2 $(OBJECTS)	

t1.o : t1.c 
	gcc -c t1.c

add.o : add.c add.h
	gcc -c add.c

sub.o : sub.c sub.h
	gcc -c sub.c

mul.o : mul.c mul.h
	gcc -c mul.c

div.o : div.c div.h
	gcc -c div.c

clean :
	rm -f $(TARGETS) 

