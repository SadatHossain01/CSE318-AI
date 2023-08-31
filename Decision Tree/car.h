#ifndef CAR_H
#define CAR_H

#include <cassert>
#include <map>
#include <string>
#include <vector>

extern std::string buy_attr, maint_attr, door_attr, person_attr, lug_attr, safety_attr, class_attr;
extern std::vector<std::string> buy_attr_values, maint_attr_values, door_attr_values, person_attr_values,
    lug_attr_values, safety_attr_values;

struct Car
{
    std::string detail;
    std::map<std::string, std::string> attributes; // classification will also be here

    Car(const std::string &str);
    Car() = default;
};

#endif