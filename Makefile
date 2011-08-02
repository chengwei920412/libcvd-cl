CFLAGS   = -std=gnu++98 -fopenmp -pthread -O3 -march=native -ffast-math
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
#CFLAGS += -g3

BINARY   = bin/test-cvd-cl
SOURCES  = $(shell find src -iname "*.cc")
LIBS     = -lOpenCL -lcvd -lm -lboost_program_options-mt

all:
	mkdir -p obj bin
	python scripts/codegen-blur.py > opencl/blur.cl
	python scripts/codegen-fast.py > opencl/fast.cl
	python scripts/codegen-filt.py > opencl/filt.cl
	python scripts/codegen-hips.py > opencl/hips.cl
	g++ -o $(BINARY) $(CFLAGS) $(SOURCES) $(LIBS)

clean:
	rm -rf obj bin
