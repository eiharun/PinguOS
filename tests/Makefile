CXXFLAGS := -m32 -g -std=c++17 -Wall -Wextra -Itests/include -I../include  -I./googletest/googletest/include

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

UNIT_TEST_SRC := $(shell find unit -name 'test_*.cpp') 
UNIT_TEST_OBJ := $(UNIT_TEST_SRC:%.cpp=$(BUILD_ROOT)/test/obj/%.o)

MAIN_OBJ :=$(BUILD_ROOT)/test/obj/unit/main.o

TEST_ALL_BIN := $(BUILD_ROOT)/test/bin/all_tests

# TEST_SINGLE_BIN := $(UNIT_TEST_SRC:%.cpp=$(BUILD_ROOT)/test/bin/%)

TEST_BIN := $(TEST_ALL_BIN) #$(TEST_SINGLE_BIN)

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

# $(BUILD_ROOT)/test/bin/%: $(BUILD_ROOT)/test/obj/%.o $(LIB_OBJ) $(TEST_SUPPORT_OBJS) $(GTEST_LIB) $(GTEST_MAIN)
# 	@mkdir -p $(dir $@)
# 	g++ $^ -m32 -pthread -lstdc++ -o $@

# Build all tests at once
$(TEST_ALL_BIN): $(MAIN_OBJ) $(UNIT_TEST_OBJ) $(TEST_SUPPORT_OBJS) $(LIB_OBJ) $(GTEST_LIB) $(GTEST_MAIN)
	@mkdir -p $(dir $@)
	g++ $^ -m32 -pthread -o $@

# # Build individual test binaries
# $(BUILD_ROOT)/test/bin/unit/%: $(BUILD_ROOT)/test/obj/unit/%.o $(MAIN_OBJ) $(TEST_SUPPORT_OBJS) $(LIB_OBJ) $(GTEST_LIB) $(GTEST_MAIN)
# 	@mkdir -p $(dir $@)
# 	g++ $^ -m32 -pthread -o $@


clean:
	rm -rf $(BUILD_ROOT)/test