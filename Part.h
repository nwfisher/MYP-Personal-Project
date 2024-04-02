#ifndef PART_H
#define PART_H

#include <string>

class Part {
    private:
        int partNumber;
        int quantity;
        std::string name;
        std::string location;
    
    public:
        // Constructors
        Part();  // Default constructor
        Part(int partNumber, std::string name, int quantity, std::string location);

        // Getter methods
        int getPartNumber() const;
        int getQuantity() const;
        std::string getName() const;
        std::string getLocation() const;
};

#endif