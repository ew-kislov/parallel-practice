#include <mpi.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

void primeNumbers(int* totalPrimes, int first, int last, int procNum, int procRank) {
    int len = (int)(ceil((last - first) * 1.0 / procNum) * procNum);
    int iMax = int(sqrt(last));
    int* trunkedPrimes = new int [iMax + 1];

    trunkedPrimes[0] = 0;
    trunkedPrimes[1] = 0;
    for (int i = 2; i <= iMax; ++i) {
        trunkedPrimes[i] = 1;
    }

    for (int i = 2; i <= iMax; ++i) {
        if (trunkedPrimes[i]) {
            for (int j = i*i; j <= iMax; j += i) {
                trunkedPrimes[j] = 0;
            }
        }
    }

    int partSize = ceil(len * 1.0 / procNum);
    int firstElement = procRank * partSize;
    int lastElement = procRank == procNum - 1 ? len - 1 : firstElement + partSize - 1;
    int* buff = new int[partSize];

    for (int i = 0; i < partSize; ++i) {
        buff[i] = 1;
    }

    for (int i = 2; i <= iMax; ++i) {
        if (trunkedPrimes[i]) {
            int a = ceil((firstElement + first - i*i) * 1.0 / i);
            a = a < 0 ? 0 : a;
            for (int j = i*i + a * i; j <= lastElement + first; j += i) {
                buff[j - firstElement - first] = 0;
            }
        }
    }

    MPI_Gather(buff, partSize, MPI_INT, totalPrimes, partSize, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == 0 && first == 1) {
        totalPrimes[0] = 0;
    }

    delete[] trunkedPrimes;
    delete[] buff;
}

int main(int argc, char** argv) {
    int* totalPrimes = NULL;
    int first, last, procNum, procRank;
    double begin, end, time, totalTime, maxTime;

    MPI_Init(&argc, &argv);

    if (argc != 3) {
        cout << "2 parameters required" << endl;
        exit(0);
    }

    first = atoi(argv[1]);
    last = atoi(argv[2]);

    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    // if (procRank == 0) {
    //     cout << endl << "Number of processes: " << procNum << endl << endl;
        totalPrimes = new int [(int)(ceil((last - first) * 1.0 / procNum) * procNum)];
    // }

    MPI_Bcast(&first, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&last, 1, MPI_INT, 0, MPI_COMM_WORLD);

    begin = MPI_Wtime();
    primeNumbers(totalPrimes, first, last, procNum, procRank);
    end = MPI_Wtime();
    time = end - begin;

    MPI_Reduce(&time, &totalTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Finalize();

    if (procRank == 0) {
        int result = count(totalPrimes, &totalPrimes[last - first], 1);
        cout << result << endl;

        delete[] totalPrimes;
    }
}