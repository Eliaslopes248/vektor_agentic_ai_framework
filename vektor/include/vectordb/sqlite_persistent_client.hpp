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

        /**
         * @brief Enum that represents SQLite data types
         */
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

        /**
         * @brief Custom exception classes for more specific
         *      error and exception handling
         */
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


        /**
         * @brief Create a collection in SQLite vector store,
         *        and returns a Collection struct payload
         * @param id 
         * @param name 
         * @param dimension 
         * @param metric 
         * @return vektor::response<collections::collection> 
         */
        vektor::response<collections::collection> create_collection(
            const std::string id, const std::string name, const unsigned int dimension, const collections::metric metric) override;
        vektor::response<collections::collection> create_collection(
            const std::string name, const unsigned int dimension, const collections::metric metric) override
            {
                const std::string id = uuid();
                return create_collection(id, name, dimension, metric);
            }
        vektor::response<collections::collection> create_collection(
            const std::string name, const unsigned int dimension) override
            {
                return create_collection(name, dimension, collections::metric::cosine);
            }
        vektor::response<collections::collection> create_collection(
            const std::string name) override 
            {
                return create_collection(name, 384, collections::metric::cosine);
            }

        
        /**
         * @brief Inserts a document into SQLite vector store,
         *        document must be a new document
         * @param id 
         * @param collection_id 
         * @param content 
         * @param embeddings 
         * @param metadata 
         * @return vektor::response<documents::document_status> 
         */
        vektor::response<documents::document_status> insert(
            std::string         id, 
            std::string         collection_id, 
            std::string         content, 
            std::vector<float>  embeddings,
            documents::json     metadata) override;


        /**
         * @brief Updates a document row in SQLite documents
         *        table. Document must already exist for
         *        method to successfully work
         * @param id 
         * @param collection_id 
         * @param content 
         * @param embeddings 
         * @param metadata 
         * @return vektor::response<documents::document_status> 
         */
        vektor::response<documents::document_status> upsert(
            std::string         id, 
            std::string         collection_id, 
            std::string         content, 
            std::vector<float>  embeddings,
            documents::json     metadata) override;
            


        /**
         * @brief Get the collection object from SQLite
         *        vector store. Error pointer is given
         *        value upon failure.
         * @param id 
         * @param err 
         * @return collections::collection 
         */
        collections::collection get_collection(std::string id, collections::collection_status* err) override;
        collections::collection get_collection(std::string id) {
            return get_collection(id, nullptr);
        }

        /**
         * @brief Get the record row from SQLite
         *      vector store by record ID. Error 
         *      pointer is given value upon failure.
         * 
         * @param id 
         * @param err 
         * @return documents::document 
         */
        documents::document get_record(std::string id, documents::document_status* err) override;
        documents::document get_record(std::string id) {
            return get_record(id, nullptr);
        }

        /**
         * @brief Removes a record row from SQLite
         *      vector store by record ID.
         * 
         * @param id 
         * @return vektor::response<documents::document_status> 
         */
        vektor::response<documents::document_status> remove_record(std::string id) override;
        
        /**
         * @brief Removes a collection row from SQLite
         *      vector store by collection ID.
         * 
         * @param collection_id 
         * @return vektor::response<documents::document_status> 
         */
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

        /**
         * @brief sqlite.h callback for sqlite3_exec(...) function.
         *        Extracts results row by row and add to a container.
         * 
         * @param data 
         * @param column_count 
         * @param values 
         * @param column_names 
         * @return int 
         */
        static int sqlite3_callback(
            void* data, int column_count, 
            char** values, char** column_names);

        /**
         * @brief Generates current time stamp
         * 
         * @return std::string 
         */
        std::string current_timestamp();


        /**
         * @brief Generates a UUID
         * 
         * @return std::string 
         */
        std::string uuid();
        

        /**
         * @brief Executes a simple SQLite command
         * @deprecated DO NOT USE
         * @param command 
         * @param file 
         * @return RC 
         */
        RC execute_sqlite_cmd(std::string command, fs::path file);
        RC execute_sqlite_cmd(std::string command);

        
        /**
         * @brief Initializes SQLite database, runs
         *      all create table commands and sets
         *      database configuration.
         * 
         * @param file 
         * @return RC 
         */
        RC init_database(fs::path file);
        RC init_database();

        /**
         * @brief Deletes SQLite database file
         * 
         * @param file 
         * @return RC 
         */
        RC destroy_database(fs::path file);
        RC destroy_database();
        
        /**
         * @brief Builds a simple SQLite command from 
         *      vector of command tokens
         * @deprecated DO NOT USE
         * @param args 
         * @return std::string 
         */
        std::string construct_sqlite_cmd(std::vector<std::string> args);

        /**
         * @brief Apply filesystem permissions to a directory or file
         * 
         * @param path 
         * @param perms 
         * @param replace 
         * @return RC 
         */
        RC apply_fs_perm(
            const fs::path path, const fs::perms perms, const bool replace);
        RC apply_fs_perm(
            const fs::path path, const fs::perms perms) {
                return apply_fs_perm(path, perms, false);
            };


    };

}