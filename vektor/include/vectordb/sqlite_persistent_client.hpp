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
#include <cstdlib>


#include "persistentclient.hpp"

#define SQLITE3_EXE "sqlite3"

namespace fs = std::filesystem;

namespace vektor{

    // Custom exception classes for more specific 
    // error and exception handling
    struct sqlite_client_error : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    struct sqlite_cmd_error : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };


    // Base class for all persistent vector databases
    // This class should not have direct instances
    class sqlite_persistent_client : public persistent_client {
        public: 

        sqlite_persistent_client(fs::path file);

        // Creates a collection inside vector database
        // so doc/embeddings can be upserted into collection namespaces
        collection::Collection create_collection(
            const std::string name, const unsigned int dimension, const collection::metric metric) override;
        
        collection::Collection create_collection(
            const std::string name, const unsigned int dimension) override
            {
                return create_collection(name, dimension, collection::metric::cosine);
            }
        
        collection::Collection create_collection(
            const std::string name) override 
            {
                return create_collection(name, 384, collection::metric::cosine);
            }

        
        // Adding records to vector database
        // insert() creates a new record and upsert() updates a existing record
        RC insert(
            std::string id, 
            std::string collection_id, 
            std::string content, 
            std::map<std::string, std::string> metadata) override;
        RC upsert(
            std::string id, 
            std::string collection_id, 
            std::string content, 
            std::map<std::string, std::string> metadata) override;

        // Access and remove existing collections
        collection::Collection get(std::string id) override;
        RC remove_record(std::string id) override;
        RC remove_collection(std::string collection_id) override;

        RC execute_sqlite_cmd(std::string command);

        private:

        const inline static fs::perms min_file_perms = 
                                        fs::perms::group_write | fs::perms::owner_write | fs::perms::others_write |
                                        fs::perms::group_read | fs::perms::owner_read | fs::perms::others_read;

        fs::path sqlite_file;

        // Execute SQLite3 command on target file
        // RC execute_sqlite_cmd(std::string command);
        RC execute_sqlite_cmd(std::string command, fs::path file);
        RC execute_sqlite_cmd(std::string command);

        // Create all tables in database
        RC init_database(fs::path file);
        RC init_database();
        
        // Construct SQLite command from vector of strings
        std::string construct_sqlite_cmd(std::vector<std::string> args);

        // Apply filesystem permissions to a directory or file
        RC apply_fs_perm(
            const fs::path path, const fs::perms perms, const bool replace);
        RC apply_fs_perm(
            const fs::path path, const fs::perms perms) {
                return apply_fs_perm(path, perms, false);
            };
    };

}