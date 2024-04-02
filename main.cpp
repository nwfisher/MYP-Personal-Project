#include <cstdlib>
#include "Inventory.h"
#include <string>
#include <iostream>
#include "Part.h"

using namespace std;

int main() {

    Inventory* inv = new Inventory();
    
    while (true) {
        int cmd;

        string location;
        char* name = nullptr;
        int quantity;
        int id;
        Part* part = nullptr;

        cout << "\n\nEnter a option:\n\n";
        cout << "1) Add an item\n";
        cout << "2) View database\n";
        cout << "3) Search by name\n";
        cout << "4) Remove an item\n";
        cout << "5) Update Quantity of an item\n";
        cout << "6) Quit\n";

        cout << "Option: ";
        cin >> cmd;
        

        switch (cmd) {
            case 1:
                cout << "Enter a part ID: ";
                cin >> id;
                cout << "Enter a part name: ";
                name = new char[100];
                cin.ignore();
                cin.getline(name, 100); //Use getline bc cin ends at whitespace
                cout << "Enter a part quantity: ";
                cin >> quantity;
                cout << "Enter a part location: ";
                cin.ignore();
                getline(cin, location);
                part = new Part(id, name, quantity, location);
                inv->addPartToDatabase(*part);
                break;
            case 2:
                inv->viewDatabase();
                break;
            case 3:
                cout << "Enter item name: ";
                name = new char[100];
                cin.ignore();
                cin.getline(name, 100);
                inv->search(name);
                break;
            case 4:
                cout << "Enter a part ID to delete: ";
                cin >> id;
                inv->rm(id);
                break;
            case 5:
                cout << "Enter a part ID: ";
                cin >> id;
                cout << "Enter the difference in quantity (can be +/-): ";
                cin >> quantity;

                inv->updateQuantity(id, quantity);
                break;
            case 6:
                delete part;
                delete[] name;
                delete inv; 
                exit(0);
                break;
            default:
                cout << "Invalid option. Please enter a valid option.\n";
                continue;
                break;
        }

        cout << "Press a key to continue...\n";
        string ready;
        cin.ignore(); 
        getline(cin, ready);
        delete[] name;
        delete part;
    }


};