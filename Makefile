all: bin/smoke

bin/smoke: smoke.c
	gcc some.c -o bin/smoke
