lib_test:
	make clear
	gcc -Wall main.c -o main -D LIB -lm
	./main lib
sys_test:
	make clear
	gcc -Wall main.c -o main -lm
	./main sys
clear:
	rm -f out.txt
