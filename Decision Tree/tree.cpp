#include "tree.h"
#include <cassert>
#include <math.h>

// assert that the attr string is one of the specified attribute names
bool attribute_validity(const std::string attr)
{
    return (attr == buy_attr || attr == maint_attr || attr == door_attr || attr == person_attr || attr == lug_attr ||
            attr == safety_attr || attr == class_attr);
}

// Given a set of examples and a particular attribute (possibly the classificaion as well), this
// will return a map mapping each value of that attribue to the number of occurences they are found.
std::map<std::string, int> count_values(const std::vector<Car> &examples, const std::string &attr)
{
    std::map<std::string, int> map;
    for (auto example : examples)
        map[example.attributes[attr]]++;

    return map;
}

// This function will return the plurality value of an example set (along
// with the number of occurences for that value). A plurality value is the
// maximally occuring classificaion for a certain set of examples.
std::pair<std::string, int> plurality_value(const std::vector<Car> &examples)
{
    std::map<std::string, int> map = count_values(examples, class_attr);
    std::pair<std::string, int> ret = {"", -1};
    for (auto they : map)
        if (they.second > ret.second)
            ret = they;
    return ret;
}

// Returns a filtered vector of only those examples who have attr_value for a certain attribute attr
std::vector<Car> filter_examples(const std::vector<Car> &examples, const std::string attr, const std::string attr_value)
{
    std::vector<Car> ret;
    for (auto example : examples)
        if (example.attributes[attr] == attr_value)
            ret.push_back(example);
    return ret;
}

double importance(const std::string &attribute, const std::vector<Car> &examples)
{
    const int example_size = examples.size();

    std::map<std::string, int> map = count_values(examples, attribute);
    double gain = 0.0;
    for (auto they : map)
    {
        if (they.second == 0)
            continue;
        double p = (double)they.second / example_size;
        std::vector<Car> filtered_examples = filter_examples(examples, attribute, they.first);
    }
}

Node *learn_decision_tree(const std::vector<Car> &examples, const std::vector<ATTRIBUTE> &attributes,
                          const std::vector<Car> parent_examples)
{
    Node n;
    if (examples.empty())
    {
        n.is_leaf = true;
        n.classification = plurality_value(parent_examples).first;
        return &n;
    }
    std::pair<std::string, int> pv = plurality_value(examples);
    if (pv.second == examples.size() || attributes.empty())
    {
        // all examples have the same classification
        // or no attributes left to check
        n.is_leaf = true;
        n.classification = pv.first;
        return &n;
    }
}