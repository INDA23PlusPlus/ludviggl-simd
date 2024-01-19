
EXE=mandelbrot

.PHONY:
	clean
	run

build: main.c
	gcc -Wall -Wextra main.c -O9 -lm -mavx -o $(EXE)

run: build
	./$(EXE)

clean:
	rm -f *.o $(EXE) *.png
