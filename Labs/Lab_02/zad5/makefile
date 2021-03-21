lib_test:
	make clear
	gcc -Wall main.c -o main -D LIB
	./main lib "in.txt" "out.txt"
sys_test:
	make clear
	gcc -Wall main.c -o main
	./main sys "in.txt" "out.txt"
clear:
	rm -f out.txt
