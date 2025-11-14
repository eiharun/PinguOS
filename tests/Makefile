CXXFLAGS := -m32 -std=c++17 -Wall -Wextra -O2 -Itests/include -I../include  -I./googletest/googletest/include

BUILD_ROOT ?= ../build

GTEST_LIB := googletest/build/lib/libgtest.a
GTEST_MAIN := googletest/build/lib/libgtest_main.a
GTEST_INC := googletest/googletest/include

LIB_SRC := \
	../src/filesystem/fat.cpp \
	../src/allocator.cpp \
# 	../src/filesystem/msdospart.cpp \

LIB_OBJ := $(LIB_SRC:../src/%.cpp=$(BUILD_ROOT)/test/lib/%.o)

TEST_SUPPORT_SRC := host_alloc.cpp
TEST_SUPPORT_OBJS := $(TEST_SUPPORT_SRC:%.cpp=$(BUILD_ROOT)/test/obj/%.o)

TEST_SRC := $(shell find unit -name '*.cpp') 
TEST_BIN := $(TEST_SRC:%.cpp=$(BUILD_ROOT)/test/bin/%)

.PHONY: all clean gtest

all: gtest $(TEST_BIN)

gtest:
	@[ -d googletest/build ] || (cd googletest && cmake -S . -B build && cmake --build build)

$(BUILD_ROOT)/test/lib/%.o: ../src/%.cpp
	@mkdir -p $(dir $@)
	g++ $(CXXFLAGS) -c $< -o $@

$(BUILD_ROOT)/test/obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ $(CXXFLAGS) -c $< -o $@

$(BUILD_ROOT)/test/bin/%: $(BUILD_ROOT)/test/obj/%.o $(LIB_OBJ) $(TEST_SUPPORT_OBJS) $(GTEST_LIB) $(GTEST_MAIN)
	@mkdir -p $(dir $@)
	g++ $^ -m32 -pthread -lstdc++ -o $@

clean:
	rm -rf $(BUILD_ROOT)/test