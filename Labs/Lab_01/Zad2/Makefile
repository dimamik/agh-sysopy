shared_tests:
	make shared
	make tests

static_tests:
	make static
	make tests

static:
	make clear_all
	gcc -Wall -c mylib.c
	ar crs libmylib.a mylib.o

	gcc -Wall main.c -o main -L. -lmylib
	make clear_temp

shared:
	make clear_all
	gcc -Wall -c mylib.c

	gcc -Wall -shared -fPIC -o libmylib.so mylib.o

	gcc main.c -Wall -o main -L. -lmylib -Wl,-rpath=`pwd`

clear_all:
	rm -f *.o *.a *.so main
clear_temp:
	rm -f *.o
tests:
	rm -f zad2_report.txt
	# Merge Testing With 3 pairs
	./main merge_small_files create_table 3 merge_files ../src_test/1_small.txt:../src_test/2_small.txt \
		../src_test/1_small.txt:../src_test/2_small.txt ../src_test/1_small.txt:../src_test/2_small.txt
	./main merge_middle_files create_table 3 merge_files ../src_test/1_middle.txt:../src_test/2_middle.txt \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt
	./main merge_large_files create_table 3 merge_files ../src_test/1_large.txt:../src_test/2_large.txt \
		../src_test/1_large.txt:../src_test/2_large.txt ../src_test/1_large.txt:../src_test/2_large.txt
	
	# Merge 10 medium pairs
	
	./main merge_many_medium_files create_table 25 \
		merge_files \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt \

	# Delete blocks of different sizes
	./main delete_medium_files create_table 25 \
		merge_files \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt \
		remove_block 1 remove_block 0

	./main delete_large_files create_table 25 \
		merge_files \
		../src_test/1_large.txt:../src_test/2_large.txt ../src_test/1_large.txt:../src_test/2_large.txt \
		remove_block 1 remove_block 0
	
	# All together on middle file
	./main all_together_middle create_table 25 \
		merge_files \
		../src_test/1_middle.txt:../src_test/2_middle.txt ../src_test/1_middle.txt:../src_test/2_middle.txt \
		remove_block 1 merge_files ../src_test/1_middle.txt:../src_test/2_middle.txt remove_row 0 5 \
		remove_block 2 merge_files ../src_test/1_middle.txt:../src_test/2_middle.txt\
		merge_files ../src_test/1_middle.txt:../src_test/2_middle.txt\
		remove_block 0
	
	./main all_together_large create_table 25 \
		merge_files \
		../src_test/1_large.txt:../src_test/2_large.txt ../src_test/1_large.txt:../src_test/2_large.txt \
		remove_block 1 merge_files ../src_test/1_large.txt:../src_test/2_large.txt remove_row 0 5 \
		remove_block 2 merge_files ../src_test/1_large.txt:../src_test/2_large.txt\
		merge_files ../src_test/1_large.txt:../src_test/2_large.txt\
		remove_block 0
	echo "All tests passed :) \
	Results are in zad2_report.txt\n\n"

