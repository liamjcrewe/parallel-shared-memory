all:
	gcc src/main.c src/arrays/arrays.c src/debug/debug.c -o bin/solve
balena:
	gcc -std=c99 -pthread src/main.c src/arrays/arrays.c src/debug/debug.c -o bin/solve
debug:
	gcc -g -Wall src/main.c src/arrays/arrays.c src/debug/debug.c -o bin/solve
clean:
	rm bin/solve; rm -rf bin/solve.dSYM/
