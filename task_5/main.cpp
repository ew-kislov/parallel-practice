#include <mpi.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <tuple>
#include <math.h>
#include <tgmath.h>

#define MASTER_TO_SLAVE_TAG 1
#define SLAVE_TO_MASTER_TAG 5

using namespace std;

void sendMatrix(int* coordinates, int procNum, int blockSize, int procRank) {
    if (coordinates[2] == 0 && coordinates[1] != coordinates[2]) {
        int* newBlockCoordinates = setCoordinates(coordinates[0], coordinates[1], coordinates[1]);
        int destRank = coordinatesToRank(newBlockCoordinates, blockNum);
        MPI_Send(A, blockSize * blockSize, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
    }

    if (coordinates[1] && coordinates[1] == coordinates[2]) {
        int* srcBlockCoordinates = setCoordinates(coordinates[0], coordinates[1], 0);
        int srcRank = coordinatesToRank(srcBlockCoordinates, blockNum);
        MPI_Recv(A, blockSize * blockSize, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
    }

    if (coordinates[1] == coordinates[2]) {
        for (int j = 0; j < blockNum; ++j) {
            if (j != coordinates[2]) {
                int* newBlockCoordinates = setCoordinates(coordinates[0], j, coordinates[1]);
                int destRank = coordinatesToRank(newBlockCoordinates, blockNum);
                MPI_Send(A, blockSize * blockSize, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
            }
        }
    } else {
        int* srcBlockCoordinates = setCoordinates(coordinates[0], coordinates[2], coordinates[2]);
        int srcRank = coordinatesToRank(srcBlockCoordinates, blockNum);
        MPI_Recv(A, blockSize * blockSize, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
    }

    if (coordinates[2] == 0 && coordinates[0] != coordinates[2]) {
        int* newBlockCoordinates = setCoordinates(coordinates[0], coordinates[1], coordinates[0]);
        int destRank = coordinatesToRank(newBlockCoordinates, blockNum);
        MPI_Send(B, blockSize * blockSize, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD);
    }

    if (coordinates[2] && coordinates[0] == coordinates[2]) {
        int* srcBlockCoordinates = setCoordinates(coordinates[0], coordinates[1], 0);
        int srcRank = coordinatesToRank(srcBlockCoordinates, blockNum);
        MPI_Recv(B, blockSize * blockSize, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD, &status);
    }

    if (coordinates[0] == coordinates[2]) {
        for (int i = 0; i < blockNum; ++i) {
            if (i != coordinates[0]) {
                int* newBlockCoordinates = setCoordinates(i, coordinates[1], coordinates[2]);
                int destRank = coordinatesToRank(newBlockCoordinates, blockNum);
                MPI_Send(B, blockSize * blockSize, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
            }
        }
    } else {
        int* srcBlockCoordinates = setCoordinates(coordinates[2], coordinates[1], coordinates[2]);
        int srcRank = coordinatesToRank(srcBlockCoordinates, blockNum);
        MPI_Recv(B, blockSize * blockSize, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
    }
}

int* getProcCoordinates(int procRank, int procNum) {
    int blockSize = int(cbrt(procNum));
    int* coordinates = new int[3];

    coordinates[0] = (procRank / blockSize) % blockSize;
    coordinates[1] = procRank % blockSize;
    coordinates[2] = (procRank / int(pow(blockSize, 2))) % blockSize;

    return coordinates;
}

int coordinatesToOffset(int* coordinates, int n, int procNum) {
    int blockNum = int(cbrt(procNum));
    int blockSize = n / blockNum;
    return coordinates[0] * n * blockSize + coordinates[1] * blockSize;
}

int coordinatesToRank(int* coordinates, int size) {
    return size * (size * coordinates[2] + coordinates[0]) + coordinates[1];
}

int* setCoordinates(int x, int y, int z) {
    int* coordinates = new int [3];
    coordinates[0] = x;
    coordinates[1] = y;
    coordinates[2] = z;
    return coordinates;
}

double* matrixFromBin(string fileName, int& n, int procRank, int procNum) {
    double* block = NULL;
    int blockNum = int(cbrt(procNum));
    int* coordinates = getProcCoordinates(procRank, procNum);

    MPI_File file;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, fileName.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_read(file, &n, 1, MPI_INT, &status);

    int blockSize = n / blockNum;
    block = new double [blockSize * blockSize];
    int offset = coordinatesToOffset(coordinates, n, procNum);

    MPI_File_set_view(file, sizeof(int) + offset * sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);

    if (coordinates[2] == 0) {
        for (int i = 0; i < blockSize; ++i) {
            MPI_File_read(file, &block[i * blockSize], blockSize, MPI_DOUBLE, &status);
            int offset;
            MPI_File_get_position(file, &offset);
            MPI_File_seek(file, offset + n - blockElems, MPI_SEEK_SET);
        }
    }

    MPI_File_close(&file);

    return block;
}

void matrixToBin(string fileName, double* C, int n, int procRank, int procNum) {
    double* block = NULL;
    int blockNum = int(cbrt(procNum));
    int blockSize = n / blockNum;
    int* coordinates = getProcCoordinates(procRank, procNum);

    MPI_File file;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, fileName.c_str(), MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_write(file, &n, 1, MPI_INT, &status);

    int offset = coordinatesToOffset(coordinates, n, procNum);
    MPI_File_set_view(file, sizeof(int) + offset * sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);

    if (coordinates[2] == 0) {
        for (int i = 0; i < blockSize; ++i) {
            MPI_File_write(file, &C[i * blockSize], blockSize, MPI_DOUBLE, &status);
            int offset;
            MPI_File_get_position(file, &offset);
            MPI_File_seek(file, offset + n - blockSize, MPI_SEEK_SET);
        }
    }

    MPI_File_close(&file);
}

int main(int argc, char* argv[]) {
    MPI_Status status;
    int procRank;
    int procNum;
    int n;

    double *A;
    double *B;
    double *C;
    double *resultC;

    string fileA = "A.bin";
    string fileB = "B.bin";
    string fileC = "C.bin";

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);

    A = matrixFromBin(fileA, n, procRank, procNum);
    B = matrixFromBin(fileB, n, procRank, procNum);

    int blockNum = int(cbrt(procNum));
    int blockSize = n / blockNum;
    int* coordinates = getProcCoordinates(procRank, procNum);

    sendMatrix(coordinates, procNum, blockSize, procRank);

    C = new double [blockSize * blockSize];
    for (int i = 0; i < blockSize; ++i) {
        for (int j = 0; j < blockSize; ++j) {
            C[i * blockSize + j] = 0;
        }
    }

    for (int i = 0; i < blockSize; ++i) {
        for (int j = 0; j < blockSize; ++j) {
            for (int k = 0; k < blockSize; ++k) {
                C[i * blockSize + j] += A[i * blockSize + k] * B[k * blockSize + j];
            }
        }
    }

    if (coordinates[2] == 0) {
        resultC = new double [blockSize * blockSize];
    }

    int* rootCoordinates = setCoordinates(coordinates[0], coordinates[1], 0);
    int root = coordinatesToRank(rootCoordinates, blockNum);

    MPI_Comm MPI_COMM_VERTICAL;
    int verticalRank, verticalNum;
    int ijOffset = coordinates[0] * blockSize + coordinates[1];
    
    MPI_Comm_split(MPI_COMM_WORLD, ijOffset, procRank, &MPI_COMM_VERTICAL);
    MPI_Comm_rank(MPI_COMM_VERTICAL, &verticalRank);
    MPI_Comm_size(MPI_COMM_VERTICAL, &verticalNum);

    MPI_Reduce(C, resultC, blockSize * blockSize, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_VERTICAL);
    MPI_Comm_free(&MPI_COMM_VERTICAL);


    matrixToBin(fileC, resultC, n, procRank, procNum);

    MPI_Finalize();
}