tests:
	make clear_res
	make clear_all
	gcc -Wall consumer.c -o consumer
	gcc -Wall producer.c -o producer
	gcc -Wall main.c -o main
	make many_to_many
	make one_to_many
	make many_to_one
	make clear_all

many_to_many:
	make clear_temp
	mkdir -p res/many_to_many/in
	mkdir -p res/many_to_many/out
	./main 10 5 50 5 many_to_many
	python3 script.py 10 5 50 5 many_to_many
	make clear_temp
	./main 10000 5 500000 5 many_to_many
	python3 script.py 10000 5 50000 5 many_to_many

one_to_many:
	make clear_temp
	mkdir -p res/one_to_many/in
	mkdir -p res/one_to_many/out
	./main 10 1 50 5 one_to_many
	python3 script.py 10 1 50 5 one_to_many
	make clear_temp
	./main 10000 5 500000 5 one_to_many
	python3 script.py 10000 5 500000 5 one_to_many

many_to_one:
	make clear_temp
	mkdir -p res/many_to_one/in
	mkdir -p res/many_to_one/out
	./main 10 5 50 1 many_to_one
	python3 script.py 10 5 50 1 many_to_one
	make clear_temp
	./main 10000 5 500000 5 many_to_one
	python3 script.py 10000 5 500000 5 many_to_one


clear_temp:
	rm -f pipe

clear_all:
	rm -f *.o *.a *.so main pipe

clear_res:
	rm -rf res