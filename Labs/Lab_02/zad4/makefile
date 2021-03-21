lib_test:
	make clear
	gcc -Wall main.c -o main -D LIB -lm
	./main lib "in.txt" "out.txt" "This needs to be replaced" "YOHOOO REPLACED TEXT"
sys_test:
	make clear
	gcc -Wall main.c -o main -lm
	./main sys "in.txt" "out.txt" "This needs to be replaced" "YOHOOO REPLACED TEXT"
clear:
	rm -f out.txt
