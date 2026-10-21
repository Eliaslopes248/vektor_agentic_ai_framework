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


#include <fstream>
#include <string>
#include <iostream>
#include <sqlite3.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>
#include <boost/uuid.hpp>

#include "vectordb/sqlite_persistent_client.hpp"
#include "vectordb/sqlite_persistent_client_sql.hpp"


// Clean up destructor
vektor::sqlite_persistent_client::~sqlite_persistent_client() {
    if (this->__database) {
        sqlite3_close(this->__database);
    }
}

// Initialize client by providing file path of SQLite3 file
vektor::sqlite_persistent_client::sqlite_persistent_client(
    fs::path file) : __database(nullptr) {

        // Resolve absolute path
        this->sqlite_file = fs::absolute(file);

        // Ensure SQLite3 file exists and is able to
        // be opened for reading and writing
        if (!fs::exists(this->sqlite_file)){

            // create parent directories
            fs::path parent_path = this->sqlite_file.parent_path();
            if (!fs::exists(parent_path)){

                // Create parent directories and sub directories
                bool created = fs::create_directories(parent_path);

                // Catch failure
                if (!created){
                    throw fs::filesystem_error(
                        "Parent path could not be created couldnt.",
                        parent_path, 
                        std::make_error_code(std::errc::io_error)
                    );
                }
            }

            // Open sqlite file
            std::ofstream file(this->sqlite_file, std::ios::binary);

            if (!file.is_open()){
                throw fs::filesystem_error(
                    "file couldnt be open for input and output.",
                    this->sqlite_file, 
                    std::make_error_code(std::errc::io_error)
                );
            }
        }

        RC rc;

        // Apply minimum permissions for sqlite file
        rc = apply_fs_perm(
            this->sqlite_file, sqlite_persistent_client::min_file_perms, false);

        if (rc != RC::OK) {
            throw fs::filesystem_error(
                "file have minimum permissions applied.",
                this->sqlite_file, 
                std::make_error_code(std::errc::io_error)
            );
        }


        // Initialize sqlite3 database connection
        int result = sqlite3_open(
                this->sqlite_file.c_str(), &this->__database);

        if (result != SQLITE_OK) {
            throw sqlite3::sqlite_database_error (
                "Unable to establish connection to SQLite3 database."
            );
        }

        // Initialize tables in database
        rc = init_database(this->sqlite_file);
        if (rc != RC::OK) {
            throw sqlite3::sqlite_database_error(
                "Unable to initialize SQLite database"
            );
        }

    }


