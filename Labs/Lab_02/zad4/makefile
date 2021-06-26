lib_test:
	make clear
	gcc -Wall main.c -o main -D LIB -lm
	./main lib "in.txt" "out.txt" "ALA" "BYE"
sys_test:
	make clear
	gcc -Wall main.c -o main -lm
	./main sys "in.txt" "out.txt" "ALA" "BYE"
clear:
	rm -f out.txt
