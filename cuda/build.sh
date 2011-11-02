#!/bin/bash -e
python cugen-fast1-gray.py > fast1_gen.cu
python cugen-fast2-gray.py > fast2_gen.cu
g++-4.4 -I../include/ -O3 -shared -fPIC -o libfastcl.so fastcl.cc
nvcc -DCVD_IMAGE_DEBUG -arch=sm_23 --compiler-bindir=/usr/bin/gcc-4.4 -O3 -L./ -L../bin/ -lfastcl -lcvdcl -lcvd -lOpenCL -o bin-test fast.cu
LD_LIBRARY_PATH=".:../bin:$LD_LIBRARY_PATH" ./bin-test

