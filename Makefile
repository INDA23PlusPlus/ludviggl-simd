
SRC=main.c
EXE=mandelbrot
CC=gcc
CFLAGS=-Wall -Wextra -O1 -lm -march=native

.PHONY:
	clean
	run
	benchmark

build: main.c
	$(CC) $(CFLAGS) $(SRC) -o $(EXE)

benchmark: main.c
	$(CC) $(CFLAGS) -DBENCHMARK $(SRC) -o benchmark
	./benchmark

run: build
	./$(EXE)

clean:
	rm -f *.o $(EXE) benchmark *.png
