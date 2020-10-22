import matrix_util as mu

# generating float type matrixes
for i in range(1, 7):
    mu.generate_test_set_to_bin('f', i)

# generating double type matrixes
for i in range(7, 13):
    mu.generate_test_set_to_bin('d', i)