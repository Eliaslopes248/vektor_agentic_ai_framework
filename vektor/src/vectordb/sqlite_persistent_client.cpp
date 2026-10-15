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


#include "vectordb/sqlite_persistent_client.hpp"
#include "vectordb/sqlite_persistent_client_sql.hpp"
#include <fstream>
#include <string>
#include <iostream>

// Initialize client by providing file path of SQLite3 file
vektor::sqlite_persistent_client::sqlite_persistent_client(
    fs::path file) {

        try{
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

            // Apply minimum permissions for sqlite file
            RC rc;

            rc = apply_fs_perm(
                this->sqlite_file, sqlite_persistent_client::min_file_perms, false);

            if (rc != RC::OK){
                throw fs::filesystem_error(
                    "file have minimum permissions applied.",
                    this->sqlite_file, 
                    std::make_error_code(std::errc::io_error)
                );
            }

        }catch (const fs::filesystem_error& e){
            std::cerr << "Unable to open backend file: " << e.what() << std::endl; 
            return;
        }
        catch(...){
            std::cerr << 
            "Unexpected error when initializing sqlite_persistent_client" 
            << std::endl;
            return;
        }
    }


// Create a collection namespace inside vector database
collection::Collection vektor::sqlite_persistent_client::create_collection(
            const std::string name, 
            const unsigned int dimension, 
            const collection::metric metric) {

            try {

                if (name.empty()){
                    throw std::invalid_argument("Collection name cannot be empty.");
                }

                std::vector<std::string> args = {
                    SQLITE3_EXE,

                };
                
                const std::string sql_cmd = construct_sqlite_cmd(args);
                
            }catch(const std::invalid_argument& e){
                std::cerr << e.what() << std::endl;
            }
            catch(...){ 
                std::cerr << 
                    "Unexpected error occurred when trying to create collection" 
                        << std::endl;
            }
            collection::Collection c;
            c.id = "123";
            c.name = "col1";
            return c;
    }

// TODO: Implement this method below
RC vektor::sqlite_persistent_client::insert(
            std::string id, 
            std::string collection_id, 
            std::string content, 
            std::map<std::string, std::string> metadata) {
                return RC::OK;
            }


// TODO: Implement this method below
RC vektor::sqlite_persistent_client::upsert(
    std::string id, 
    std::string collection_id, 
    std::string content, 
    std::map<std::string, std::string> metadata) {
        return RC::OK;
    }

// TODO: Implement this method below
collection::Collection vektor::sqlite_persistent_client::get(std::string id) {
    collection::Collection c;
    return c;
}

// TODO: Implement this method below
RC vektor::sqlite_persistent_client::remove_record(std::string id) {
    return RC::OK;
}

// TODO: Implement this method below
RC vektor::sqlite_persistent_client::remove_collection(std::string collection_id) {
    return RC::OK;
}

RC vektor::sqlite_persistent_client::execute_sqlite_cmd(
    std::string command) {
        return this->execute_sqlite_cmd(command, this->sqlite_file);
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
                throw sqlite_cmd_error("SQLite command returned non-zero return code.");
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

RC vektor::sqlite_persistent_client::init_database(fs::path file) {
    try {

        RC rc;

        std::string tables[] = {
            COLLECTION_CREATE_TABLE,
            DOCUMENT_CREATE_TABLE
        };

        int size = sizeof(tables) / sizeof(std::string);

        for (int i=0; i<size; i++) {
            rc = execute_sqlite_cmd(tables[i], file);
        }
        

    }catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return RC::DB_ERROR;
    }
    catch(...) {
        std::cerr << "Unable to initialize SQLite3 database" << std::endl;
        return RC::DB_ERROR;
    }
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