// Create a collection namespace inside vector database
vektor::response<collections::collection>
    vektor::sqlite_persistent_client::create_collection(
            const std::string id, 
            const std::string name, 
            const unsigned int dimension, 
            const collections::metric metric) {

            // Collection struct
            vektor::response<collections::collection> response;

            try {
                // Catch invalid collection name
                if (id.empty()) {
                    throw std::invalid_argument("Collection id cannot be empty.");
                }
                if (name.empty()) {
                    throw std::invalid_argument("Collection name cannot be empty.");
                }

                // convert strings to char*
                std::string metric_str  = collections::metric_to_string(metric);
                const char* _metric     = metric_str.c_str();
                const char* _id         = id.c_str();
                const char* _name       = name.c_str();

                // prepare sql command
                const std::string sql = R"(
                    INSERT INTO collections (id, name, dimension, metric)
                    VALUES (?, ?, ?, ?);
                )";

                sqlite3_stmt* stmt = nullptr;

                // Create the SQL statement
                int rc = sqlite3_prepare_v3(
                    this->__database,
                    sql.c_str(),
                    -1,
                    0,
                    &stmt,
                    nullptr
                );

                // Bind values to statement
                if (rc != SQLITE_OK) {
                    // free SQL statement pointer resources
                    sqlite3_finalize(stmt);
                    throw vektor::sqlite3::sqlite_cmd_error(
                        "Unable to prepare SQL statement " + sql
                    );
                }

                sqlite3_bind_text(stmt, 1, _id, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, _name, -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt,  3, dimension);
                sqlite3_bind_text(stmt, 4, _metric, -1, SQLITE_TRANSIENT);
                
                rc = sqlite3_step(stmt);

                if (rc != SQLITE_DONE) {
                    std::string msg = sqlite3_errmsg(this->__database);
                    sqlite3_finalize(stmt);
                    throw vektor::sqlite3::sqlite_cmd_error(
                        "SQL command failed" + sql + "\n" + msg
                    );
                }

                // construct return payload
                collections::collection _collection;
                _collection.id          = id;
                _collection.name        = name;
                _collection.dimension   = dimension;
                _collection.metric      = metric;
                
                response.data = _collection;

                // free SQL statement pointer resources
                sqlite3_finalize(stmt);
                return response;
            }catch(const vektor::sqlite3::sqlite_cmd_error& e) {
                response.rc      = RC::DB_ERROR;
                response.message = e.what();
                std::cerr << e.what() << std::endl;
                return response;
            }
            catch(const std::invalid_argument& e) {
                response.rc      = RC::BAD_PARAM;
                response.message = e.what();
                std::cerr << e.what() << std::endl;
                return response;
            }
            catch(...) { 
                response.rc      = RC::ERROR;
                std::string err = "Unexpected error occurred when trying to create collection";
                response.message = err;
                std::cerr << err << std::endl;
                return response;
            }
           
    }

// Insert a new document into documents table
vektor::response<documents::document_status>
         vektor::sqlite_persistent_client::insert(
            std::string         id, 
            std::string         collection_id, 
            std::string         content, 
            std::vector<float>  embeddings,
            documents::json     metadata) {

            vektor::response<documents::document_status> _response;
            try {

                // validate method parameters
                if (id.empty()) {
                    throw std::invalid_argument(
                        "Insert failed: \'id\' cannot be empty string."
                    );
                }
                if (collection_id.empty()) {
                    throw std::invalid_argument(
                        "Insert failed: \'collection_id\' cannot be empty string."
                    );
                }
                if (content.empty()) {
                    throw std::invalid_argument(
                        "Insert failed: \'content\' cannot be empty string."
                    );
                }

                // Validate dimension size of embeddings
                // fetching the collection will also validate the FK exists
                // err is expected to be NULL if successful query
                collections::collection_status* err = nullptr;
                collections::collection _collection = this->get_collection(collection_id, err);

                if (err) {
                    throw vektor::sqlite3::sqlite_query_error(
                        "Collection passed doesnt exist. \'id\' " +
                        collection_id
                    );
                }

                if (_collection.dimension != embeddings.size()) {
                    throw std::invalid_argument(
                        "Vector dimension size doesnt match collection dimension"
                    );
                }

                // prepare SQL statement
                // bind values to statement
                std::string sql = R"(
                    INSERT INTO
                    documents 
                    (
                        id, collection_id, content, 
                        embedding, metadata, 
                        created_at, updated_at
                    )
                    VALUES (?, ?, ?, ?, ?, ?, ?);
                )";
            
                sqlite3_stmt* stmt;

                int rc = sqlite3_prepare_v3(
                    this->__database, sql.c_str(), -1, 0, &stmt, nullptr);
                
                if (rc != SQLITE_OK) {
                    sqlite3_finalize(stmt);
                    throw vektor::sqlite3::sqlite_cmd_error(
                        "Unable to prepare SQL command " + sql
                    );
                }

                // Get current timestamp
                std::string current_timestamp = this->current_timestamp();

                // extract metadata
                std::string metadata_str = metadata.dump();
                if (metadata_str.empty()) {
                    metadata_str = "{}";
                }

                // Bind values to SQL parameters
                const char* _id            = id.c_str();
                const char* _collection_id = collection_id.c_str();
                const char* _content       = content.c_str();
                const char* _metadata      = metadata_str.c_str();
                const char* _timestamp     = current_timestamp.c_str();

                sqlite3_bind_text(stmt, 1, _id, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, _collection_id, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, _content, -1, SQLITE_TRANSIENT);
                
                sqlite3_bind_blob(
                    stmt, 
                    4, 
                    embeddings.data(), 
                    static_cast<int>(embeddings.size() * sizeof(float)), 
                    SQLITE_TRANSIENT
                );

                sqlite3_bind_text(stmt, 5, _metadata, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 6, _timestamp, -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 7, _timestamp, -1, SQLITE_TRANSIENT);

                // execute SQL command 
                rc = sqlite3_step(stmt);

                if (rc != SQLITE_DONE) {
                    std::string error = sqlite3_errmsg(this->__database);
                    sqlite3_finalize(stmt);
                    throw std::runtime_error(error);
                }
                
                // free up statement resources
                sqlite3_finalize(stmt);

                _response.rc = RC::OK;
                _response.data = documents::document_status::created;

                return _response;
            }
            catch(const vektor::sqlite3::sqlite_cmd_error& e){
                std::cerr << e.what() <<std::endl;
                _response.rc = RC::QUERY_ERROR;
                _response.data = documents::document_status::operation_error;
                return _response;
            }
            catch(const std::runtime_error& e){
                std::cerr << e.what() <<std::endl;
                _response.rc = RC::RUNTIME_ERROR;
                _response.data = documents::document_status::operation_error;
                return _response;
            }
            catch(const std::invalid_argument& e){
                std::cerr << e.what() <<std::endl;
                _response.rc = RC::BAD_PARAM;
                _response.data = documents::document_status::operation_error;
                return _response;
            }
            catch(const std::exception& e){
                std::cerr << e.what() <<std::endl;
                _response.rc = RC::ERROR;
                _response.data = documents::document_status::operation_error;
                return _response;
            }
            
        }


