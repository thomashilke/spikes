CXX = clang++-3.8
DEPS_BIN = clang++-3.8
CXXFLAGS = -g -std=c++11
LDFLAGS = -g
LDLIB = -lpthread
AR = ar
ARFLAGS = rc

PREFIX = ~/.local/
BIN_DIR = bin/
INCLUDE_DIR = include/spikes/
LIB_DIR = lib/


SOURCES = test/array.cpp test/image.cpp test/thread_pool.cpp

HEADERS = include/array.hpp include/thread_pool.hpp

BIN = bin/test_array bin/test_image bin/test_thread_pool

bin/test_array: build/test/array.o
bin/test_image: build/test/image.o
bin/test_thread_pool: build/test/thread_pool.o
#bin/...: ...

LIB = 

#lib/...: ...
