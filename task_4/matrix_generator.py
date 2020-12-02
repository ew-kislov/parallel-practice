import numpy as np

def write_matrix(matrix, rows, cols, file_name):
    file = open(file_name, 'wb')
    np.array([rows]).tofile(file)
    np.array([cols]).tofile(file)
    np.array(matrix).tofile(file)


# change values here to get different matrix elements spread
def generate_matrix(rows, cols):
    return 100 * np.random.rand(rows, cols)

m = 512
n = 512

A = generate_matrix(m, n)
write_matrix(A, m, n, 'matrix.bin')

b = generate_matrix(n, 1)
write_matrix(A, m, n, 'vector.bin')