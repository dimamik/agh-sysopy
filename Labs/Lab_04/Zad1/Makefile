all:
	make non_exec
	make exec
non_exec:
	# Making non-exec
	gcc -Wall main.c -o main
	./main ignore
	./main handler
	./main mask 
	./main pending

exec:
	# Making exec
	gcc -Wall main.c -o main
	gcc -Wall to_execute.c -o to_execute
	./main ignore exec
	./main mask  exec
	./main pending exec
	# There we are waiting for exception because new executable doesn't have access to previous process handler
	./main handler exec
