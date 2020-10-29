import pytest
import numpy as np
import matrix_util as mu
import subprocess

def execute(command):
    process = subprocess.Popen(command, shell = True)
    process.communicate()

def test_matrix():
    for i in range(1, 6):
        file_name_A = str(i) + "_A"
        file_name_B = str(i) + "_B"
        file_name_C = str(i) + "_C"
        file_name_C_expected = str(i) + "_C_expected"
        mode = 1

        command = ("./a.out " +
            mu.files_dir + file_name_A + " " +
            mu.files_dir + file_name_B + " " +
            mu.files_dir + file_name_C + " " +
            str(mode) +
            52)
        
        execute(command)
        print()

        matrix_C = mu.read_matrix(file_name_C)
        matrix_C_expected = mu.read_matrix(file_name_C_expected)

        assert mu.are_matrixes_equal(matrix_C, matrix_C_expected) == True, f"Fails on matrixes with names {file_name_A} and {file_name_B}"