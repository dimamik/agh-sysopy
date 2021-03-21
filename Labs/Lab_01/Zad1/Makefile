all: static shared clear_temp

static:
	make clear_all
	gcc -Wall -c mylib.c
	ar crs libmylib.a mylib.o

shared:
	make clear_all
	gcc -Wall -c mylib.c
	gcc -Wall -shared -fPIC -o libmylib.so mylib.o

clear_all:
	rm -f *.o *.a *.so main
clear_temp:
	rm -f *.o