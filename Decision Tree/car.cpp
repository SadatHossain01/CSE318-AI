#include "car.h"
#include <sstream>
#include <vector>

Car::Car(const std::string &str)
{
    this->detail = str;
    std::stringstream ss(str);
    std::vector<std::string> substrings;
    while (ss.good())
    {
        std::string substr;
        getline(ss, substr, ',');
        substrings.push_back(substr);
    }

    assert(substrings.size() == 7);
    attributes[buy_attr] = substrings[0];
    attributes[maint_attr] = substrings[1];
    attributes[door_attr] = substrings[2];
    attributes[person_attr] = substrings[3];
    attributes[lug_attr] = substrings[4];
    attributes[safety_attr] = substrings[5];
    attributes[class_attr] = substrings[6];
}