CFLAGS   = -std=gnu++98 -fopenmp -pthread -O3 -march=native -ffast-math
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
#CFLAGS += -g3

LIBS     = -lOpenCL -lcvd -lm -lboost_program_options-mt

SOURCES  = $(shell ls src/*/*.cc)

all:
	mkdir -p obj bin

	scripts/codegen-blur-gray.py     | tee opencl/blur-gray.cl     | scripts/embed.py OCL_BLUR_GRAY      > src/kernels/blur-gray.hh
	scripts/codegen-blur-rich.py     | tee opencl/blur-rich.cl     | scripts/embed.py OCL_BLUR_RICH      > src/kernels/blur-rich.hh
	scripts/codegen-prefast-gray.py  | tee opencl/prefast-gray.cl  | scripts/embed.py OCL_PRE_FAST_GRAY  > src/kernels/prefast-gray.hh
	scripts/codegen-prefast-rich.py  | tee opencl/prefast-rich.cl  | scripts/embed.py OCL_PRE_FAST_RICH  > src/kernels/prefast-rich.hh
	scripts/codegen-fast-gray.py     | tee opencl/fast-gray.cl     | scripts/embed.py OCL_FAST_GRAY      > src/kernels/fast-gray.hh
	scripts/codegen-fast-rich.py     | tee opencl/fast-rich.cl     | scripts/embed.py OCL_FAST_RICH      > src/kernels/fast-rich.hh
	scripts/codegen-fast-best.py     | tee opencl/fast-best.cl     | scripts/embed.py OCL_FAST_BEST      > src/kernels/fast-best.hh
	scripts/codegen-hips-gray.py     | tee opencl/hips-gray.cl     | scripts/embed.py OCL_HIPS_GRAY      > src/kernels/hips-gray.hh
	scripts/codegen-hips-find.py     | tee opencl/hips-find.cl     | scripts/embed.py OCL_HIPS_FIND      > src/kernels/hips-find.hh

	scripts/codegen-cholesky.py 3    | tee opencl/cholesky3.cl     | scripts/embed.py OCL_CHOLESKY_3     > src/kernels/cholesky3.hh
	scripts/codegen-cholesky.py 4    | tee opencl/cholesky4.cl     | scripts/embed.py OCL_CHOLESKY_4     > src/kernels/cholesky4.hh
	scripts/codegen-cholesky.py 5    | tee opencl/cholesky5.cl     | scripts/embed.py OCL_CHOLESKY_5     > src/kernels/cholesky5.hh

	g++ -shared -fPIC -o bin/libcvdcl.so $(CFLAGS) $(LIBS) $(SOURCES)

	g++ -o bin/test-cvd-cl $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test.cc
	g++ -o bin/test-rich   $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test-rich.cc
	g++ -o bin/test-cholesky $(CFLAGS) $(LIBS) bin/libcvdcl.so src/cholesky.cc

clean:
	rm -rfv obj bin src/kernels/*.hh src/opencl/*.cl
