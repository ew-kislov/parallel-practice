import sys
import numpy as np
import math
import subprocess

files_dir = './tests/'


def generate_test_set_to_bin(size):
    matrix_A = generate_matrix(size, size)
    matrix_B = generate_matrix(size, size)
    matrix_C = np.dot(matrix_A, matrix_B)

    write_matrix(matrix_A, size, size, str(size) + '_A')
    write_matrix(matrix_B, size, size, str(size) + '_B')
    write_matrix(matrix_C, size, size, str(size) + '_C_expected')


def write_matrix(matrix, rows, cols, file_name):
    file = open(files_dir + file_name, 'wb')
    np.array([rows]).tofile(file)
    np.array([cols]).tofile(file)
    np.array(matrix).tofile(file)


def read_matrix(file_name):
    file = open(files_dir + file_name, 'rb')

    rows = int.from_bytes(file.read(4), 'little')
    file.seek(4, 1)

    cols = int.from_bytes(file.read(4), 'little')
    file.seek(4, 1)

    matrix_bin = np.fromfile(file, dtype='float32')
    matrix = np.reshape(matrix_bin, (-1, cols))
    return matrix


def are_matrixes_equal(matrix_1, matrix_2):
    return np.allclose(matrix_1, matrix_2, 0.001)


# change values here to get different matrix elements spread
def generate_matrix(rows, cols):
    return 100 * np.random.rand(rows, cols).astype('float32')


def get_program_perf(command):
    print('Starting command ' + command)
    process = subprocess.run(command.split(' '), stdout=subprocess.PIPE)
    output = process.stdout.decode()
    print(output)

    strings = output.split('\n')
    # time = strings[3].split(' ')[-2]
    # l1_misses = strings[4].split(' ')[-1]
    # l2_misses = strings[5].split(' ')[-1]
    time = strings[-2].split(' ')[-1]

    return time


if __name__ == "__main__":
    
    time_file = open('flops.csv', 'w')
    # l1_file = open('l1_misses.csv', 'w')
    # l2_file = open('l2_misses.csv', 'w')
    # cycles_file = open('cycles.csv', 'w')

    for i in range(1, 6):
        factor = 1000
        size = i * factor
        generate_test_set_to_bin(size)

        program_name = 'a.out'
        command1 = f"./{program_name} tests/{size}_A tests/{size}_B tests/{size}_A 32 1"
        command2 = f"./{program_name} tests/{size}_A tests/{size}_B tests/{size}_A 32 2"
        command3 = f"./{program_name} tests/{size}_A tests/{size}_B tests/{size}_A 52 2"

        perf1 = get_program_perf(command1)
        perf2 = get_program_perf(command2)
        perf3 = get_program_perf(command3)

        perf_str = f"{i} {perf1} {perf2} {perf3}\n"
        print(perf_str)
        time_file.writelines(perf_str)
        # l1_file.writelines(f"{i} {perf1[1]} {perf2[1]} {perf3[1]}\n")
        # l2_file.writelines(f"{i} {perf1[2]} {perf2[2]} {perf3[2]}\n")
        # cycles_file.writelines(f"{i} {perf1[3]} {perf2[3]} {perf3[3]}\n")