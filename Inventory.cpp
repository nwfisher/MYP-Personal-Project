#include <vector>
#include "Part.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include "Inventory.h"

Inventory::Inventory() {
    init();
}

// Destructor
Inventory::~Inventory() {
    sqlite3_close(db);
}

void Inventory::init()
{
    ifstream file("inventory.db");
    bool fileExists = file.good();
    file.close();

    if (!fileExists)
    {
        cout << "Creating database file...\n";
        ofstream createFile("inventory.db");
        createFile.close();
    }

    int result = sqlite3_open("inventory.db", &db);

    if (result != SQLITE_OK) {
        throw runtime_error("FATAL: Could not open sqlite3 database 'inventory.db' despite checking for its existence.");
    }

    createPartsTable();
}

bool Inventory::partExists(const char* name) const {
    /* 
        Check if there is at least one row in the 'parts' table where the
        'name' column matches the specified value. 
    */
    const char* selectSQL = "SELECT 1 FROM parts WHERE name = ? LIMIT 1;";
    sqlite3_stmt *selectStatement;

    int result = sqlite3_prepare_v2(db, selectSQL, -1, &selectStatement, nullptr);

    if (result != SQLITE_OK) {
        throw runtime_error("FATAL: Could not prepare SELECT SQL statement!");
    }

    sqlite3_bind_text(selectStatement, 1, name, -1, SQLITE_STATIC);

    result = sqlite3_step(selectStatement);

    sqlite3_finalize(selectStatement);

    return result == SQLITE_ROW;
}

void Inventory::createPartsTable()
{
    /*
        Create a table if a 'parts' table does not exist with the following
        columns.
    */
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS parts ("
                                 "part_number INTEGER PRIMARY KEY,"
                                 "name TEXT NOT NULL,"
                                 "quantity INTEGER NOT NULL,"
                                 "location TEXT NOT NULL);";

    char *errorMessage;
    int result = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        throw runtime_error("FATAL: Could not execute SQL statement 'CREATE TABLE IF NOT EXISTS parts ("
                            "part_number INTEGER PRIMARY KEY,"
                            "name TEXT NOT NULL,"
                            "quantity INTEGER NOT NULL,"
                            "location TEXT NOT NULL);'"
                            "while creating parts table. The following error is provided: " + string(errorMessage)
                            );
    }
}


void Inventory::rm(int id) {
    /* Delete the row where provided part number is found */
    const char *deleteSQL = "DELETE FROM parts WHERE part_number = ?;";

    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(db, deleteSQL, -1, &statement, nullptr);

    if (result != SQLITE_OK) {
        cout << "Error preparing DELETE statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(statement, 1, id);

    result = sqlite3_step(statement);

    if (result != SQLITE_DONE) {
        cout << "Error executing DELETE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(statement);
        return;
    }

    sqlite3_finalize(statement);

    cout << "Part with ID " << id << " deleted successfully." << endl;
}
void Inventory::updateQuantity(int id, int difference) {
    int result = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cout << "Error starting transaction: " << sqlite3_errmsg(db) << endl;
        return;
    }

    /* Update the quantity of a provided part number by a provided value */
    const char* updateSQL = "UPDATE parts SET quantity = quantity + ? WHERE part_number = ?;";
    sqlite3_stmt *updateStatement;

    result = sqlite3_prepare_v2(db, updateSQL, -1, &updateStatement, nullptr);

    if (result != SQLITE_OK) {
        cout << "Error preparing UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    sqlite3_bind_int(updateStatement, 1, difference);
    sqlite3_bind_int(updateStatement, 2, id);

    result = sqlite3_step(updateStatement);

    if (result != SQLITE_DONE) {
        cout << "Error executing UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr); 
    } else {
        cout << "Quantity updated successfully." << endl;
    }

    sqlite3_finalize(updateStatement);

    result = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        throw runtime_error("\nFATAL: Error commiting transaction while updating item Quantity. The following error message is given: " + string(sqlite3_errmsg(db)));
    }
}

void Inventory::search(char* name) {

    if (!partExists(name)) {
        std::cout << "Part with name '" << name << "' does not exist.\n";
        return;
    }

    /*
        Select all items from the parts table that contain a provided
        value in their name column
    */
    const char* selectSQL = "SELECT * FROM parts WHERE name LIKE ?;";
    sqlite3_stmt *selectStatement;

    int result = sqlite3_prepare_v2(db, selectSQL, -1, &selectStatement, nullptr);

    if (result != SQLITE_OK) {
        cout << "Error preparing SELECT statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(selectStatement, 1, name, -1, SQLITE_STATIC);

    while ((result = sqlite3_step(selectStatement)) == SQLITE_ROW) {
        int partNumber = sqlite3_column_int(selectStatement, 0);
        const char *partName = reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, 1));
        int quantity = sqlite3_column_int(selectStatement, 2);
        const char *location = reinterpret_cast<const char *>(sqlite3_column_text(selectStatement, 3));

        cout << "\n-------------------\n";
        cout << "Part Number: " << partNumber << "\n";
        cout << "Name: " << partName << "\n";
        cout << "Quantity: " << quantity << "\n";
        cout << "Location: " << location << "\n";
        cout << "-------------------\n";
    }

    sqlite3_finalize(selectStatement);
}

