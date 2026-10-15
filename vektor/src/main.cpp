#include <iostream>
#include "vectordb/sqlite_persistent_client.hpp"
#include "vectordb/vectordb.hpp"
int main(){

    std::cout << "Running" << std::endl;
    fs::path p = DEFAULT_DATA_STORE_DIR;

    vektor::sqlite_persistent_client c(p);
    std::string cmd = "CREATE TABLE IF NOT EXISTS collections (id TEXT PRIMARY KEY, name TEXT NOT NULL UNIQUE, dimension INTEGER NOT NULL, metric TEXT NOT NULL DEFAULT 'cosine');";
    c.execute_sqlite_cmd(cmd);
    cmd = "INSERT INTO collections (id, name, dimension, metric) VALUES ('col_001', 'documents', 384, 'cosine');";
    c.execute_sqlite_cmd(cmd);
    
    std::cout << "Done." << std::endl;
   
    return 0;
}