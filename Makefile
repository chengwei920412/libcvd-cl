CFLAGS   = -std=gnu++98 -fopenmp -pthread -O3 -march=native -ffast-math
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
#CFLAGS += -g3

LIBS     = -lOpenCL -lcvd -lm -lboost_program_options-mt

SOURCES  = $(shell ls src/*/*.cc)

all:
	mkdir -p obj bin

	scripts/codegen-blur.py | tee opencl/blur.cl | scripts/embed.py OCL_BLUR > src/kernels/blur.hh
	scripts/codegen-cull.py | tee opencl/cull.cl | scripts/embed.py OCL_CULL > src/kernels/cull.hh
	scripts/codegen-fast.py | tee opencl/fast.cl | scripts/embed.py OCL_FAST > src/kernels/fast.hh
	scripts/codegen-filt.py | tee opencl/filt.cl | scripts/embed.py OCL_FILT > src/kernels/filt.hh
	scripts/codegen-hips.py | tee opencl/hips.cl | scripts/embed.py OCL_HIPS > src/kernels/hips.hh
	scripts/codegen-find.py | tee opencl/find.cl | scripts/embed.py OCL_FIND > src/kernels/find.hh

	scripts/codegen-cholesky.py 3 | tee opencl/cholesky3.cl | scripts/embed.py OCL_CHOLESKY_3 > src/kernels/cholesky3.hh
	scripts/codegen-cholesky.py 4 | tee opencl/cholesky4.cl | scripts/embed.py OCL_CHOLESKY_4 > src/kernels/cholesky4.hh
	scripts/codegen-cholesky.py 5 | tee opencl/cholesky5.cl | scripts/embed.py OCL_CHOLESKY_5 > src/kernels/cholesky5.hh

	g++ -shared -fPIC -o bin/libcvdcl.so $(CFLAGS) $(LIBS) $(SOURCES)

	g++ -o bin/test-cvd-cl $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test.cc
	g++ -o bin/test-cholesky $(CFLAGS) $(LIBS) bin/libcvdcl.so src/cholesky.cc

clean:
	rm -rf obj bin