/*
    Function that will add a part to the database. Will replace an existing row if
    part already exists
*/
void Inventory::addPartToDatabase(const Part &newPart)
{
    int result = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cout << "Error starting transaction: " << sqlite3_errmsg(db) << endl;
        return;
    }

    /* Select all items that have the provided part number */
    const char* selectSQL = "SELECT * FROM parts WHERE part_number = ?;";
    sqlite3_stmt *selectStatement;

    result = sqlite3_prepare_v2(db, selectSQL, -1, &selectStatement, nullptr);

    if (result != SQLITE_OK) {
        cout << "Error preparing SELECT statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr); // Rollback on error
        return;
    }

    sqlite3_bind_int(selectStatement, 1, newPart.getPartNumber());

    result = sqlite3_step(selectStatement);

    /* If the part is already in the database */
    if (result == SQLITE_ROW) {
        const char* updateSQL = "UPDATE parts SET name = ?, quantity = ?, location = ? WHERE part_number = ?;";
        sqlite3_stmt *updateStatement;

        result = sqlite3_prepare_v2(db, updateSQL, -1, &updateStatement, nullptr);

        if (result != SQLITE_OK) {
            cout << "Error preparing UPDATE statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            sqlite3_finalize(updateStatement);
            return;
        }

        sqlite3_bind_text(updateStatement, 1, newPart.getName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(updateStatement, 2, newPart.getQuantity());
        sqlite3_bind_text(updateStatement, 3, newPart.getLocation().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(updateStatement, 4, newPart.getPartNumber());

        result = sqlite3_step(updateStatement);

        if (result != SQLITE_DONE) {
            cout << "Error executing UPDATE statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            sqlite3_finalize(updateStatement);
        } else {
            cout << "Part updated successfully." << endl;
        }

        sqlite3_finalize(updateStatement);
    } else if (result == SQLITE_DONE) {
        
        /* Insert a new row into parts table with following values */
        const char* insertSQL = "INSERT INTO parts (part_number, name, quantity, location) VALUES (?, ?, ?, ?);";
        sqlite3_stmt *insertStatement;

        result = sqlite3_prepare_v2(db, insertSQL, -1, &insertStatement, nullptr);

        if (result != SQLITE_OK) {
            cout << "Error preparing INSERT statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return;
        }

        sqlite3_bind_int(insertStatement, 1, newPart.getPartNumber());
        sqlite3_bind_text(insertStatement, 2, newPart.getName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(insertStatement, 3, newPart.getQuantity());
        sqlite3_bind_text(insertStatement, 4, newPart.getLocation().c_str(), -1, SQLITE_STATIC);

        result = sqlite3_step(insertStatement);

        if (result != SQLITE_DONE) {
            cout << "Error executing INSERT statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        } else {
            cout << "Part added successfully." << endl;
        }

        sqlite3_finalize(insertStatement);
    } else {
        cout << "Error executing SELECT statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }

    // Commit the transaction
    result = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        cout << "Error committing transaction: " << sqlite3_errmsg(db) << endl;
    }

    // Finalize the SELECT statement to release resources
    sqlite3_finalize(selectStatement);
    cout << "Part added successfully." << endl;
}

void Inventory::viewDatabase() const
{
    cout << "\nDatabase Contents:\n";
    cout << "-------------------\n";

    const char *selectSQL = "SELECT * FROM parts;";
    sqlite3_stmt *statement;

    int result = sqlite3_prepare_v2(db, selectSQL, -1, &statement, nullptr);

    if (result != SQLITE_OK)
    {
       throw runtime_error("Error preparing SELECT statement: " + string(sqlite3_errmsg(db)));
    }

    while ((result = sqlite3_step(statement)) == SQLITE_ROW)
    {
        int partNumber = sqlite3_column_int(statement, 0);
        const char *name = reinterpret_cast<const char *>(sqlite3_column_text(statement, 1));
        int quantity = sqlite3_column_int(statement, 2);
        const char *location = reinterpret_cast<const char *>(sqlite3_column_text(statement, 3));

        cout << "Part Number: " << partNumber << "\n";
        cout << "Name: " << name << "\n";
        cout << "Quantity: " << quantity << "\n";
        cout << "Location: " << location << "\n";
        cout << "-------------------\n";
    }

    // Finalize the statement to release resources
    sqlite3_finalize(statement);
}
