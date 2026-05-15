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

# Auto-discover all .cpp files recursively (3 levels)
SRCS = $(wildcard $(SRC_DIR)/*.cpp) \
       $(wildcard $(SRC_DIR)/*/*.cpp) \
       $(wildcard $(SRC_DIR)/*/*/*.cpp)

# Object files mirror the source tree under build/
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Executable name
TARGET = text_editor

# Default target
all: $(TARGET)

# Link executable
$(TARGET): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

# Compile source files (auto-create output directory before compiling)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@) 2>/dev/null; true
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@rm -rf $(BUILD_DIR) $(TARGET).exe 2>/dev/null; true

# Test dependencies (all .o files except main.o)
TEST_DEPS = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

# Pattern rule: build test executables (e.g. build/test_commandparser.exe from tests/test_commandparser.cpp)
$(BUILD_DIR)/%.exe: $(TESTS_DIR)/%.cpp $(TEST_DEPS)
	$(CXX) $(CXXFLAGS) -I. $< $(TEST_DEPS) $(LDFLAGS) -o $@

# Test targets — one per test file
test_commandparser:    $(BUILD_DIR)/test_commandparser.exe
	./$<
test_commands:         $(BUILD_DIR)/test_commands.exe
	./$<
test_documentmanager:  $(BUILD_DIR)/test_documentmanager.exe
	./$<
test_edit_duration:    $(BUILD_DIR)/test_edit_duration.exe
	./$<
test_editor_factory:   $(BUILD_DIR)/test_editor_factory.exe
	./$<
test_engine:           $(BUILD_DIR)/test_engine.exe
	./$<
test_log:              $(BUILD_DIR)/test_log.exe
	./$<
test_log_recovery:     $(BUILD_DIR)/test_log_recovery.exe
	./$<
test_loggermanager:    $(BUILD_DIR)/test_loggermanager.exe
	./$<
test_outputservice:    $(BUILD_DIR)/test_outputservice.exe
	./$<
test_spell_check:      $(BUILD_DIR)/test_spell_check.exe
	./$<
test_workspace:        $(BUILD_DIR)/test_workspace.exe
	./$<
test_xml_commands:     $(BUILD_DIR)/test_xml_commands.exe
	./$<
test_xml_integration:  $(BUILD_DIR)/test_xml_integration.exe
	./$<

# Run all tests (utf8 target switches terminal to UTF-8 before tests)
utf8:
	@chcp 65001 >nul 2>&1 || true

test: all utf8 \
	test_commandparser test_commands test_documentmanager \
	test_edit_duration test_editor_factory test_engine \
	test_log test_log_recovery test_loggermanager \
	test_outputservice test_spell_check test_workspace \
	test_xml_commands test_xml_integration
	@echo ========================================
	@echo All 14 test suites finished.

# Phony targets
.PHONY: all clean test utf8 \
	test_commandparser test_commands test_documentmanager \
	test_edit_duration test_editor_factory test_engine \
	test_log test_log_recovery test_loggermanager \
	test_outputservice test_spell_check test_workspace \
	test_xml_commands test_xml_integration
