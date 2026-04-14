# Makefile for Text Editor System
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8
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

# Run tests
test: all
	@echo "Running tests..."
	@for test in $(TESTS_DIR)/test_*.cpp; do \
		test_name=$$(basename $$test .cpp); \
		echo "Building $$test_name..."; \
		$(CXX) $(CXXFLAGS) -I. $$test -o $(BUILD_DIR)/$$test_name 2>&1 | grep -v "warning: " || true; \
		echo "Running $$test_name..."; \
		./$(BUILD_DIR)/$$test_name 2>&1; \
	done

# Phony targets
.PHONY: all clean test