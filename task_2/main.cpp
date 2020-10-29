#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <exception>
#include <cmath>
#include <time.h>
#include <string>

#include <papi.h>

using namespace std;

float** tileMultiplication(float** A, float** B, int M, int N, int K, int blockSize, int mode) {
    float** C = new float*[M];
    for (int i = 0; i < M; i++) {
        C[i] = new float[K];
        for (int j = 0; j < K; j++) {
            C[i][j] = 0;
        }
    }

    if (mode == 1) {
        for (int i0 = 0; i0 < M; i0 += blockSize) {
            int iMax = (i0 + blockSize) > M ? M : i0 + blockSize;

            for (int j0 = 0; j0 < N; j0 += blockSize) {
                int jMax = (j0 + blockSize) > N ? N : j0 + blockSize;

                for (int k0 = 0; k0 < K; k0 += blockSize) {
                    int kMax = (k0 + blockSize) > K ? K : k0 + blockSize;

                    for (int i = i0; i < iMax; i++) {
                        for (int j = j0; j < jMax; j++) {
                            for (int k = k0; k < kMax; k++) {
                                C[i][j] += A[i][k] * B[k][j];
                            }
                        }
                    }
                }
            }
        }
    } else {
        for (int i0 = 0; i0 < M; i0 += blockSize) {
            int iMax = (i0 + blockSize) > M ? M : i0 + blockSize;

        for (int k0 = 0; k0 < K; k0 += blockSize) {
            int kMax = (k0 + blockSize) > K ? K : k0 + blockSize;

                for (int j0 = 0; j0 < N; j0 += blockSize) {
                    int jMax = (j0 + blockSize) > N ? N : j0 + blockSize;

                    for (int i = i0; i < iMax; i++) {
                        for (int k = k0; k < kMax; k++) {
                            float firstElement = A[i][k];
                            for (int j = j0; j < jMax; j++) {
                                C[i][j] += firstElement * B[k][j];
                            }
                        }
                    }
                }
            }
        }
    }

    return C;
}

float** fromBin(const char* filename, size_t& rows, size_t& cols) {
    int fd = open(filename, O_RDONLY, 0777);

    read(fd, &rows, sizeof(size_t));
    read(fd, &cols, sizeof(size_t));

    float** matrix = new float*[rows];

    for (size_t i = 0; i < rows; i++) {
        matrix[i] = new float[cols];
        read(fd, matrix[i], sizeof(float) * cols);
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

void print(float** matrix, size_t rows, size_t cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void parseCommand(int argc, char** argv, char** fileA, char** fileB, char** fileC, int* blockSize, int* mode) {
    if (argc != 6) {
        throw new invalid_argument("Wrong number of arguments for main()");
    }

    *fileA = argv[1];
    *fileB = argv[2];
    *fileC = argv[3];
    *blockSize = atoi(argv[4]);
    *mode = atoi(argv[5]);

    if (*blockSize == 0) {
        throw new invalid_argument("4th argument must be unsigned integer");
    } else if (*mode < 1 || *mode > 2) {
        throw new invalid_argument("5th argument must be 1 or 2");
    }
}

void readMatrixes(
    char* fileA, char* fileB,
    size_t* rowsA, size_t* colsA, size_t* rowsB, size_t* colsB, float*** A, float*** B
) {
    *A = fromBin(fileA, *rowsA, *colsA);
    *B = fromBin(fileB, *rowsB, *colsB);

    if (*colsA != *rowsB) {
        throw logic_error("Column number of A should be equal rows number of B");
    }
}

void printPerformanceInfo(size_t rowsA, size_t colsA, size_t rowsB, size_t colsB, int blockSize, int mode, float time, long long papiValues[3]) {
    cout << "Multiplication mode: " << (mode == 1 ? "ijk" : "ikj") << endl;
    cout << "Block size: " << blockSize << endl;

    cout << "Sizes: " << rowsA << "x" << colsA;
    cout << " * " << rowsB << "x" << colsB;
    cout << " -> " << rowsA << "x" << colsB << endl;
        
    cout << "Took time: " << ((float)time)/CLOCKS_PER_SEC << " sec" << endl;

    cout << "L1 cache misses: " << papiValues[0] << endl;
    cout << "L2 cache misses: " << papiValues[1] << endl;
    cout << "Total cycles: " << papiValues[2] << endl;

}

int main(int argc, char** argv) {
    try {
        char *fileA, *fileB, *fileC;
        int blockSize, mode;

        parseCommand(argc, argv, &fileA, &fileB, &fileC, &blockSize, &mode);

        size_t rowsA, colsA;
        size_t rowsB, colsB;
        float **A, **B;

        readMatrixes(fileA, fileB, &rowsA, &colsA, &rowsB, &colsB, &A, &B);

        clock_t time = 0;
        int papiEvents[3] = { PAPI_L1_DCM, PAPI_L2_DCM, PAPI_TOT_CYC };
        long long papiValues[3];
        int papiStatus;

        papiStatus = PAPI_start_counters(papiEvents, 3);
        if (papiStatus != PAPI_OK) {
            cerr << PAPI_strerror(papiStatus) << endl;
            return 1;
        }

        float** C = tileMultiplication(A, B, rowsA, colsA, colsB, blockSize, mode);

        time = clock() - time;
        papiStatus = PAPI_read_counters(papiValues, 3);
        if (papiStatus != PAPI_OK) {
            cerr << PAPI_strerror(papiStatus) << endl;
            return 1;
        }

        printPerformanceInfo(rowsA, colsA, rowsB, colsB, blockSize, mode, time, papiValues);

        toBin(fileC, C, rowsA, colsB);
    }
    catch(exception& e) {
        cerr << e.what() << endl;
    }
}