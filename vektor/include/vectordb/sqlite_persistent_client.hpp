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
#include <sqlite3.h>

#include "vectordb.hpp"
#include "persistentclient.hpp"

#define SQLITE3_EXE "sqlite3"

namespace fs = std::filesystem;

// Wrap inside namespace to avoid naming
// clashing with custom namespace
namespace database {
    typedef sqlite3 sqlite3;
}


namespace vektor{

    namespace sqlite3 {

        // SQLite data types
        enum data_types {
            _int,
            _text,
            _blob
        };

        // SQL row
        typedef std::vector<std::string> sqlite3_row;

        // SQL result
        typedef std::vector<sqlite3_row> sqlite3_results;

        // SQL query callback for parsing results
        static int sqlite3_callback(
            void* data, int column_count, 
            char** values, char** column_names);

        // Custom exception classes for more specific 
        // error and exception handling
        struct sqlite_client_error : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };
        struct sqlite_cmd_error : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };
        struct sqlite_database_error : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };
        struct sqlite_query_error : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };
        struct sqlite_fk_error : public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

    };

    // Base class for all persistent vector databases
    // This class should not have direct instances
    class sqlite_persistent_client : public persistent_client {
        public: 

        sqlite_persistent_client(fs::path file);
        sqlite_persistent_client() 
            : sqlite_persistent_client(
                static_cast<fs::path>(DEFAULT_DATA_STORE_DIR) / 
                static_cast<fs::path>(DEFAULT_BACKEND_FILE)) {};

        ~sqlite_persistent_client();


        // Creates a collection inside vector database
        // so doc/embeddings can be upserted into collection namespaces
        vektor::response<collection::Collection> create_collection(
            const std::string id, const std::string name, const unsigned int dimension, const collection::metric metric) override;
        
        vektor::response<collection::Collection> create_collection(
            const std::string name, const unsigned int dimension, const collection::metric metric) override
            {
                const std::string id = uuid();
                return create_collection(id, name, dimension, metric);
            }

        vektor::response<collection::Collection> create_collection(
            const std::string name, const unsigned int dimension) override
            {
                return create_collection(name, dimension, collection::metric::cosine);
            }
        
        vektor::response<collection::Collection> create_collection(
            const std::string name) override 
            {
                return create_collection(name, 384, collection::metric::cosine);
            }

        
        // Adding records to vector database
        // insert() creates a new record and upsert() updates a existing record
        vektor::response<documents::document_status> insert(
            std::string         id, 
            std::string         collection_id, 
            std::string         content, 
            std::vector<float>  embeddings,
            documents::json     metadata) override;

        vektor::response<documents::document_status> upsert(
            std::string         id, 
            std::string         collection_id, 
            std::string         content, 
            std::vector<float>  embeddings,
            documents::json     metadata) override;

        // Access and remove existing collections
        collection::Collection get_collection(std::string id, collection::collection_status* err) override;
        collection::Collection get_collection(std::string id) {
            return get_collection(id, nullptr);
        }
        documents::document get_record(std::string id, documents::document_status* err) override;
        documents::document get_record(std::string id) {
            return get_record(id, nullptr);
        }

        vektor::response<documents::document_status> remove_record(std::string id) override;
        vektor::response<documents::document_status> remove_collection(std::string collection_id) override;

        fs::path get_sqlite_file() {
            return this->sqlite_file;
        }

        private:

        database::sqlite3* __database;

        const inline static fs::perms min_file_perms = 
                                        fs::perms::group_write | fs::perms::owner_write | fs::perms::others_write |
                                        fs::perms::group_read | fs::perms::owner_read | fs::perms::others_read;

        fs::path sqlite_file;

        static int sqlite3_callback(
            void* data, int column_count, 
            char** values, char** column_names);

        // SQL Helper methods
        std::string current_timestamp();
        std::string uuid();
        

        // Execute SQLite3 command on target file
        RC execute_sqlite_cmd(std::string command, fs::path file);
        RC execute_sqlite_cmd(std::string command);

        // Create all tables in database
        RC init_database(fs::path file);
        RC init_database();

        // Deletes database file
        RC destroy_database(fs::path file);
        RC destroy_database();
        
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