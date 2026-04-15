# Makefile for Text Editor System
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude
LDFLAGS =

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
TESTS_DIR = tests

# Source files (all .cpp files in src directory, excluding test files)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# Object files
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Executable name
TARGET = text_editor

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# 定义测试所需的依赖项（排除 main.o）
TEST_DEPS = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

test: all
	@echo "Running tests..."
	@for test_src in $(wildcard $(TESTS_DIR)/*.cpp); do \
		test_name=$$(basename $$test_src .cpp); \
		echo "---------------------------------------"; \
		echo "Building $$test_name..."; \
		$(CXX) $(CXXFLAGS) -I. $$test_src $(TEST_DEPS) -o $(BUILD_DIR)/$$test_name; \
		echo "Running $$test_name..."; \
		./$(BUILD_DIR)/$$test_name; \
	done
	@echo "---------------------------------------"
	@echo "All tests execution finished."

# Phony targets
.PHONY: all clean test