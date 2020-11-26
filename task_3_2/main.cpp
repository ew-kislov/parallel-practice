#include <pthread.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

using namespace std;

int* trunkedPrimes;
int* primes;
int iMax;

void* threadFunction(void* params) {
    int* bounds = ((int *)params);
    int firstElement = bounds[0];
    int lastElement = bounds[1];

    for (int i = firstElement; i <= lastElement; i++) {
        primes[i] = 1;
    }

    for (int i = 2; i <= iMax; i++) {
        if (trunkedPrimes[i]) {
            int a = ceil((firstElement - i*i) * 1.0 / i);
            a = a < 0 ? 0 : a;
            for (int j = i*i + a * i; j <= lastElement; j += i) {
                primes[j] = 0;
            }
        }
    }

    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    int first, last, threadNum;

    if (argc != 4) {
        cout << "3 parameters required" << endl;
        exit(0);
    }

    threadNum = atoi(argv[1]);
    first = atoi(argv[2]);
    last = atoi(argv[3]);

    cout << "Number of threads: " << threadNum << endl << endl;

	struct timeval start, end;
	gettimeofday(&start, NULL);

    int len = last - first;
    iMax = int(sqrt(last));

    trunkedPrimes = new int [iMax + 1];

    trunkedPrimes[0] = 0;
    trunkedPrimes[1] = 0;
    for (int i = 2; i <= iMax; ++i) {
        trunkedPrimes[i] = 1;
    }

    for (int i = 2; i <= iMax; ++i) {
        if (trunkedPrimes[i]) {
            for (int j = i*i; j <= iMax; j += i) {
                trunkedPrimes[j] = 0;
            }
        }
    }

    int partSize = ceil(len * 1.0 / threadNum);

    pthread_t threadIds[threadNum];
    pthread_attr_t threadAttributes;

    pthread_attr_init(&threadAttributes);

    primes = new int[last];

    for (int i = 0; i < threadNum; i++) {
        int* bounds = new int[2];
        bounds[0] = i * partSize + first;
        bounds[1] = i == threadNum - 1 ? len - 1 + first : bounds[0] + partSize - 1;
        int res = pthread_create(&threadIds[i], &threadAttributes, threadFunction, (void*)bounds);
    }

    for (int i = 0; i < threadNum; i++) {
        int res = pthread_join(threadIds[i], NULL);
    }

    if (first == 1) {
        primes[1] = 0;
    }

	gettimeofday(&end, NULL);
    float totalTime = ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000) * 1.0 / 1000;

    int result = count(&primes[first], &primes[last], 1);

    cout << "Total time: " << totalTime << endl;
    cout << "Result: " << result << " prime numbers between " << first << " and " << last << endl;

    ofstream outFile;
    ofstream totalFile;
    
    outFile.open("out.txt", ios::out | ios::trunc);
    totalFile.open("total.csv", ios::out | ios::app);

    for (int i = first; i < last; i++) {
        if (primes[i]) {
            outFile << i << " ";
        }
    }

    totalFile << threadNum << "\t" << totalTime << endl;

    outFile.close();
    totalFile.close();

    delete[] primes;
    delete[] trunkedPrimes;
}