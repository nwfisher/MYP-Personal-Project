#include <cstdlib>
#include <string>
#include "Part.h"

using namespace std;

Part::Part(int partNumber, std::string name, int quantity, std::string location)
    : partNumber(partNumber), quantity(quantity), name(name), location(location) {}

int Part::getPartNumber() const {
    return partNumber;
}

int Part::getQuantity() const {
    return quantity;
}

std::string Part::getName() const {
    return name;
}

std::string Part::getLocation() const {
    return location;
}