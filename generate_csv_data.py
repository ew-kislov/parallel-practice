import matrix_util as mu

# generating matrixes for performance test
mu.generate_exec_set_to_bin('f', 'perf', 500, 300, 400)

# writes performance data for all modes to .csv file for diagram
mu.output_performance_data()