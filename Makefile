CFLAGS   = -std=gnu++98 -fopenmp -pthread -O3 -march=native -ffast-math
CFLAGS  += -I/usr/local/include -Iinclude -Isrc
CFLAGS  += -Wall -Wextra -Wno-unused -Wno-ignored-qualifiers -fmessage-length=0
#CFLAGS += -g3

BINARY   = bin/test-cvd-cl
SOURCES  = $(shell find src -iname "*.cc")
LIBS     = -lOpenCL -lm -lboost_program_options-mt

all:
	mkdir -p obj bin
	g++ -o $(BINARY) $(CFLAGS) $(SOURCES) $(LIBS)

clean:
	rm -rf obj bin
