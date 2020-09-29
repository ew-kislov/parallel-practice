#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

struct Matrix {
    void** data;
    char type;
    size_t rows;
    size_t cols;

    Matrix(void** data, char type, size_t rows, size_t cols): data(data), type(type), rows(rows), cols(cols) {};

    template<typename T, size_t rows, size_t cols> static Matrix* createFromStatic(T staticMatrix[rows][cols]) {
        T** data = new T*[rows];
        for (size_t i = 0; i < rows; i++) {
            data[i] = new T[cols];
            for (size_t j = 0; j < cols; j++) {
                data[i][j] = staticMatrix[i][j];
            }
        }

        return new Matrix(reinterpret_cast<void**>(data), typeid(T).name()[0], rows, cols);
    }

    double get(size_t i, size_t j) {
        return type == 'f' ? reinterpret_cast<float**>(data)[i][j] : reinterpret_cast<double**>(data)[i][j];
    }

    void print() {
        for (size_t i = 0; i < rows; i++) {
            for (size_t j = 0; j < cols; j++) {
                cout << get(i,j) << " ";
            }
            cout << endl;
        }
    }

    static Matrix* fromBin(char* filename) {
        int fd = open(filename, O_RDONLY, 0777);

        char type;
        size_t rows, cols;

        read(fd, &type, sizeof(char));
        read(fd, &rows, sizeof(size_t));
        read(fd, &cols, sizeof(size_t));

        size_t typeSize = type == 'f' ? sizeof(float) : sizeof(double);

        void** data = new void*[rows];

        for (size_t i = 0; i < rows; i++) {
            if (type == 'f') {
                data[i] = new float[cols];
            } else {
                data[i] = new double[cols];
            }
            read(fd, data[i], typeSize * cols);
        }

        close(fd);

        return new Matrix(data, type, rows, cols);
    }

    void toBin(char* filename) {
        int fd = open(filename, O_CREAT | O_WRONLY, 0777);

        write(fd, &(this->type), sizeof(char));
        write(fd, &(this->rows), sizeof(size_t));
        write(fd, &(this->cols), sizeof(size_t));

        size_t typeSize = this->type == 'f' ? sizeof(float) : sizeof(double);

        for (size_t i = 0; i < this->rows; i++) {
            write(fd, this->data[i], typeSize * (this->cols));
        }

        close(fd);
    }
};

Matrix* multiply(Matrix* first, Matrix* second, int mode) {
    if (first->type != second->type) {
        throw "First matrix should be the same type as second";
    }
    if (first->cols != second->rows) {
        throw "First matrix column number should be equal second matrix row number";
    }

    void** result = new void*[first->rows];
    for (size_t i = 0; i < first->rows; i++) {
        if (first->type == 'f') {
            result[i] = new float[second->cols];
        } else {
            result[i] = new double[second->cols];
        }
    }

    for (size_t i = 0; i < first->rows; i++) {
        for (size_t j = 0; j < second->cols; j++) {
            for (size_t k = 0; k < first->cols; k++) {
                if (first->type == 'f') {
                    ((float**)result)[i][j] += first->get(i, k) * second->get(k, j);
                } else {
                    ((double**)result)[i][j] += first->get(i, k) * second->get(k, j);
                }
            }
        }
    }

    return new Matrix(result, first->type, first->rows, second->cols);
}

int main(int argc, char ** argv) {
    double firstStatic[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    double secondStatic[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    
    Matrix* first = Matrix::createFromStatic<double, 3, 3>(firstStatic);
    Matrix* second = Matrix::createFromStatic<double, 3, 3>(secondStatic);

    Matrix* result = multiply(first, second, 1);

    first->print();
    cout << endl;
    second->print();
    cout << endl;
    result->print();

    // matrix->print();
    // matrix->toBin("matrix.bin");

    // Matrix* newMatrix = Matrix::fromBin("matrix.bin");
    // newMatrix->print();
}