// TODO: Implement this method below
vektor::response<documents::document_status> vektor::sqlite_persistent_client::upsert(
    std::string         id, 
    std::string         collection_id, 
    std::string         content, 
    std::vector<float>  embeddings,
    documents::json     metadata) {

        vektor::response<documents::document_status> response;
                
        try{
            // validate input arguments
            if (id.empty()) {
                throw std::invalid_argument(
                    "Unable to upsert document \'id\' cannot be empty string."
                );
            } 

            if (collection_id.empty()) {
                throw std::invalid_argument(
                    "Unable to upsert document \'collection_id\' cannot be empty string."
                );
            } 

            // ensure collection FK exists
            collections::collection_status* err;
            collections::collection _fk = this->get_collection(collection_id, err);

            if (err != nullptr) {
                throw std::invalid_argument(
                    "Unable to upsert document \'collection_id\' doesnt reference any collection."
                );
            }

            // ensure document already exists so we can update it

            // find out which fields were updated

            // prepare SQL command

            // bind values to SQL parameters

            // handle exceptions


        }
        catch(const vektor::sqlite3::sqlite_cmd_error& e) {
            response.data = documents::document_status::operation_error;
            std::cerr << e.what() << std::endl;
        }
        catch(const std::invalid_argument& e) {
            response.data = documents::document_status::operation_error;
            std::cerr << e.what() << std::endl;
        }
        catch(const std::exception& e) {
            response.data = documents::document_status::unexpected_error;
            std::cerr << e.what() << std::endl;
        }
    
    
    }


// Returns a document fetched by collection ID
// FIXME:
documents::document
    vektor::sqlite_persistent_client::get_record(
        std::string id, documents::document_status* err) {

    }

