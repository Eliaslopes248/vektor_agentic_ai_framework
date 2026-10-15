# Copyright 2026, Elias Lopes
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Elias Lopes nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.



# This project uses QTest Framework for all unit testing
# this library is not statically linked to the final binary
# so its expected that you supply yourself with the library.
# This file will have default paths for the search lib paths.

# Basic C++ compiler definitions and FLAGS
CXX=clang++

# NOTE: I use optimize 3 for faster production speed
# 		This will result in a slower build time.
CPP_FLAGS=-std=c++17 -Wall


# Path to root Makefile
ROOT_DIR=..
ROOT_BUILD_DIR = $(ROOT_DIR)/build

# /unittests subdirectories
UT_INCLUDE_DIR	= ./include 
UT_LIBS_DIR	 	= ./libs
UT_SRC_DIR		= ./src
UT_BUILD_DIR	= ./build

# Build directory sub directories
UT_BUILD_BIN_DIR  = $(UT_BUILD_DIR)/bin
UT_BUILD_OBJ_DIR  = $(UT_BUILD_DIR)/objects
UT_BUILD_LIBS_DIR = $(UT_BUILD_DIR)/libs

# Get all source .cpp files and resolve expected object file paths
UT_SRC_FILES = $(shell find $(UT_SRC_DIR) -type f -name '*.cpp')
UT_OBJ_FILES = $(patsubst $(UT_SRC_DIR)/%.cpp, $(UT_BUILD_OBJ_DIR)/%.o, $(UT_SRC_FILES))

UT_INCLUDE_PATH_LIST = -Iinclude \
						-I$(ROOT_DIR)/include

# Commands to get all QT Paths
QT_CFLAGS := $(shell pkg-config --cflags Qt6Test)
QT_LIBS   := $(shell pkg-config --libs Qt6Test Qt6Core)

# Aggregate all search paths for libraries
UT_LIB_PATH_LIST = -L$(ROOT_BUILD_DIR)/libs
UT_RPATH_LIST = -Wl,-rpath,@executable_path/../../../build/libs


# Aggregate all -l library names
UT_LIB_LIST = -lvektor
				

# Create unittests/build/* directory and child directories
.PHONY: ut_build_dir_setup
ut_build_dir_setup:
	@mkdir -p ${UT_BUILD_LIBS_DIR}
	@mkdir -p ${UT_BUILD_OBJ_DIR}
	@mkdir -p ${UT_BUILD_BIN_DIR}


# TARGET EXECUTABLE AND TARGET Vektor LIBRARY TO TEST ON
UT_EXE	   = $(UT_BUILD_BIN_DIR)/unittests 
VEKTOR_LIB = $(ROOT_BUILD_DIR)/libs/libvektor.so


# Call sub make file to build Vektor shared library
# Add .PHONY short cut for calling target
.PHONY: build-vektor 
build-vektor: $(VEKTOR_LIB)

# Call make target from top level Makefile
$(VEKTOR_LIB):
	$(MAKE) -C $(ROOT_DIR) lib

# Compile .cpp files into object files
$(UT_BUILD_OBJ_DIR)/%.o : $(UT_SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPP_FLAGS) \
	$(QT_CFLAGS) $(UT_INCLUDE_PATH_LIST) \
	-c $< -o $@

.PHONY: ut 
ut: ut_build_dir_setup build-vektor $(UT_EXE)

# Compile ./unittest executable
$(UT_EXE): $(UT_OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $(CPP_FLAGS) \
	$^ 			\
	$(QT_LIBS) \
	$(UT_LIB_PATH_LIST) \
	$(UT_RPATH_LIST) \
	$(UT_LIB_LIST) \
	-o $@

# Clean up targets
.PHONY: clean
clean:
	@if [ -d "$(UT_BUILD_DIR)" ]; then \
		rm -r "$(UT_BUILD_DIR)"; \
	fi 
	$(MAKE) ut_build_dir_setup