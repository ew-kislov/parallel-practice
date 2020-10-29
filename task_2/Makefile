report_data: init report_data_inner clean
test: init test_inner clean
init:
	mkdir -p tests
	g++ main.cpp
report_data_inner:
	python matrix_util.py
	gnuplot generate_diagram
test_inner:
	python test_matrix.py
clean:
	rm -rf tests
	rm a.out