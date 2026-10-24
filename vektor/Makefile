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

# Resolve platform specific file extensions
# Must identify which OS we're building for
TARGET_VEKTOR_LIB = $(BUILD_LIBS_DIR)/libvektor.so

# Include Search List
VEKTOR_INCLUDE_PATHS = -Iinclude

# Library search path
VEKTOR_LIB_PATHS = -Lbuild/libs
MAIN_LINK_LIBS = -lvektor -lsqlite3 
VEKTOR_LDFLAGS = -lsqlite3

UNAME_S 		:= $(shell uname -s)
SHARED_FLAGS 	:= 
INSTALL_NAME 	:= 
UT_RPATH_LIST 	:=

# Set flags for platform and set file extension for library file
ifeq ($(UNAME_S),Darwin)
    # macOS
    SHARED_FLAGS	:= -dynamiclib
    INSTALL_NAME 	:= -Wl,-install_name,@rpath/libvektor.so
    UT_RPATH_LIST 	:= -Wl,-rpath,@executable_path/../libs

	
	BOOST_PREFIX := $(shell brew --prefix boost)
    JSON_PREFIX := $(shell brew --prefix nlohmann-json)

    VEKTOR_INCLUDE_PATHS += -I$(BOOST_PREFIX)/include
    VEKTOR_INCLUDE_PATHS += -I$(JSON_PREFIX)/include

else ifeq ($(UNAME_S),Linux)
    # Linux
    SHARED_FLAGS := -shared
    INSTALL_NAME := -Wl,-soname,libvektor.so
    UT_RPATH_LIST := -Wl,-rpath,'$$ORIGIN/../libs'

else
    $(error Unsupported platform: $(UNAME_S))
endif

# Compile a source file into object file
$(BUILD_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp build_dir_setup
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) \
	$(VEKTOR_INCLUDE_PATHS) \
	-c $< -o $@

# Compile ./main and link libraries
$(TARGET_MAIN): $(VEKTOR_OBJECT_FILES)
	$(CXX) $(CXX_FLAGS) $^ \
	$(VEKTOR_LIB_PATHS) $(UT_RPATH_LIST) $(MAIN_LINK_LIBS) \
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
	$(CXX) $(SHARED_FLAGS) $(INSTALL_NAME) \
	$(VEKTOR_LDFLAGS)	\
	-o $@ \
	$^

# Clean up targets
.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
	$(MAKE) build_dir_setup

	


