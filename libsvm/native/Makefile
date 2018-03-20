CXX_FLAGS=-I/opt/intel/sgxsdk/include -I/usr/include/libsvm/ -O3

all:
	@g++ ${CXX_FLAGS} benchmark.cpp -o benchmark
	@gcc ${CXX_FLAGS} svm-predict.c -o svm -lsvm
	@gcc ${CXX_FLAGS} sha3.c -o sha3
	@make -C GeneticAlgorithm/build

test: all
	@./svm ../heart_scale ../heart_scale.model out
	@./benchmark
	@./sha3
	@./GeneticAlgorithm/build/GeneticAlgorithm