collections::collection 
    vektor::sqlite_persistent_client::get_collection(
        std::string id, collections::collection_status* err) {

    collections::collection _collection;
    try {

        if (id.empty()) {
            throw std::invalid_argument(
                "Unable to fetch collection, \'id\' cannot be empty string."
            );
        }

        std::string sql = R"(
            SELECT * 
            FROM collections
            WHERE id = ?
            LIMIT 1;
        )";

        // prepare SQL statement
        sqlite3_stmt* stmt;

        int rc = sqlite3_prepare_v3(this->__database, sql.c_str(), -1, 0, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "Unable to prepare SQL command " + sql + "\n"
            );
        }
        const char* _id = id.c_str();

        // bind SQL command parameters
        sqlite3_bind_text(stmt, 1, _id, -1, SQLITE_TRANSIENT);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "SQL command failed to bind" + sql
            );
        }

        // execute SQL command
        rc = sqlite3_step(stmt);

        // No ROWS returned
        if (rc == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return _collection;
        }

        // parse row data
        if (rc == SQLITE_ROW) {
            std::string _id    = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 0)
            );

            std::string _name   = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 1)
            );

            int _dimension      = sqlite3_column_int(stmt, 2);

            std::string _metric = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 3)
            );

            // load return payload
            _collection.id          = _id;
            _collection.name        = _name;
            _collection.dimension   = _dimension;
            _collection.metric      = collections::string_to_metric(_metric);

        }
        // free SQL statement pointer resources
        sqlite3_finalize(stmt);

        return _collection;

    }
    catch (const std::invalid_argument e){
        std::cerr << e.what() << std::endl;
        *err = collections::collection_status::unexpected_error;
        return _collection;
    }
    catch (const vektor::sqlite3::sqlite_cmd_error& e){
        std::cerr << e.what() << std::endl;
        *err = collections::collection_status::operation_error;
        return _collection;
    }
    catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        *err = collections::collection_status::unexpected_error;
        return _collection;
    }
    
    
}

// Deletes a document from SQLite database
vektor::response<documents::document_status> 
    vektor::sqlite_persistent_client::remove_record(std::string id) {
    
    vektor::response<documents::document_status> response;

    try {

        if (id.empty()) {
            throw std::invalid_argument(
                "Unable to remove document. \'id\' cannot be empty string."
            );
        }

        // prepare SQL command and bind values to parameters
        std::string sql = R"(
            DELETE FROM
            documents
            WHERE id = ?;
        )";

        sqlite3_stmt* stmt;

        int rc = sqlite3_prepare_v3(this->__database, sql.c_str(), -1, 0, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "Unable to prepare SQL command " + sql
            );
        }

        // bind collection id
        const char* _id = id.c_str();
        rc = sqlite3_bind_text(stmt, 1, _id, -1, SQLITE_TRANSIENT);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "Unable to bind \'id\' SQL command: " + id
            );
        }

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "SQL command failed: " + id
            );
        }

        sqlite3_finalize(stmt);

        response.rc = RC::OK;
        response.data = documents::document_status::deleted;

        return response;

    }
    catch(const vektor::sqlite3::sqlite_cmd_error& e) {
        response.data = documents::document_status::unexpected_error;
        response.rc = RC::BAD_PARAM;
        return response;
    }
    catch(const std::invalid_argument& e) {
        response.data = documents::document_status::unexpected_error;
        response.rc = RC::BAD_PARAM;
        return response;
    }
    catch(const std::exception& e) {
        response.data = documents::document_status::unexpected_error;
        response.rc = RC::ERROR;
        return response;
    }
    
}

