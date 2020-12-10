#include <mpi.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <math.h>
#include <tgmath.h>

#define MASTER_TO_SLAVE_TAG 1
#define SLAVE_TO_MASTER_TAG 5

using namespace std;

int coordsToRank(int* coords, int size) {
    return size * (size * coords[2] + coords[0]) + coords[1];
}

int coordsToRank(int i, int j, int k, int size) {
    return size * (size * k + i) + j;
}

int* getProcCoords(int procRank, int procNum) {
    int blockSize = int(cbrt(procNum));
    int* coords = new int[3];

    coords[0] = (procRank / blockSize) % blockSize;
    coords[1] = procRank % blockSize;
    coords[2] = (procRank / int(pow(blockSize, 2))) % blockSize;

    return coords;
}

int coordsToOffset(int* coords, int n, int procNum) {
    int blockNum = int(cbrt(procNum));
    int blockElems = n / blockNum;
    return coords[0] * n * blockElems + coords[1] * blockElems;
}

int sendMatrix(int* procCoords, int procRank, int procNum) {
    int di, dj, dk;
    int si, sj, sk;

    if (procCoords[2] == 0 && procCoords[1] != procCoords[2]) {
        di = procCoords[0];
        dj = procCoords[1];
        dk = procCoords[1];

        int destRank = coordsToRank(di, dj, dk, blockNum);

        MPI_Send(A, blockElems * blockElems, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
    }

    if (procCoords[1] != 0 && procCoords[1] == procCoords[2]) {
        si = procCoords[0];
        sj = procCoords[1];
        sk = 0;

        int srcRank = coordsToRank(si, sj, sk, blockNum);

        MPI_Recv(A, blockElems * blockElems, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
    }

    if (procCoords[1] == procCoords[2]) {
        for (int j = 0; j < blockNum; ++j) {
            if (j != procCoords[2]) {
                di = procCoords[0];
                dj = j;
                dk = procCoords[1];
                int destRank = coordsToRank(di, dj, dk, blockNum);
                MPI_Send(A, blockElems * blockElems, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
            }
        }
    } else {
        si = procCoords[0];
        sj = procCoords[2];
        sk = procCoords[2];
        int srcRank = coordsToRank(si, sj, sk, blockNum);
        MPI_Recv(A, blockElems * blockElems, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
    }

    if (procCoords[2] == 0 && procCoords[0] != procCoords[2]) {
        di = procCoords[0];
        dj = procCoords[1];
        dk = procCoords[0];
        int destRank = coordsToRank(di, dj, dk, blockNum);
        MPI_Send(B, blockElems * blockElems, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD);
    }

    if (procCoords[2] != 0 && procCoords[0] == procCoords[2]) {
        si = procCoords[0];
        sj = procCoords[1];
        sk = 0;
        int srcRank = coordsToRank(si, sj, sk, blockNum);
        MPI_Recv(B, blockElems * blockElems, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD, &status);
    }

    if (procCoords[0] == procCoords[2]) {
        for (int i = 0; i < blockNum; ++i) {
            if (i != procCoords[0]) {
                di = i;
                dj = procCoords[1];
                dk = procCoords[2];
                int destRank = coordsToRank(di dj, dk, blockNum);
                MPI_Send(B, blockElems * blockElems, MPI_DOUBLE, destRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
            }
        }
    } else {
        si = procCoords[2];
        sj = procCoords[1];
        sk = procCoords[2];
        int srcRank = coordsToRank(si, sj, sk, blockNum);
        MPI_Recv(B, blockElems * blockElems, MPI_DOUBLE, srcRank, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
    }
}

double* fromBin(string fileName, int& n, int procRank, int procNum) {
    double* block = NULL;

    int blockNum = int(cbrt(procNum));
    int* procCoords = getProcCoords(procRank, procNum);

    MPI_File file;
    MPI_Status status;

    MPI_File_open(MPI_COMM_WORLD, fileName.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_read(file, &n, 1, MPI_INT, &status);

    int blockElems = n / blockNum;
    block = new double [blockElems * blockElems];
    int offset = coordsToOffset(procCoords, n, procNum);

    MPI_File_set_view(file, sizeof(int) + offset * sizeof(double), MPI_DOUBLE, MPI_DOUBLE, "native", MPI_INFO_NULL);

    if (procCoords[2] == 0) {
        for (int i = 0; i < blockElems; ++i) {
            for (int j = 0; j < blockElems; ++j) {
                MPI_File_read(file, &block[i * blockElems + j], 1, MPI_DOUBLE, &status);
            }
            int offset;
            MPI_File_get_position(file, &offset);
            MPI_File_seek(file, offset + n - blockElems, MPI_SEEK_SET);
        }
    }

    MPI_File_close(&file);

    return block;
}

int main(int argc, char* argv[]) {
    MPI_Status status;
    int procRank, procNum, n;

    double *A, *B, *C, *resultC;

    string fileA = "A.bin";
    string fileB = "B.bin";

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);

    A = fromBin(fileA, n, procRank, procNum);
    B = fromBin(fileB, n, procRank, procNum);

    int blockNum = int(cbrt(procNum));
    int blockElems = n / blockNum;
    int* procCoords = getProcCoords(procRank, procNum);

    sendMatrix(procCoords, procRank, procNum);

    C = new double [blockElems * blockElems];

    for (int i = 0; i < blockElems; ++i) {
        for (int j = 0; j < blockElems; ++j) {
            C[i * blockElems + j] = 0;
        }
    }

    for (int i = 0; i < blockElems; ++i) {
        for (int j = 0; j < blockElems; ++j) {
            for (int k = 0; k < blockElems; ++k) {
                C[i * blockElems + j] += A[i * blockElems + k] * B[k * blockElems + j];
            }
        }
    }

    MPI_Finalize();
}