report_data: init report_data_inner clean
init:
	mkdir -p tests
	g++ main.cpp
report_data_inner:
	python matrix_util.py
	gnuplot generate_diagram
clean:
	rm -rf tests
	rm a.out