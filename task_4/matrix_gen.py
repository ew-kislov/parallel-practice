import sys
import numpy as np
import struct

EPS = 0.001

def generate_matrix(rows, cols):
    file_name_matrix = "data/matrix.bin"
    file_name_vector = "data/vector.bin"
    file_name_result = "data/result.bin"

    matrix = 100 * np.random.rand(rows, cols).astype('float')
    vector = 100 * np.random.rand(cols, 1).astype('float')
    result = np.dot(matrix, vector).astype('float')

    print(matrix)
    print()
    print(vector)
    print()
    print(result)
    print()

    to_files(rows, cols, matrix, vector, file_name_matrix, file_name_vector)


def generate_and_test(rows, cols, proc):
    file_name_matrix = "data/matrix.bin"
    file_name_vector = "data/vector.bin"
    file_name_result = "data/result.bin"

    matrix = 100 * np.random.rand(rows, cols)
    vector = 100 * np.random.rand(rows, 1)
    result = np.dot(matrix, vector)

    toFiles(rows, cols, matrix, vector, file_name_matrix, file_name_vector)

    run_shell(f"mpirun -np {proc} ./main")

    check(rows, cols, result, file_name_result)


def to_files(rows, cols, matrix, vector, file_name_matrix, file_name_vector):
    file_matrix = open(file_name_matrix, 'wb')
    file_vector = open(file_name_vector, 'wb')

    np.array([rows]).tofile(file_matrix)
    np.array([cols]).tofile(file_matrix)
    np.array(matrix).tofile(file_matrix)

    np.array([cols]).tofile(file_vector)
    np.array(vector).tofile(file_vector)

    file_matrix.close()
    file_vector.close()


def check(rows_exp, cols_exp, vector_exp, file_name_result):
    file_result = open(file_name_result, 'rb')
    
    rows = struct.unpack('i', fileRes.read(4))[0]
    fileRes.seek(4, 1)
    cols = struct.unpack('i', fileRes.read(4))[0]
    fileRes.seek(4, 1)
    data = np.fromfile(fileRes, 'float')

    file_result.close()

    vector = np.reshape(data, (-1, 1))

    if (
        (rows != rowsExp) |
        (cols != colsExp) |
        (not np.allclose(vector, vector_exp, EPS))
    ):
        print("Wrong result!")
        sys.exit(1)


def run_shell(bash_command):
    import subprocess
    process = subprocess.Popen(bash_command, shell = True)
    process.communicate()

print("Rows: ", end = '')
rows = int(input())
print("Columns: ", end = '')
cols = int(input())
generate_matrix(rows, cols)