.PHONY: all clean
all: init
plot: init plot_inner clean
init:
	rm -f max.csv
	rm -f total.csv
	mpic++ -o main main.cpp
plot_inner:
	mpirun -np 1 ./main 1 10000000
	mpirun -np 2 ./main 1 10000000
	mpirun -np 4 ./main 1 10000000
	mpirun -np 8 ./main 1 10000000
	mpirun -np 16 ./main 1 10000000
	mpirun -np 32 ./main 1 10000000
	gnuplot plot_max.p
	gnuplot plot_total.p
clean:
	rm ./main
	rm out.txt
	rm max.csv
	rm total.csv