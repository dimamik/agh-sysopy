tests:
	make clear_all
	make clear_temp
	touch output_consumer.txt
	gcc -Wall consumer.c -o consumer
	gcc -Wall producer.c -o producer
	gcc -Wall main.c -o main
	mkdir -p res/oneToMany
	./main 10 1 50 5 oneToMany
	make clear_all





clear_temp:
	rm -f output_consumer.txt

clear_all:
	rm -f *.o *.a *.so main pipe

