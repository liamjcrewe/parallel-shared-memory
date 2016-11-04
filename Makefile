all:
	gcc src/main.c src/array/array.c src/debug/debug.c src/problem/problem.c src/solve/solve.c -o bin/solve
balena:
	gcc -std=c99 -pthread src/main.c src/array/array.c src/debug/debug.c src/problem/problem.c src/solve/solve.c -o bin/solve
debug:
	gcc -g -Wall src/main.c src/array/array.c src/debug/debug.c src/problem/problem.c src/solve/solve.c -o bin/solve
clean:
	rm bin/solve; rm -rf bin/solve.dSYM/
