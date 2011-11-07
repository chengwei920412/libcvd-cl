CFLAGS   = -std=gnu++98 -fopenmp -pthread -O2 -march=native
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
CFLAGS  += -g3 -DCVD_IMAGE_DEBUG -O0
#CFLAGS  += -DCVD_CL_VERBOSE
#CFLAGS  += -DCVD_CL_DEBUG
#CFLAGS  += -DNDEBUG
CFLAGS  += -DBZ_DEBUG

C99FLAGS  = -std=gnu99 -fopenmp -pthread -O2 -march=native
C99FLAGS += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0

LIBS     = -lOpenCL -lGL -lcvd -lm -lblitz -lboost_program_options-mt

CSOURCES = $(shell ls src/*/*.c src/*/*/*.c)

SOURCES  = $(shell ls src/*/*.cc src/*/*/*.cc)

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
	scripts/codegen-hips-blend.py    | tee opencl/hips-blend.cl    | scripts/embed.py OCL_HIPS_BLEND     > src/kernels/hips-blend.hh
	scripts/codegen-hips-clip.py     | tee opencl/hips-clip.cl     | scripts/embed.py OCL_HIPS_CLIP      > src/kernels/hips-clip.hh
	scripts/codegen-hips-find.py     | tee opencl/hips-find.cl     | scripts/embed.py OCL_HIPS_FIND      > src/kernels/hips-find.hh
	scripts/codegen-hips-tfind.py    | tee opencl/hips-tfind.cl    | scripts/embed.py OCL_HIPS_TFIND     > src/kernels/hips-tfind.hh
	scripts/codegen-clip-depth.py    | tee opencl/clip-depth.cl    | scripts/embed.py OCL_CLIP_DEPTH     > src/kernels/clip-depth.hh

	scripts/codegen-cholesky.py 3    | tee opencl/cholesky3.cl     | scripts/embed.py OCL_CHOLESKY_3     > src/kernels/cholesky3.hh
	scripts/codegen-cholesky.py 4    | tee opencl/cholesky4.cl     | scripts/embed.py OCL_CHOLESKY_4     > src/kernels/cholesky4.hh
	scripts/codegen-cholesky.py 5    | tee opencl/cholesky5.cl     | scripts/embed.py OCL_CHOLESKY_5     > src/kernels/cholesky5.hh
	scripts/codegen-cholesky.py 6    | tee opencl/cholesky6.cl     | scripts/embed.py OCL_CHOLESKY_6     > src/kernels/cholesky6.hh

	scripts/codegen-mat-mul.py 3     | tee opencl/mat-mul-3.cl     | scripts/embed.py OCL_MAT_MUL_3      > src/kernels/mat-mul-3.hh
	scripts/codegen-mat-mul.py 4     | tee opencl/mat-mul-4.cl     | scripts/embed.py OCL_MAT_MUL_4      > src/kernels/mat-mul-4.hh
	scripts/codegen-mat-mul.py 5     | tee opencl/mat-mul-5.cl     | scripts/embed.py OCL_MAT_MUL_5      > src/kernels/mat-mul-5.hh
	scripts/codegen-mat-mul.py 6     | tee opencl/mat-mul-6.cl     | scripts/embed.py OCL_MAT_MUL_6      > src/kernels/mat-mul-6.hh

	scripts/codegen-mat-mki.py 3     | tee opencl/mat-mki-3.cl     | scripts/embed.py OCL_MAT_MKI_3      > src/kernels/mat-mki-3.hh
	scripts/codegen-mat-mki.py 4     | tee opencl/mat-mki-4.cl     | scripts/embed.py OCL_MAT_MKI_4      > src/kernels/mat-mki-4.hh
	scripts/codegen-mat-mki.py 5     | tee opencl/mat-mki-5.cl     | scripts/embed.py OCL_MAT_MKI_5      > src/kernels/mat-mki-5.hh
	scripts/codegen-mat-mki.py 6     | tee opencl/mat-mki-6.cl     | scripts/embed.py OCL_MAT_MKI_6      > src/kernels/mat-mki-6.hh

	scripts/codegen-wls-uvq.py       | tee opencl/wls-uvq.cl       | scripts/embed.py OCL_WLS_UVQ        > src/kernels/wls-uvq.hh
	scripts/codegen-se3-exp.py       | tee opencl/se3-exp.cl       | scripts/embed.py OCL_SE3_EXP        > src/kernels/se3-exp.hh
	scripts/codegen-se3-score.py     | tee opencl/se3-score.cl     | scripts/embed.py OCL_SE3_SCORE      > src/kernels/se3-score.hh
	scripts/codegen-se3-run1.py      | tee opencl/se3-run1.cl      | scripts/embed.py OCL_SE3_RUN1       > src/kernels/se3-run1.hh

	scripts/codegen-fxy.py           | tee opencl/fxy.cl           | scripts/embed.py OCL_FXY            > src/kernels/fxy.hh
	scripts/codegen-to-uvquv.py      | tee opencl/to-uvquv.cl      | scripts/embed.py OCL_TO_UVQUV       > src/kernels/to-uvquv.hh
	scripts/codegen-fmix.py          | tee opencl/fmix.cl          | scripts/embed.py OCL_FMIX           > src/kernels/fmix.hh
	scripts/codegen-random-int.py    | tee opencl/random-int.cl    | scripts/embed.py OCL_RANDOM_INT     > src/kernels/random-int.hh

	gcc -shared -fPIC -o bin/libcvdcl99.so $(C99FLAGS) $(CSOURCES) $(LIBS)
	g++ -shared -fPIC -o bin/libcvdcl.so $(CFLAGS) $(SOURCES) $(LIBS)

	g++ -o bin/test-cholesky $(CFLAGS) src/cholesky.cc bin/libcvdcl.so bin/libcvdcl99.so $(LIBS)
	g++ -o bin/test-se3-exp $(CFLAGS) src/test-se3.cc bin/libcvdcl.so bin/libcvdcl99.so $(LIBS)
	g++ -o bin/test-pose $(CFLAGS) src/test-pose.cc bin/libcvdcl.so bin/libcvdcl99.so $(LIBS)

	# Utilities
	g++ -o bin/img-to-txt $(CFLAGS) src/img-to-txt.cc -lcvd

clean:
	rm -rfv obj bin src/kernels/*.hh opencl/*.cl
