all:
	gcc main.c arrays.c debug.c -o solve
balena:
	gcc -std=c99 -pthread main.c arrays.c debug.c -o solve
debug:
	gcc -g -Wall main.c arrays.c debug.c -o solve
clean:
	rm solve
