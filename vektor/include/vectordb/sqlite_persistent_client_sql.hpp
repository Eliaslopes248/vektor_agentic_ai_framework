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


// This header file will define macros of strings
// defining SQLite3 commands as shortcuts

/***
 * 
 * MACRO Conventions:
 * 
 * TABLE CREATE:
 *     XXX = Table Name
 *     #define XXX_CREATE_TABLE R"(
 *          CREATE TABLE IF NOT EXISTS XXX (
 *              column type constraints
 *              ....
 *          )
 *      )""
 * 
 * 
 */


/**
 * ======================
 * SQLITE3 CONFIG MACROS
 * ======================
 */

 #define ENABLE_FK R"(PRAGMA foreign_keys = ON;)"

/**
 * ======================
 * CREATE TABLE MACROS
 * ======================
 */

#define COLLECTION_CREATE_TABLE R"(
    CREATE TABLE IF NOT EXISTS collections (
    id          TEXT PRIMARY KEY,
    name        TEXT NOT NULL UNIQUE,
    dimension   INTEGER NOT NULL CHECK (dimension > 0),
    metric      TEXT NOT NULL DEFAULT 'cosine'
                CHECK (metric IN ('cosine', 'l2', 'inner_product'))
    );
)"


#define DOCUMENT_CREATE_TABLE R"(
    CREATE TABLE IF NOT EXISTS documents (
        id            TEXT PRIMARY KEY,
        collection_id TEXT NOT NULL,
        content       TEXT,
        embedding     BLOB NOT NULL,
        metadata      TEXT NOT NULL DEFAULT '{}',
        created_at    TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at    TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY (collection_id)
            REFERENCES collections(id)
            ON DELETE CASCADE
    );
)"

/**
 * ======================
 * CREATE TABLE INDEX MACROS
 * ======================
 */

#define DOCUMENT_CREATE_INDEX R"(
    CREATE INDEX IF NOT EXISTS idx_documents_collection
        ON documents(collection_id);
)"

/**
 * ====================
 * INIT ALL SCHEMAS
 * ====================
 */
#define SQLITE3_INIT_ALL_SCHEMAS R"(
    PRAGMA foreign_keys = ON;

    CREATE TABLE IF NOT EXISTS collections (
        id          TEXT PRIMARY KEY,
        name        TEXT NOT NULL UNIQUE,
        dimension   INTEGER NOT NULL CHECK (dimension > 0),
        metric      TEXT NOT NULL DEFAULT 'cosine'
                    CHECK (metric IN ('cosine', 'l2', 'inner_product'))
    );

    CREATE TABLE IF NOT EXISTS documents (
        id            TEXT PRIMARY KEY,
        collection_id TEXT NOT NULL,
        content       TEXT,
        embedding     BLOB NOT NULL,
        metadata      TEXT NOT NULL DEFAULT '{}',
        created_at    TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
        updated_at    TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY (collection_id)
            REFERENCES collections(id)
            ON DELETE CASCADE
    );

    CREATE INDEX IF NOT EXISTS idx_documents_collection
        ON documents(collection_id);
)"
