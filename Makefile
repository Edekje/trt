# Adapted from:
# https://makefiletutorial.com/ (Good resource)
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

# Instructions:
# make or make build/trt		: compiles the trt project binary
# make build/<name of unittest>		: compiles a unittest binary
# make clean 				: deletes all compiled files & deps.

# Compiler settings
CXX = g++
CXXFLAGS = -g
LDFLAGS= -g # Linker flags

# Relevant Directories
BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests

# Project source files except main.cpp
SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp' ! -name 'main.cpp')
# Main project main.cpp:
MAIN_SRC_FILE := $(SRC_DIR)/main.cpp
# Test source files (each one contains a main function)
TEST_SRC_FILES := $(shell find $(TEST_DIR) -name '*.cpp')

# Object file destinations
# E.g. src/file.cpp is sent to build/src/file.cpp.o .
OBJ_FILES := $(SRC_FILES:%=$(BUILD_DIR)/%.o)
MAIN_OBJ_FILE := $(MAIN_SRC_FILE:%=$(BUILD_DIR)/%.o)
# TEST_OBJ_FILES := $(TEST_SRC_FILES:%=$(BUILD_DIR)/%.o) # Now superfluous

# Main project executable targeti:
MAIN_TARGET_EXEC := $(BUILD_DIR)/trt # Name of final main.cpp binary, in BUILD_DIR.
# Target executables for each unit test
TEST_TARGET_EXECS := $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/%,$(TEST_SRC_FILES))

# Pass each folder with headers to GCC so that it can localise headers.
INC_DIRS := $(shell find $(SRC_DIR) -type d) $(shell find $(TEST_DIR) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# -MMD & -MP create dependency .d files alongside the .o files
CPPFLAGS := $(INC_FLAGS) -MMD -MP

# Link the main project into the final executable:
$(MAIN_TARGET_EXEC) : $(OBJ_FILES) $(MAIN_OBJ_FILE)
	$(CXX) $(OBJ_FILES) $(MAIN_OBJ_FILE) -o $@ $(LDFLAGS)

# Link a single unit test mainfile into an executable:
#.SECONDEXPANSION is necessary for delayed evaluation of the prerequisite with $$() / $$@
.SECONDEXPANSION:
$(TEST_TARGET_EXECS) : $$(patsubst $(BUILD_DIR)/%,$(BUILD_DIR)/$(TEST_DIR)/%.cpp.o,$$@) $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) $< -o $@ $(LDFLAGS)

# Compile required .o files from the corresponding src file.
# Make directories, and parent directories if necessary,
# in order to mirror the source structure:
$(BUILD_DIR)/%.cpp.o : %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean # .PHONY means this is still executed even if a file 'clean' existed
clean:
	rm -r $(BUILD_DIR)/*

# Include .d makefiles. '-' suppresses errors from missing ones.
DEPS := $(OBJ_FILES:.o=.d) $(TEST_OBJ_FILES:.o=.d)
-include $(DEPS)

