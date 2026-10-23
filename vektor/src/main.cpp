#include <iostream>
#include "vectordb/sqlite_persistent_client.hpp"
#include "vectordb/vectordb.hpp"
int main(){

    std::cout << "\n\n---------------------- Running ---------------------- " << std::endl;

    vektor::sqlite_persistent_client c;

    collections::collection _collection = c.get_collection_by_id(
            "2a1e2770-ab77-4bac-b918-fcd1cc550515"
        );

    std::cout << "ID: " << _collection.id << std::endl;
    std::cout << "NAME: " << _collection.name << std::endl;
    
    std::cout << "--------------------------------------------------- " << std::endl;

    documents::document doc;
    doc.id      = "137372373dvdvehehc";
    doc.content = "i love noire";

    c.upsert(doc);

    std::cout << "---------------------- Done. ---------------------- " << std::endl;
   
    return 0;
}