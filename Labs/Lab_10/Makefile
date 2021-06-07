all:
	gcc -Wall common.h server.c -o server -lm -lpthread
	gcc -Wall common.h client.c -o client -lm -lpthread
	./server /tmp/address 8081