#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <exception>
#include <cmath>
#include <time.h>
#include <string>

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

    static Matrix* fromBin(const char* filename) {
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

    void toBin(const char* filename) {
        int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0777);

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

template <typename T> Matrix* multiply(Matrix* first, Matrix* second, int mode) {
    if (first->type != second->type) {
        throw "First matrix should be the same type as second";
    }
    if (first->cols != second->rows) {
        throw "First matrix column number should be equal second matrix row number";
    }

    void** result = new void*[first->rows];
    for (size_t i = 0; i < first->rows; i++) {
        result[i] = new T[second->cols];
    }

    switch (mode) {
        case 0:
            for (size_t i = 0; i < first->rows; i++) {
                for (size_t j = 0; j < second->cols; j++) {
                    for (size_t k = 0; k < first->cols; k++) {
                        ((T**)result)[i][j] += first->get(i, k) * second->get(k, j);
                    }
                }
            }
            break;
        case 1:
            for (size_t i = 0; i < first->rows; i++) {
                for (size_t k = 0; k < first->cols; k++) {
                    T firstElement = first->get(i, k);
                    for (size_t j = 0; j < second->cols; j++) {
                        ((T**)result)[i][j] += firstElement * second->get(k, j);
                    }
                }
            }
            break;
        case 2:
            for (size_t k = 0; k < first->cols; k++) {
                for (size_t i = 0; i < first->rows; i++) {
                    double firstElement = first->get(i, k);
                    for (size_t j = 0; j < second->cols; j++) {
                        ((T**)result)[i][j] += firstElement * second->get(k, j);
                    }
                }
            }
            break;
        case 3:
            for (size_t j = 0; j < second->cols; j++) {
                for (size_t i = 0; i < first->rows; i++) {
                    T sum = 0;
                    for (size_t k = 0; k < first->cols; k++) {
                        sum += first->get(i, k) * second->get(k, j);
                    }
                    ((T**)result)[i][j] = sum;
                }
            }
            break;
        case 4:
            for (size_t j = 0; j < second->cols; j++) {
                for (size_t k = 0; k < first->cols; k++) {
                    double secondElement = second->get(k, j);
                    for (size_t i = 0; i < first->rows; i++) {
                        ((T**)result)[i][j] += first->get(i, 0) * secondElement;
                    }
                }
            }
            break;
        case 5:
            for (size_t k = 0; k < first->cols; k++) {
                for (size_t j = 0; j < second->cols; j++) {
                    double secondElement = second->get(k, j);
                    for (size_t i = 0; i < first->rows; i++) {
                        ((T**)result)[i][j] += first->get(i, k) * secondElement;
                    }
                }
            }
            break;
        default:
            throw invalid_argument("Wrong mode");
    }

    return new Matrix(result, first->type, first->rows, second->cols);
}

void parseCommand(int argc, char** argv, string& fileNameA, string& fileNameB, string& fileNameC, int& mode) {
    if (argc != 5 || atoi(argv[4]) < 0 || atoi(argv[4]) > 5) {
        throw invalid_argument("Wrong parameters");
    }

    fileNameA = argv[1];
    fileNameB = argv[2];
    fileNameC = argv[3];
    mode = atoi(argv[4]);
}

int main(int argc, char ** argv) {
    try {
        string fileA, fileB, fileC;
        int mode;
        clock_t time = 0;

        parseCommand(argc, argv, fileA, fileB, fileC, mode);

        Matrix* matrixA = Matrix::fromBin(fileA.c_str());
        Matrix* matrixB = Matrix::fromBin(fileB.c_str());
        Matrix* matrixC;

        if (matrixA->type == 'f') {
            matrixC = multiply<float>(matrixA, matrixB, mode);
        } else {
            matrixC = multiply<double>(matrixA, matrixB, mode);
        }

        matrixC->toBin(fileC.c_str());

        time = clock() - time;

        cout << "Multiplication mode: " << mode << endl;
        cout << "Elements type: " << (matrixC->type == 'f' ? "float" : "double") << endl;

        cout << "Sizes: " << matrixA->rows << "x" << matrixA->cols;
        cout << " * " << matrixB->rows << "x" << matrixB->cols;
        cout << " -> " << matrixC->rows << "x" << matrixC->cols << endl;
        
        cout << "Took time: " << ((float)time)/CLOCKS_PER_SEC << " sec" << endl;
    } catch (exception &err) {
        cout << err.what() << endl;
    }
}
