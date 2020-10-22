test: init test_inner clean
report_data: init report_data_inner clean
init:
	mkdir -p tests
	g++ main.cpp -o main.sh
test_inner:
	python generate_tests.py
	pytest -s
report_data_inner:
	python generate_csv_data.py
	gnuplot generate_diagram
clean:
	rm -rf tests
	rm main.sh