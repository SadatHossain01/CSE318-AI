#ifndef CAR_H
#define CAR_H

#include <cassert>
#include <map>
#include <string>

enum class ATTRIBUTE
{
    BUYING_PRICE,
    MAINTENANCE_PRICE,
    DOOR,
    PERSON,
    LUGGAGE_BOOT,
    SAFETY,
    CLASS
};

std::string buy_attr = "buying";
std::string maint_attr = "maint";
std::string door_attr = "doors";
std::string person_attr = "persons";
std::string lug_attr = "lug_boot";
std::string safety_attr = "safety";
std::string class_attr = "class";

struct Car
{
    std::string detail;
    std::map<std::string, std::string> attributes; // classfication will also be here

    Car(const std::string &str);
    Car() = default;
};

#endif