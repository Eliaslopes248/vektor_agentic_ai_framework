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

namespace vektor{

    // Base class for all persistent vector databases
    // This class should not have direct instances
    class persistent_client {
        public:     

        virtual ~persistent_client() = default;

        // Creates a collection inside vector database
        // so doc/embeddings can be upserted into collection namespaces
        virtual collection::Collection create_collection(
            const std::string name, const unsigned int dimension, const collection::metric metric) = 0;
        virtual collection::Collection create_collection(
            const std::string name, const unsigned int dimension) = 0;
        virtual collection::Collection create_collection(
            const std::string name) = 0;

        // Adding records to vector database
        // insert() creates a new record and upsert() updates a existing record
        virtual RC insert(
            std::string id, 
            std::string collection_id, 
            std::string content, 
            std::map<std::string, std::string> metadata) = 0;
        virtual RC upsert(
            std::string id, 
            std::string collection_id, 
            std::string content, 
            std::map<std::string, std::string> metadata) = 0;

        // Access and remove existing collections
        virtual collection::Collection get(std::string id) = 0;
        virtual RC remove_record(std::string id) = 0;
        virtual RC remove_collection(std::string collection_id) = 0;

    };

}