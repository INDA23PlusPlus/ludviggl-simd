
SRC=main.c
EXE=mandelbrot
CC=gcc
CFLAGS=-Wall -Wextra -O9 -lm -mavx

.PHONY:
	clean
	run

build: main.c
	$(CC) $(CFLAGS) $(SRC) -o $(EXE)

benchmark:
	$(CC) $(CFLAGS) -DBENCHMARK $(SRC) -o benchmark


run: build
	./$(EXE)

clean:
	rm -f *.o $(EXE) *.png
