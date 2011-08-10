CFLAGS   = -std=gnu++98 -fopenmp -pthread -O3 -march=native -ffast-math
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
#CFLAGS += -g3

LIBS     = -lOpenCL -lcvd -lm -lboost_program_options-mt

SOURCES  = $(shell ls src/*/*.cc)

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

	python scripts/embed.py OCL_BLUR < opencl/blur.cl > src/kernels/blur.hh
	python scripts/embed.py OCL_CULL < opencl/cull.cl > src/kernels/cull.hh
	python scripts/embed.py OCL_FAST < opencl/fast.cl > src/kernels/fast.hh
	python scripts/embed.py OCL_FILT < opencl/filt.cl > src/kernels/filt.hh
	python scripts/embed.py OCL_HIPS < opencl/hips.cl > src/kernels/hips.hh
	python scripts/embed.py OCL_FIND < opencl/find.cl > src/kernels/find.hh

	python scripts/embed.py OCL_CHOLESKY_3 < opencl/cholesky3.cl > src/kernels/cholesky3.hh
	python scripts/embed.py OCL_CHOLESKY_4 < opencl/cholesky4.cl > src/kernels/cholesky4.hh
	python scripts/embed.py OCL_CHOLESKY_5 < opencl/cholesky5.cl > src/kernels/cholesky5.hh

	g++ -shared -fPIC -o bin/libcvdcl.so $(CFLAGS) $(LIBS) $(SOURCES)

	g++ -o bin/test-cvd-cl $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test.cc
	g++ -o bin/test-cholesky $(CFLAGS) $(LIBS) bin/libcvdcl.so src/cholesky.cc

clean:
	rm -rf obj bin
