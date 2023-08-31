#include "car.h"
#include <sstream>
#include <vector>

std::string buy_attr = "buying";
std::string maint_attr = "maint";
std::string door_attr = "doors";
std::string person_attr = "persons";
std::string lug_attr = "lug_boot";
std::string safety_attr = "safety";
std::string class_attr = "class";

std::vector<std::string> buy_attr_values = {"vhigh", "high", "med", "low"};
std::vector<std::string> maint_attr_values = {"vhigh", "high", "med", "low"};
std::vector<std::string> door_attr_values = {"2", "3", "4", "5more"};
std::vector<std::string> person_attr_values = {"2", "4", "more"};
std::vector<std::string> lug_attr_values = {"small", "med", "big"};
std::vector<std::string> safety_attr_values = {"low", "med", "high"};

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