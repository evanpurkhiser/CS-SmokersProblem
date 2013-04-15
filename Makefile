all: bin/smoke

bin/smoke: smoke.c
	gcc -std=gnu99 -Wall -pedantic -pthread smoke.c -o bin/smoke

clean:
	rm bin/smoke
