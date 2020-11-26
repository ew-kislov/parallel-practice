.PHONY: all clean
all: init
plot: init plot_inn clean
init:
	mpic++ -o main main.cpp
plot_inn:
	mpirun -np 1 ./main
	mpirun -np 2 ./main
	mpirun -np 4 ./main
	mpirun -np 8 ./main
	mpirun -np 16 ./main
	mpirun -np 20 ./main
clean:
	rm ./main