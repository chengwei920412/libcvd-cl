#!/bin/bash -e
python cugen-fast1-gray.py > fast1_gen.cu
python cugen-fast2-gray.py > fast2_gen.cu
nvcc -DCVD_IMAGE_DEBUG --gpu-architecture=compute_20 --compiler-bindir=/usr/bin/gcc-4.4 -O3 -lcvd -o bin-test fast.cu
./bin-test

