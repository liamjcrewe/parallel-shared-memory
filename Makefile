all:
	gcc main.c arrays.c debug.c -o solve
debug:
	gcc -g -Wall main.c arrays.c debug.c -o solve