// Delete a collection for SQLite database by collection id
vektor::response<documents::document_status> 
    vektor::sqlite_persistent_client::remove_collection(std::string collection_id) {
    
    vektor::response<documents::document_status> response;

    try {

        if (collection_id.empty()) {
            throw std::invalid_argument(
                "Unable to remove collection. \'collection_id\' cannot be empty string."
            );
        }

        // prepare SQL command and bind values to parameters
        std::string sql = R"(
            DELETE FROM
            collections
            WHERE id = ?;
        )";

        sqlite3_stmt* stmt;

        int rc = sqlite3_prepare_v3(this->__database, sql.c_str(), -1, 0, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "Unable to prepare SQL command " + sql
            );
        }

        // bind collection id
        const char* _id = collection_id.c_str();
        rc = sqlite3_bind_text(stmt, 1, _id, -1, SQLITE_TRANSIENT);

        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "Unable to bind \'collection_id\' SQL command: " + collection_id
            );
        }

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw vektor::sqlite3::sqlite_cmd_error(
                "SQL command failed: " + collection_id
            );
        }

        sqlite3_finalize(stmt);

        response.rc = RC::OK;
        response.data = documents::document_status::deleted;

        return response;

    }
    catch(const vektor::sqlite3::sqlite_cmd_error& e) {
        response.data = documents::document_status::unexpected_error;
        response.rc = RC::BAD_PARAM;
        return response;
    }
    catch(const std::invalid_argument& e) {
        response.data = documents::document_status::unexpected_error;
        response.rc = RC::BAD_PARAM;
        return response;
    }
    catch(const std::exception& e) {
        response.data = documents::document_status::unexpected_error;
        response.rc = RC::ERROR;
        return response;
    }
    
}


RC vektor::sqlite_persistent_client::execute_sqlite_cmd(std::string command) {
    // Check if database file exists
    if (!fs::exists(this->sqlite_file)){
        return RC::FILESYSTEM_ERROR;
    }
    return execute_sqlite_cmd(command, this->sqlite_file);
} 

RC vektor::sqlite_persistent_client::execute_sqlite_cmd(
    std::string command, fs::path file) {
        try {

            // Build sub process command for SQLite
            std::string sql_cmd = construct_sqlite_cmd({
                SQLITE3_EXE,
                file.string(),
                command
            });
            
            // Run SQLite command on host environment
            int code = std::system(sql_cmd.c_str());

            // Handle failed process
            if (code != 0){
                throw sqlite3::sqlite_cmd_error("SQLite command returned non-zero return code.");
            }

        }catch(const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            return RC::ERROR;
        
        }catch(...) {
            std::cerr << "Unexpected error when executing SQLite3 command." << std::endl;
            return RC::DB_ERROR;
        }

        return RC::OK;
    }

RC vektor::sqlite_persistent_client::init_database() {
    return init_database(this->sqlite_file);
}

// Initialize database schemas
RC vektor::sqlite_persistent_client::init_database(fs::path file) {
    try {

        // Execute SQL command that aggregates 
        // starting database schema
        char* error = nullptr;
        int success = sqlite3_exec(
            this->__database,
            SQLITE3_INIT_ALL_SCHEMAS,
            nullptr,
            nullptr,
            &error
        );

        if (success != SQLITE_OK) {
            // set error message
            std::string msg = error
                ? error
                : sqlite3_errmsg(this->__database);


            sqlite3_free(error);
            
            throw vektor::sqlite3::sqlite_cmd_error(
                    "SQLite3 database initialization failed: " + msg      
            );

        }
        
        return RC::OK;

    }catch(const vektor::sqlite3::sqlite_cmd_error& e) {
        std::cerr << e.what() << std::endl;
        return RC::DB_ERROR;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return RC::DB_ERROR;
    }
    catch(...) {
        std::cerr << "Unable to initialize SQLite3 database" << std::endl;
        return RC::DB_ERROR;
    }
}

