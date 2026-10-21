#include <iostream>
#include "vectordb/sqlite_persistent_client.hpp"
#include "vectordb/vectordb.hpp"
int main(){

    std::cout << "\n\n---------------------- Running ---------------------- " << std::endl;

    vektor::sqlite_persistent_client c;

    collections::collection _collection = c.get(
            "2a1e2770-ab77-4bac-b918-fcd1cc550515");

    std::cout << "ID: " << _collection.id << std::endl;
    std::cout << "NAME: " << _collection.name << std::endl;
    
    std::cout << "--------------------------------------------------- " << std::endl;

    c.insert(
        "137372373dvdvehehc",
        "2a1e2770-ab77-4bac-b918-fcd1cc550515",
        "this is doc chunk",
        {1.0f,1.5f,4.0f,5.4f,2.3f},
        {}
    );

    std::cout << "---------------------- Done. ---------------------- " << std::endl;
   
    return 0;
}