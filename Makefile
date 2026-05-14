# Makefile for Text Editor System (plugin-ready package structure)
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 \
	-Iinclude/common \
	-Iinclude/command -Iinclude/workspace -Iinclude/service \
	-Iinclude/log -Iinclude/event -Iinclude/editor -Iinclude/util \
	-Iinclude/text -Iinclude/xml \
	-Iinclude/plugin/spell -Iinclude/plugin/stats
LDFLAGS = -lwinhttp

# Directories
SRC_DIR = src
BUILD_DIR = build
TESTS_DIR = tests

# Auto-discover all .cpp files in src/ recursively (supports plugin packages)
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
# Object files mirror the source tree under build/
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Executable name
TARGET = text_editor

# Default target
all: dirs $(TARGET)

# Create build directories matching source tree
dirs:
	@for d in $$(find $(SRC_DIR) -type d); do \
		mkdir -p $(BUILD_DIR)/$${d#$(SRC_DIR)/}; \
	done

# Link executable
$(TARGET): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

# Compile source files (pattern rule for subdirectories)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Test dependencies (all .o files except main.o)
TEST_DEPS = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

# Run all tests
test: all
	@echo "Running tests..."
	@for test_src in $(wildcard $(TESTS_DIR)/*.cpp); do \
		test_name=$$(basename $$test_src .cpp); \
		echo "---------------------------------------"; \
		echo "Building $$test_name..."; \
		$(CXX) $(CXXFLAGS) -I. $$test_src $(TEST_DEPS) $(LDFLAGS) -o $(BUILD_DIR)/$$test_name; \
		echo "Running $$test_name..."; \
		./$(BUILD_DIR)/$$test_name; \
	done
	@echo "---------------------------------------"
	@echo "All tests execution finished."

# Phony targets
.PHONY: all clean test dirs
