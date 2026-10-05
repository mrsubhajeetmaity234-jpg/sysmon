CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude -pthread
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:src/%.cpp=build/%.o)
TARGET   := bin/sysmon

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

run: all
	./$(TARGET)

test: all
	./tests/smoke_test.sh

clean:
	rm -rf build bin

-include $(OBJ:.o=.d)
.PHONY: all run test clean
