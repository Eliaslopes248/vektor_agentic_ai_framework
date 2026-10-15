# Author: Elias Lopes
#
# Desc:	This Makefile will have all the main build targets
# 		for this project. It will also have the main build targets
# 		testing infrastructure too.
#
#		This project is mainly a singular library that will be 
#		by other projects, however in this project we do have a
#		src/main.cpp that can be used to run quick checks of code.
#
# Usage:
# 	make [TARGET_NAME]


# Basic C++ compiler definitions and FLAGS
CXX=clang++

# NOTE: I use optimize 3 for faster production speed
# 		This will result in a slower build time.
CXX_FLAGS=-std=c++17 -Wall

# Define project top level directories
BUILD_DIR	 = ./build
INCLUDE_DIR	 = ./include 
LIBS_DIR	 = ./libs
SRC_DIR		 = ./src
UNITTEST_DIR = ./unittests

# Build directory targets and paths
BUILD_BIN_DIR  = $(BUILD_DIR)/bin
BUILD_OBJ_DIR  = $(BUILD_DIR)/objects
BUILD_LIBS_DIR = $(BUILD_DIR)/libs

# Target to ensure the build directory has it's
# needed sub directories before other operations can be done
.PHONY: build_dir_setup
build_dir_setup:
	@mkdir -p ${BUILD_BIN_DIR}
	@mkdir -p ${BUILD_OBJ_DIR}
	@mkdir -p ${BUILD_LIBS_DIR}


# Compiling source .cpp files and convert to object files
VEKTOR_SOURCE_CPP_FILES := $(shell find $(SRC_DIR) -type f -name '*.cpp')
VEKTOR_OBJECT_FILES 	 = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_OBJ_DIR)/%.o, $(VEKTOR_SOURCE_CPP_FILES))

# Target output executables and libfiles
TARGET_MAIN 	  = $(BUILD_BIN_DIR)/main 
TARGET_VEKTOR_LIB = $(BUILD_LIBS_DIR)/libvektor.so

# Include Search List
VEKTOR_INCLUDE_PATHS = -Iinclude

# Library search path
VEKTOR_LIB_PATHS = -Lbuild/libs
VEKTOR_LINK_LIBS = -lvektor
UT_RPATH_LIST = -Wl,-rpath,@executable_path/../libs

# Compile a source file into object file
$(BUILD_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp build_dir_setup
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) \
	$(VEKTOR_INCLUDE_PATHS) \
	-c $< -o $@

# Compile ./main and link libraries
$(TARGET_MAIN): $(VEKTOR_OBJECT_FILES)
	$(CXX) $(CXX_FLAGS) $^ \
	$(VEKTOR_LIB_PATHS) $(UT_RPATH_LIST) $(VEKTOR_LINK_LIBS) \
	-o $@



.PHONY: main
main: lib $(TARGET_MAIN)

.PHONY: debug
debug: CXX_FLAGS += -DDEBUG
debug: clean
debug: main

# Build main executable
# Add -DDEBUG macro
# Run debug program
.PHONY: drun
drun: debug 
	$(TARGET_MAIN)

.PHONY: lib 
lib: build_dir_setup $(TARGET_VEKTOR_LIB)

# Target to build shared library
$(TARGET_VEKTOR_LIB): $(VEKTOR_OBJECT_FILES)
	@echo "--------------------------------"
	@echo "Building shared library..."
	@echo "--------------------------------"
	$(CXX) -dynamiclib \
	-install_name @rpath/libvektor.so \
	-o $@ \
	$^


# Clean up targets
.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
	$(MAKE) build_dir_setup

	


