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

#include <iostream>
#include <map>

#include "collections.hpp"
#include "utils/rc.hpp"
#include "documents.hpp"

namespace vektor{

    /**
     * @brief Standardized struct for operation
     *       responses. This struct by default expects
     *       a payload of type T
     * 
     * @tparam T 
     */
    template <typename T = void>
    struct response {

        RC rc;                                  // Return code
        std::optional<T> data;                  // Optional field for returned data
        std::optional<std::string> message;     // Optional field for messages

        bool ok() {
            return rc == RC::OK;
        }

        // Accessor helpers
        T& get_data() {             
            return data.value();
        }

        T& get_msg() {             
            return message.value();
        }

        const T& get_data() const { 
            return data.value();
        }

        const T& get_msg() const { 
            return message.value();
        }
    };

    /**
     * @brief Standardized struct for operation
     *        responses. This struct by default expects
     *        no payload.
     * 
     * @tparam T 
     */
    template <>
    struct response<void> {
        RC rc;
        std::optional<std::string> message;

        bool ok() const {
            return rc == RC::OK;
        }

        std::string& get_msg() {
            return message.value();
        }

        const std::string& get_msg() const {
            return message.value();
        }
    };

    // Base class for all persistent vector databases
    // This class should not have direct instances
    class persistent_client {
        public:     

        virtual ~persistent_client() = default;

        /**
         * @brief Create a collection in SQLite vector store,
         *        and returns a Collection struct payload
         * @param id 
         * @param name 
         * @param dimension 
         * @param metric 
         * @return vektor::response<collections::collection> 
         */
        virtual vektor::response<collections::collection> create_collection(
            const std::string id, const std::string name, 
            const unsigned int dimension, const collections::metric metric) = 0;

        virtual vektor::response<collections::collection> create_collection(
            const std::string name, const unsigned int dimension, const collections::metric metric) = 0;

        virtual vektor::response<collections::collection> create_collection(
            const std::string name, const unsigned int dimension) = 0;

        virtual vektor::response<collections::collection> create_collection(
            const std::string name) = 0;

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
        virtual vektor::response<documents::document_status> insert(
            std::string id, 
            std::string collection_id, 
            std::string content, 
            std::vector<float> embeddings,
            documents::json metadata) = 0;

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
        virtual vektor::response<documents::document_status> upsert(
            std::string id, 
            std::string collection_id, 
            std::string content, 
            std::vector<float> embeddings,
            documents::json metadata) = 0;


        /**
         * @brief Get the collection object from SQLite
         *        vector store. Error pointer is given
         *        value upon failure.
         * @param id 
         * @param err 
         * @return collections::collection 
         */
        virtual collections::collection get_collection(
                std::string id, collections::collection_status* err) = 0;


        /**
         * @brief Get the record row from SQLite
         *      vector store by record ID. Error 
         *      pointer is given value upon failure.
         * 
         * @param id 
         * @param err 
         * @return documents::document 
         */
        virtual documents::document get_record(
                std::string id, documents::document_status* err) = 0;
        
        /**
         * @brief Removes a record row from SQLite
         *      vector store by record ID.
         * 
         * @param id 
         * @return vektor::response<documents::document_status> 
         */
        virtual vektor::response<documents::document_status> remove_record(std::string id) = 0;
        
        /**
         * @brief Removes a collection row from SQLite
         *      vector store by collection ID.
         * 
         * @param collection_id 
         * @return vektor::response<documents::document_status> 
         */
        virtual vektor::response<documents::document_status> remove_collection(std::string collection_id) = 0;

    };

}