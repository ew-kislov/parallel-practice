import sys
import numpy as np
import math

files_dir = './tests/'

def generate_matrixes_to_bin(type, matrixes_index):
    # m = generate_matrix_dim()
    # k = generate_matrix_dim
    # n = generate_matrix_dim

    m = 5
    k = 3
    n = 8

    matrix_A = generate_matrix(m, k, type)
    matrix_B = generate_matrix(k, n, type)
    matrix_C = np.dot(matrix_A, matrix_B)

    write_matrix(matrix_A, type, m, k, str(matrixes_index) + '_A')
    write_matrix(matrix_B, type, k, n, str(matrixes_index) + '_B')
    write_matrix(matrix_C, type, m, n, str(matrixes_index) + '_C_expected')

def write_matrix(matrix, type, rows, cols, file_name):
    file = open(files_dir + file_name, 'wb')
    file.write(type.encode('ascii'))
    np.array([rows]).tofile(file)
    np.array([cols]).tofile(file)
    np.array(matrix).tofile(file)

# change values here to get different matrix size spread
def generate_matrix_dim():
    return math.trunc(400 * np.random.rand() + 100)

# change values here to get different matrix elements spread
def generate_matrix(rows, cols, type):
    return 100 * np.random.rand(rows, cols).astype(type)

# if __name__ == '__main__':
#     # generating float type matrixes
#     for i in range(1, 7):
#         generate_matrixes_to_bin('f', i)

#     # generating double type matrixes
#     for i in range(7, 13):
#         generate_matrixes_to_bin('d', i)

file = open('matrix.bin', 'rb')

type = file.read(1).decode()
rows = int.from_bytes(file.read(4), 'little')
file.seek(4, 1)
cols = int.from_bytes(file.read(4), 'little')
file.seek(4, 1)

print(rows)
print(cols)

np_type = 'float32' if type == 'f' else 'float'
matrix_bin = np.fromfile(file, dtype=np_type)
matrix = np.reshape(matrix_bin, (-1, 5))
print(matrix)

