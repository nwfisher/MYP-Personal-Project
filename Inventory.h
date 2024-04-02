#ifndef INVENTORY_H
#define INVENTORY_H

#include "Part.h"
#include <sqlite3.h>


using namespace std;

class Inventory {
private:
    vector<Part> parts;
    sqlite3 *db;

    void init();
    void createPartsTable();
    bool partExists(const char* name) const;

public:
    Inventory();
    ~Inventory();

    void rm(int id);
    void search(char* name);
    void updateQuantity(int id, int difference);
    void addPartToDatabase(const Part &newPart);
    void viewDatabase() const;
};

#endif
