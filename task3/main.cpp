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

void parseCommand(int argc, char** argv, int& first, int& last) {
    if (argc != 3) {
        cout << "2 parameters required" << endl;
        exit(0);
    }

    first = atoi(argv[1]);
    last = atoi(argv[2]);
}

void printInfo(double time, double maxTime, int result, int first, int last) {
    cout << "Total time: " << time << endl;
    cout << "Max time among processes: " << maxTime << endl;
    cout << result << " prime numbers between " << first << " and " << last << endl;
}

void toFile(int first, int* all, int len, string fileName) {
    ofstream file;
    
    file.open(fileName.c_str(), ios::out | ios::trunc);

    for (int i = 0; i < len; ++i) {
        if (all[i]) {
            file << first + i << " ";
        }
    }
    
    file.close();
}

void timeToCSV(string fileName, double time, int procNum) {
    ofstream file;
    file.open(fileName.c_str(), ios::out | ios::app);

    file << procNum << "\t" << time << endl;

    file.close();
}

void init(int& procNum, int& procRank, int*& all, int first, int last) {
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0) {
        cout << endl << "Number of processes: " << procNum << endl << endl;
        all = new int [last - first];
    }

    MPI_Bcast(&first, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&last, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void primeNumbers(int* all, int first, int last, int procNum, int procRank) {
    int len = last - first;
    int iMax = int(sqrt(last));
    int* primeHelp = new int [iMax + 1];

    primeHelp[0] = 0;
    primeHelp[1] = 0;
    for (int i = 2; i <= iMax; ++i) {
        primeHelp[i] = 1;
    }

    for (int i = 2; i <= iMax; ++i) {
        if (primeHelp[i]) {
            for (int j = i*i; j <= iMax; j += i) {
                primeHelp[j] = 0;
            }
        }
    }

    int partSize = ceil(len * 1.0 / procNum);
    int firstElem = procRank * partSize;
    int lastElem = procRank == procNum - 1 ? len - 1 : firstElem + partSize - 1;
    int buf[partSize];

    for (int i = 0; i < partSize; ++i) {
        buf[i] = 1;
    }

    for (int i = 2; i <= iMax; ++i) {
        if (primeHelp[i]) {
            int a = ceil((firstElem + first - i*i) * 1.0 / i);
            a = a < 0 ? 0 : a;
            for (int j = i*i + a * i; j <= lastElem + first; j += i) {
                buf[j - firstElem - first] = 0;
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

    parseCommand(argc, argv, first, last);
    init(procNum, procRank, all, first, last);

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