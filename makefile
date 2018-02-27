#SRCS = $(wildcard *.c) #list of all .c files

main: $(SRCS)
	gcc mandelbrot-hplewa2.c -o mandelbrot
	gcc mandelCalc-hplewa2.c -o mandelCalc
	gcc mandelDisplay-hplewa2.c -o mandelDisplay

clean:
	rm mandelbrot mandelCalc mandelDisplay
