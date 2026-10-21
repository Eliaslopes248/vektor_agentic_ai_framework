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


#include <filesystem>
#include <fstream>

#include "vectordb/vectordb.hpp"
#include "utils/rc.hpp"

namespace fs = std::filesystem;

/**
 * @todo: Implement constructor
 * Add exception handlers
 * 
 */
vektor::vector_db::vector_db(
    const fs::path dir_path, const fs::path backend_file) {

    fs::path absolute_dir_path = fs::absolute(dir_path);

    RC rc;

    try {

        // Create data store directory and database file
        rc = this->create_data_store(absolute_dir_path, backend_file);

        if (rc != RC::OK){
            throw vektor::vector_db_error(
                "Unable to create data store environment",
                rc
            );
        }

    }catch (const vektor::vector_db_error& e){

    }catch(...) {

    }
   
}

RC vektor::vector_db::create_data_store(
        const fs::path dir_path, const fs::path backend_file) {

    if (!fs::exists(dir_path)){
            // Create data store directory
            try {
                bool open = fs::create_directories(dir_path);
                
                // throw failure to create directory
                if (!open){
                    throw fs::filesystem_error(
                        "Failed to create data directory",
                        fs::path(dir_path),
                        std::make_error_code(std::errc::permission_denied)
                    );
                } 
            
            }catch(const fs::filesystem_error& e){
                std::cout << 
                    "Error when trying to make data store directory: " 
                    << e.what() << std::endl;
                return RC::IO_ERROR;
            }
    }

    // ensure directory has read permissions
    bool has_read = vektor::has_read_permission(dir_path);

    // if no read permission, attempt to apply read permissions
    if (!has_read) {
        try {
            std::error_code ec;
            fs::permissions(
                dir_path,
                fs::perms::all,
                fs::perm_options::replace,
                ec
            );

            if (ec) {
                std::cerr << 
                    "Unable to apply permissions to " 
                    << dir_path << std::endl;
            }

        }catch(const fs::filesystem_error e){
            std::cerr << 
                    "Unable to apply permissions to " 
                    << dir_path << ": " << e.what() << std::endl;
            return RC::PERMISSION_ERROR;
        }
    }


    // create backend vector database file
    try {

        fs::path backend_file_path = dir_path / backend_file;
        std::ofstream backend(backend_file_path, std::ios::binary);

        if (!backend.is_open()){
            throw fs::filesystem_error(
                    "file couldnt be open for input and output.",
                    backend_file_path, 
                    std::make_error_code(std::errc::io_error)
                );
        }

    }catch (const fs::filesystem_error& e){
        std::cerr << "Unable to open backend file: " << e.what() << std::endl; 
        return RC::IO_ERROR;
    }

    return RC::OK;
}

bool vektor::has_read_permission(const fs::path& path){
        auto p = fs::status(path).permissions();

        return (p & (fs::perms::owner_read |
                    fs::perms::group_read |
                    fs::perms::others_read))
            != fs::perms::none;
    }