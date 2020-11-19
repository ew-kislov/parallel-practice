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

void primeNumbers(int* all, int first, int last, int procNum, int procRank) {
    int len = last - first;
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
    int buf[partSize];

    for (int i = 0; i < partSize; ++i) {
        buf[i] = 1;
    }

    for (int i = 2; i <= iMax; ++i) {
        if (trunkedPrimes[i]) {
            int a = ceil((firstElement + first - i*i) * 1.0 / i);
            a = a < 0 ? 0 : a;
            for (int j = i*i + a * i; j <= lastElement + first; j += i) {
                buf[j - firstElement - first] = 0;
            }
        }
    }

    MPI_Gather(&buf[0], partSize, MPI_INT, &all[0], partSize, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == 0 && first == 1) {
        all[0] = 0;
    }
}

int main(int argc, char** argv) {
    int* all;
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

    if (procRank == 0) {
        cout << endl << "Number of processes: " << procNum << endl << endl;
        all = new int [last - first];
    }

    MPI_Bcast(&first, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&last, 1, MPI_INT, 0, MPI_COMM_WORLD);

    begin = MPI_Wtime();
    primeNumbers(all, first, last, procNum, procRank);
    end = MPI_Wtime();
    time = end - begin;

    MPI_Reduce(&time, &totalTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Finalize();

    if (procRank == 0) {
        int result = count(all, &all[last - first], 1);
        
        cout << "Total time: " << time << endl;
        cout << "Max process time: " << maxTime << endl;
        cout << result << " prime numbers between " << first << " and " << last << endl;

        ofstream outFile;
        ofstream maxFile;
        ofstream totalFile;
    
        outFile.open("out.txt", ios::out | ios::trunc);
        maxFile.open("max.csv", ios::out | ios::app);
        totalFile.open("total.csv", ios::out | ios::app);

        for (int i = 0; i < last - first; i++) {
            if (all[i]) {
                outFile << first + i << " ";
            }
        }

        maxFile << procNum << "\t" << maxTime << endl;
        totalFile << procNum << "\t" << totalTime << endl;

        outFile.close();
        maxFile.close();
        totalFile.close();
    }
}