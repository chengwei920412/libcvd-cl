CFLAGS   = -std=gnu++98 -fopenmp -pthread -O0 -march=native
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
CFLAGS  += -g3 -DCVD_IMAGE_DEBUG

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
	scripts/codegen-hips-rich.py     | tee opencl/hips-rich.cl     | scripts/embed.py OCL_HIPS_RICH      > src/kernels/hips-rich.hh
	scripts/codegen-hips-find.py     | tee opencl/hips-find.cl     | scripts/embed.py OCL_HIPS_FIND      > src/kernels/hips-find.hh

	scripts/codegen-cholesky.py 3    | tee opencl/cholesky3.cl     | scripts/embed.py OCL_CHOLESKY_3     > src/kernels/cholesky3.hh
	scripts/codegen-cholesky.py 4    | tee opencl/cholesky4.cl     | scripts/embed.py OCL_CHOLESKY_4     > src/kernels/cholesky4.hh
	scripts/codegen-cholesky.py 5    | tee opencl/cholesky5.cl     | scripts/embed.py OCL_CHOLESKY_5     > src/kernels/cholesky5.hh
	scripts/codegen-cholesky.py 6    | tee opencl/cholesky6.cl     | scripts/embed.py OCL_CHOLESKY_6     > src/kernels/cholesky6.hh

	scripts/codegen-wls-uvq.py       | tee opencl/wls-uvq.cl       | scripts/embed.py OCL_WLS_UVQ        > src/kernels/wls-uvq.hh
	scripts/codegen-se3-exp.py       | tee opencl/se3-exp.cl       | scripts/embed.py OCL_SE3_EXP        > src/kernels/se3-exp.hh
	scripts/codegen-se3-score.py     | tee opencl/se3-score.cl     | scripts/embed.py OCL_SE3_SCORE      > src/kernels/se3-score.hh

	scripts/codegen-fxy.py           | tee opencl/fxy.cl           | scripts/embed.py OCL_FXY            > src/kernels/fxy.hh
	scripts/codegen-fmix.py          | tee opencl/fmix.cl          | scripts/embed.py OCL_FMIX           > src/kernels/fmix.hh
	scripts/codegen-random-int.py    | tee opencl/random-int.cl    | scripts/embed.py OCL_RANDOM_INT     > src/kernels/random-int.hh

	g++ -shared -fPIC -o bin/libcvdcl.so $(CFLAGS) $(LIBS) $(SOURCES)

	g++ -o bin/test-cvd-cl $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test.cc
	g++ -o bin/test-rich   $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test-rich.cc
	g++ -o bin/test-cholesky $(CFLAGS) $(LIBS) bin/libcvdcl.so src/cholesky.cc
	g++ -o bin/test-se3-exp $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test-se3.cc
	g++ -o bin/test-pose $(CFLAGS) $(LIBS) bin/libcvdcl.so src/test-pose.cc

clean:
	rm -rfv obj bin src/kernels/*.hh opencl/*.cl
