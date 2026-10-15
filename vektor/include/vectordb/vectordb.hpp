// Copyright 2026, Elias Lopes
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Elias Lopes nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <filesystem>
#include <iostream>

#include "collections.hpp"
#include "utils/rc.hpp"


// -DDEBUG macro for DEBUG mode path changes
// to a debug directory
#ifdef DEBUG
    #define DEFAULT_DATA_STORE_DIR "/tmp/vektor/debug/data_store"
#else  
    #define DEFAULT_DATA_STORE_DIR "/tmp/vektor/data_store"
#endif

#define DEFAULT_BACKEND_FILE "vektor.sqlite3"

namespace fs = std::filesystem;

namespace vektor {

    // Exception base class for any errors
    class vector_db_error : public std::exception {
    public:
        std::string msg;
        int code;

        explicit vector_db_error(const std::string& msg)
            : vector_db_error(msg, 1) {}

        vector_db_error(const std::string& msg, int code)
            : msg(msg), code(code) {}

        const char* what() const noexcept override {
            return msg.c_str();
        }
    };

    // This class is the top level client for interacting with
    // vector databases in the vektor framework. It has the standard
    // vector database operation and aggregates sub components as helper.
    class vector_db {

    public:
        
        // Absolute path to directory where database is located
        fs::path directory;

        // Persistent database file name
        fs::path backend_file;

        // Construct the enviroment for vector database
        // The directory will be created if not exists
        vector_db()
            : vector_db(DEFAULT_DATA_STORE_DIR, DEFAULT_BACKEND_FILE) {};
        vector_db(
            const fs::path dir_path, const fs::path backend_file);


    private:

        // Creates the data store directory and opens a backend file
        RC create_data_store(
            const fs::path dir_path, const fs::path backend_file);
        

    };

    // Checks if a directory/file has read permissions
    bool has_read_permission(const fs::path& path);

}