RC vektor::sqlite_persistent_client::destroy_database(fs::path file) {
    try{

        // Check if database file exists
        if (!fs::exists(file)) {
            throw fs::filesystem_error(
                "Database file does not exist",
                file,
                std::make_error_code(std::errc::io_error)
            );
        }

        // Delete SQLite file
        std::error_code ec;
        bool success = fs::remove(file, ec);

        if (ec) {
            throw std::runtime_error(
                "Failed to remove file: " + ec.message()
            );
        }

        // Log only in debug mode
        if (!success) {
            #ifdef DEBUG
                std::cout <<
                     "File does not exist: " 
                            << file << std::endl;
            #endif
        }

        // close connection
        int rc = sqlite3_close(this->__database);

        if (rc != 0) {
            throw vektor::sqlite3::sqlite_database_error(
                "Unable to close database connection."
            );
        }

        return RC::OK;

    }catch(const vektor::sqlite3::sqlite_database_error& e){
        std::cerr << e.what() << std::endl;
        return RC::DB_ERROR;
    }
    catch(const fs::filesystem_error& e){
        std::cerr << e.what() << std::endl;
        return RC::FILESYSTEM_ERROR;
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return RC::ERROR;
    }
}


RC vektor::sqlite_persistent_client::destroy_database() {
    return destroy_database(this->sqlite_file);
}

// Construct SQLite command from vector of strings
std::string vektor::sqlite_persistent_client::construct_sqlite_cmd(
    std::vector<std::string> args){
        if (args.empty()){
            throw std::runtime_error("Error: Empty SQLite3 command list");
        }

        bool append_exe = false;
        if (args.at(0) != SQLITE3_EXE){
            append_exe = true;
        }

        std::string command = "";

        if (append_exe){
            command += SQLITE3_EXE;
        }

        // Append each sub command
        for (std::size_t i = 0; i < args.size(); ++i) {
            command += " '";

            // resolve quotes in command string
            for (char c : args[i]) {
                if (c == '\'') {
                    command += "'\\''";
                } else {
                    command += c;
                }
            }

            command += "'";
        }
        #ifdef DEBUG
            std::cout << "\n============ SQLITE COMMAND ============ " << std::endl;
            std::cout << "SQLITE COMMAND:" << command << std::endl;
            std::cout << "" << std::endl;
        #endif
        return command;
    }


// Applies filesystem permissions to either a directory or a file
// Used to fix content with lacking permissions.
RC vektor::sqlite_persistent_client::apply_fs_perm(const fs::path path, const fs::perms perms, const bool replace) {

    try {

        // Invalid path
        if (!fs::exists(path)){
            throw fs::filesystem_error(
                "Path given does not exist.",
                path,
                std::make_error_code(std::errc::io_error)
            );
        }

        std::error_code ec;

        // Overwrite existing permissions
        if (replace){
            fs::permissions(
                path,
                perms,
                fs::perm_options::replace,
                ec
            );
        }
        // Append to existing permissions 
        else{
            fs::permissions(
                path,
                perms,
                fs::perm_options::add,
                ec
            );
        } 

        // Handle permission command errors
        if (ec) {
            std::string error_msg = 
                "Unable to apply permissions to path" + path.string();
            throw std::runtime_error(error_msg);
        }
        return RC::OK;
    }catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return RC::ERROR;
    }
    
}

// SQL query callback for parsing results
int vektor::sqlite3::sqlite3_callback(
    void* data,
    int column_count,
    char** values,
    char** column_names
) {
    // holds all row results
    auto* results = static_cast<sqlite3::sqlite3_results*>(data);

    sqlite3::sqlite3_row row;

    // append column data to a row
    for (int i = 0; i < column_count; ++i) {
        row.push_back(values[i] ? values[i] : "NULL");
    }

    // append row to result list
    results->push_back(std::move(row));
    return 0;
}

std::string vektor::sqlite_persistent_client::uuid() {
    // generator instance
    boost::uuids::random_generator gen;

    // create uuid
    boost::uuids::uuid id = gen();

    // convert the UUID to a standard string
    std::string id_str = boost::uuids::to_string(id);

    return id_str;
}

std::string vektor::sqlite_persistent_client::current_timestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};

    #ifdef _WIN32
        gmtime_s(&tm, &time);
    #else
        gmtime_r(&time, &tm);
    #endif

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}