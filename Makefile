CFLAGS   = -std=gnu++98 -fopenmp -pthread -O3 -march=native -ffast-math
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
#CFLAGS += -g3

LIBS     = -lOpenCL -lcvd -lm -lboost_program_options-mt

all:
	mkdir -p obj bin
	python scripts/codegen-blur.py > opencl/blur.cl
	python scripts/codegen-cull.py > opencl/cull.cl
	python scripts/codegen-fast.py > opencl/fast.cl
	python scripts/codegen-filt.py > opencl/filt.cl
	python scripts/codegen-hips.py > opencl/hips.cl
	python scripts/codegen-find.py > opencl/find.cl

	python scripts/codegen-cholesky.py 3 > opencl/cholesky3.cl
	python scripts/codegen-cholesky.py 4 > opencl/cholesky4.cl
	python scripts/codegen-cholesky.py 5 > opencl/cholesky5.cl

	g++ -o bin/test-cvd-cl $(CFLAGS) $(LIBS) src/test.cc
	g++ -o bin/test-cholesky $(CFLAGS) $(LIBS) src/cholesky.cc

clean:
	rm -rf obj bin
