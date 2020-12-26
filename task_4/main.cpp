#include <mpi.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

#define MASTER_TO_SLAVE_TAG 1
#define SLAVE_TO_MASTER_TAG 4

using namespace std;

double* matrixFromBin(string fileName, int& rows, int& cols, int& mode) {
    double* matrix;
    int fd = open(fileName.c_str(), O_RDONLY, 0777);
    int realRows, realCols;

    size_t rows_t, cols_t;

    read(fd, &rows_t, sizeof(size_t));
    read(fd, &cols_t, sizeof(size_t));

    rows = static_cast<int>(rows_t);
    cols = static_cast<int>(cols_t);

    mode = rows >= cols ? 1 : 2;

    matrix = new double [rows * cols];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mode == 1) {
                read(fd, &(matrix[i * cols + j]), sizeof(double));
            } else {
                read(fd, &(matrix[j * rows + i]), sizeof(double));
            }
        }
    }

    close(fd);

    return matrix;
}

void toBin(const char* filename, float** matrix, size_t rows, size_t cols) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0777);

    write(fd, &rows, sizeof(size_t));
    write(fd, &cols, sizeof(size_t));

    for (size_t i = 0; i < rows; i++) {
        write(fd, matrix[i], sizeof(float) * cols);
    }

    close(fd);
}

double* vectorFromBin(string fileName, int& length) {
    int fd = open(fileName.c_str(), O_RDONLY, 0777);

    size_t len_t;

    read(fd, &len_t, sizeof(size_t));

    length = static_cast<int>(len_t);
    double* vector = new double [length];
    read(fd, vector, sizeof(double) * length);
    close(fd);

    return vector;
}

int* getBounds(int size, int rank, int procNum) {
    int* bounds = new int [2];
    int lowBound, upperBound;
    int activeProcNum = size >= procNum ? procNum : size;

    int portion = size / activeProcNum;

    if (rank < activeProcNum) {
        lowBound = rank * portion;
        upperBound = rank == activeProcNum - 1 ? size : lowBound + portion;
    } else {
        lowBound = 0;
        upperBound = 0;
    }

    bounds[0] = lowBound;
    bounds[1] = upperBound;

    return bounds;
}

double* multiplyParallelColumns(double* matrix, double* vector, int rows, int cols, int procRank, int procNum) {
    int lowBound, upperBound;
    double *result, *buffResult, *buffVector, *buffCols;
    MPI_Status status;
    MPI_Request request;

    int* bounds = getBounds(cols, procRank, procNum);
    lowBound = bounds[0];
    upperBound = bounds[1];

    if (procRank == 0) {
        result = new double [rows];
        for (int i = 1; i < procNum; i++) {
            int* iBounds = getBounds(cols, i, procNum);
            int iterLowBound = iBounds[0];
            int iterUpperBound = iBounds[1];
            MPI_Isend(&matrix[iterLowBound * rows], (iterUpperBound - iterLowBound) * rows, MPI_DOUBLE, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &request);
            MPI_Isend(&vector[iterLowBound], iterUpperBound - iterLowBound, MPI_DOUBLE, i, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD, &request);
            delete[] iBounds;
        }
    }

    if (procRank == 0) {
        buffCols = &matrix[lowBound * rows];
        buffVector = &vector[lowBound];
    } else {
        buffCols = new double [(upperBound - lowBound) * rows];
        buffVector = new double [upperBound - lowBound];
        MPI_Recv(buffCols, (upperBound - lowBound) * rows, MPI_DOUBLE, 0, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(buffVector, upperBound - lowBound, MPI_DOUBLE, 0, MASTER_TO_SLAVE_TAG + 1, MPI_COMM_WORLD, &status);
    }

    buffResult = new double [rows];
    for (int i = 0; i < rows; i++) {
        buffResult[i] = 0;
    }

    for (int i = 0; i < upperBound - lowBound; i++) {
        for (int j = 0; j < rows; j++) {
            buffResult[j] += buffCols[i * rows + j] * buffVector[i];
        }
    }

    MPI_Reduce(buffResult, result, rows, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    delete[] buffResult;
    delete[] bounds;

    if (procRank == 0) {
        return result;
    } else {
        delete[] buffVector;
        delete[] buffCols;
    }

    return NULL;
}

double* multiplyParallelRows(double* matrix, double* vector, int rows, int cols, int procRank, int procNum) {
    int lowBound, upperBound;
    double *result, *buffResult, *buffRows;
    MPI_Status status;
    MPI_Request request;

    if (procRank > 0) {
        vector = new double [cols];
    }

    MPI_Bcast(vector, cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int* bounds = getBounds(rows, procRank, procNum);
    lowBound = bounds[0];
    upperBound = bounds[1];

    if (procRank == 0) {
        result = new double [rows];
        for (int i = 1; i < procNum; i++) {
            int* iBounds = getBounds(rows, i, procNum);
            int iterLowBound = iBounds[0];
            int iterUpperBound = iBounds[1];
            MPI_Isend(&matrix[iterLowBound * cols], (iterUpperBound - iterLowBound) * cols, MPI_DOUBLE, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &request);

            delete[] iBounds;
        }
    }
    
    if (procRank == 0) {
        buffRows = &matrix[lowBound * cols];
        buffResult = &result[lowBound];
    } else {
        buffResult = new double [upperBound - lowBound];
        buffRows = new double [(upperBound - lowBound) * cols];
        MPI_Recv(buffRows, (upperBound - lowBound) * cols, MPI_DOUBLE, 0, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD, &status);
    }

    for (int i = 0; i < upperBound - lowBound; i++) {
        buffResult[i] = 0;
        for (int j = 0; j < cols; j++) {
            buffResult[i] += buffRows[i * cols + j] * vector[j];
        }
    }
    
    if (procRank > 0) {
        MPI_Isend(buffResult, upperBound - lowBound, MPI_DOUBLE, 0, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD, &request);
    }

    delete[] bounds;

    if (procRank == 0) {
        for (int i = 1; i < procNum; i++) {
            int* iBounds = getBounds(rows, i, procNum);
            int iterLowBound = iBounds[0];
            int iterUpperBound = iBounds[1];
            MPI_Recv(&result[iterLowBound], iterUpperBound - iterLowBound, MPI_DOUBLE, i, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD, &status);

            delete[] iBounds;
        }

        return result;
    } else {
        delete[] buffResult;
        delete[] buffRows;
        return NULL;
    }
}

int main(int argc, char *argv[]) {
    int procRank, procNum, strategy;

    string fileMatrix = "matrix.bin";
    string fileVector = "vector.bin";
    string fileResult = "result.bin";
    string fileTime = "time.csv";

    double *vector, *result, *matrix;
    int rows, cols;
    double start, time, maxTime;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);

    if (procRank == 0) {
        matrix = matrixFromBin(fileMatrix, rows, cols, strategy);
        vector = vectorFromBin(fileVector, cols);
    }

    MPI_Bcast(&strategy, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    start = MPI_Wtime();
    if (strategy == 1) {
        result = multiplyParallelRows(matrix, vector, rows, cols, procRank, procNum);
    } else {
        result = multiplyParallelColumns(matrix, vector, rows, cols, procRank, procNum);
    }
    time = MPI_Wtime() - start;

    MPI_Reduce(&time, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Finalize();

    if (procRank == 0) {
        cout << procNum << " " << maxTime << endl;
        ofstream file(fileTime, ios::app);
        file << procNum << "\t" << maxTime << endl;
        file.close();

        toBin(fileResult, result, rows, 1);

        delete[] matrix;
        delete[] vector;
        delete[] result;
    }

    return 0;
}