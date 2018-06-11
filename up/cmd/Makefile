TARGETS = cmd cmdjmp

all : $(TARGETS)

cmdjmp: cmdjmp.c
	gcc -o cmdjmp cmdjmp.c

cmd : cmd.c
	gcc -o cmd cmd.c

clean:
	rm -f core $(TARGETS) 
