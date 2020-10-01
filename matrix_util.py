import sys
import numpy as np
import math
import subprocess

files_dir = './tests/'


def generate_test_set_to_bin(type, matrixes_index):
    m = generate_matrix_dim()
    k = generate_matrix_dim()
    n = generate_matrix_dim()

    matrix_A = generate_matrix(m, k, type)
    matrix_B = generate_matrix(k, n, type)
    matrix_C = np.dot(matrix_A, matrix_B)

    write_matrix(matrix_A, type, m, k, str(matrixes_index) + '_A')
    write_matrix(matrix_B, type, k, n, str(matrixes_index) + '_B')
    write_matrix(matrix_C, type, m, n, str(matrixes_index) + '_C_expected')


def generate_exec_set_to_bin(type, set_name, m, k, n):
    matrix_A = generate_matrix(m, k, type)
    matrix_B = generate_matrix(k, n, type)

    write_matrix(matrix_A, type, m, k, set_name + '_A')
    write_matrix(matrix_B, type, k, n, set_name + '_B')


def write_matrix(matrix, type, rows, cols, file_name):
    file = open(files_dir + file_name, 'wb')
    file.write(type.encode('ascii'))
    np.array([rows]).tofile(file)
    np.array([cols]).tofile(file)
    np.array(matrix).tofile(file)


def read_matrix(file_name):
    file = open(files_dir + file_name, 'rb')
    type = file.read(1).decode()

    rows = int.from_bytes(file.read(4), 'little')
    file.seek(4, 1)

    cols = int.from_bytes(file.read(4), 'little')
    file.seek(4, 1)

    np_type = 'float32' if type == 'f' else 'float'

    matrix_bin = np.fromfile(file, dtype=np_type)
    matrix = np.reshape(matrix_bin, (-1, cols))
    return matrix


def are_matrixes_equal(matrix_1, matrix_2):
    return np.allclose(matrix_1, matrix_2, 0.001)


# change values here to get different matrix size spread
def generate_matrix_dim():
    return math.trunc(400 * np.random.rand() + 100)


# change values here to get different matrix elements spread
def generate_matrix(rows, cols, type):
    return 100 * np.random.rand(rows, cols).astype(type)

def get_running_time(command):
    process = subprocess.run(command.split(' '), stdout=subprocess.PIPE)
    time = process.stdout.decode().split('\n')[3].split(' ')[-2]

    return time

def output_performance_data():
    file = open('data.csv', 'w')
    for i in range(0, 6):
        time = get_running_time(f"./main.sh tests/perf_A tests/perf_B tests/perf_C {i}")
        file.writelines(f"{i} {time}\n")