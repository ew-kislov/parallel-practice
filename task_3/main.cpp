#include <mpi.h>
#include <iostream>
#include <fstream>
#include <tgmath.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

void primeNumbers(vector<int>& all, int procNum, int procRank) {
    MPI_Status status;
    int len = all.size();
    int partSize = ceil(len * 1.0 / procNum);
    int firstElem = procRank * partSize;
    int lastElem = procRank == procNum - 1 ? len - 1 : firstElem + partSize - 1;
    int buf[partSize];

    for (int i = 2; i*i < len; ++i) {
        MPI_Bcast(&all[0], all.size(), MPI_INT, 0, MPI_COMM_WORLD);

        if (all[i]) {
            MPI_Scatter(&all[0], partSize, MPI_INT, &buf[0], partSize, MPI_INT, 0, MPI_COMM_WORLD);

            for (int j = i*i; j <= lastElem; j += i) {
                if (j >= firstElem) {
                    buf[j - firstElem] = 0;
                }
            }

            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gather(&buf[0], partSize, MPI_INT, &all[0], partSize, MPI_INT, 0, MPI_COMM_WORLD);
        }
    }
}

void getInfo(int& first, int& last, string& fileName) {
    cout << "First element: ";
    cin >> first;
    cout << "Last element: ";
    cin >> last;
    cout << "Output file: ";
    cin >> fileName;
}

void printInfo(double time, int result, int first, int last) {
    cout << "Time: " << time << endl;
    cout << result << " prime numbers between " << first << " and " << last << endl;
}

void toFile(int first, vector<int> all, string fileName) {
    ofstream file;
    
    file.open(fileName, ios::out | ios::trunc);

    for (int i = first; i < all.size(); ++i) {
        if (all[i]) {
            file << i << " ";
        }
    }
    
    file.close();
}


int main(int argc, char** argv) {
    int first, last, procNum, procRank;
    double beg, end, time;
    string fileName;
    vector<int> all;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    if (procRank == 0) {
        getInfo(first, last, fileName);
        cout << endl << "Number of processes: " << procNum << endl << endl;
    }

    MPI_Bcast(&first, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&last, 1, MPI_INT, 0, MPI_COMM_WORLD);

    all.push_back(0); // 0 and 1 are not a prime numbers
    all.push_back(0);
    for (int i = 2; i < last; ++i) {
        all.push_back(1);
    }

    beg = MPI_Wtime();
    primeNumbers(all, procNum, procRank);
    end = MPI_Wtime();
    time = end - beg;

    MPI_Finalize();

    if (procRank == 0) {
        int result = count(all.begin() + first, all.end(), 1);
        printInfo(time, result, first, last);
        toFile(first, all, fileName);
    }
}
