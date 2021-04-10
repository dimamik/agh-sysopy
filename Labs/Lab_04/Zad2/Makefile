all:
	gcc -Wall main.c -o main
	./main siginfo value
	./main siginfo child
	./main siginfo alarm
	./main SA_RESTART__SA_NOCLDSTOP__SA_NOCLDWAIT
