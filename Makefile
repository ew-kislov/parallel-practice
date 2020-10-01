test: init test_inner clean
init:
	mkdir -p tests
	g++ main.cpp -o main.sh
	python matrix_util.py
test_inner:
	pytest
clean:
	rm -rf tests
	rm main.sh