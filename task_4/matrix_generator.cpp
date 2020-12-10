#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    size_t rows, cols;
    string fileMatrix = "matrix.bin";
    string fileVector = "vector.bin";

    if (argc == 3) {
        rows = atoi(argv[1]);
        cols = atoi(argv[2]);
    }

    double* matrix = new double [rows * cols];
    double* vector = new double [cols];
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[i * cols + j] = rand() % 100;
        }
    }

    for (size_t i = 0; i < cols; ++i) {
        vector[i] = rand() % 100;
    }

    int fdMatrix = open(fileMatrix.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);
    int fdVector = open(fileVector.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);

    write(fdMatrix, &rows, sizeof(size_t));
    write(fdMatrix, &cols, sizeof(size_t));
    for (int i = 0; i < rows; ++i) {
        write(fdMatrix, &matrix[i * cols], sizeof(double) * cols);
    }

    write(fdVector, &cols, sizeof(size_t));
    write(fdVector, vector, sizeof(double) * cols);

    close(fdMatrix);
    close(fdVector);

    return 0;
}