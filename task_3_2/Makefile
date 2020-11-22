.PHONY: all clean
all: init
plot: init plot_inner clean
init:
	rm -f total.csv
	g++ -o main main.cpp
plot_inner:
	./main 1 1 10000000
	./main 2 1 10000000
	./main 4 1 10000000
	./main 8 1 10000000
	./main 16 1 10000000
	./main 32 1 10000000
	./main 64 1 10000000
	./main 128 1 10000000
	gnuplot plot_total.p
clean:
	rm ./main
	rm out.txt
	rm total.csv