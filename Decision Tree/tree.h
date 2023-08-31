#ifndef TREE_H
#define TREE_H
#include "car.h"
#include <map>
#include <string>
#include <vector>

struct Node
{
    ATTRIBUTE to_check;
    bool is_leaf;
    std::string classification; // applicable iff is_leaf is true
    std::map<std::string, Node *> child;
};

bool attribute_validity(const std::string attr);
std::map<std::string, int> count_values(const std::vector<Car> &examples, const std::string &attr);
std::pair<std::string, int> plurality_value(const std::vector<Car> &examples);
std::vector<Car> filter_examples(const std::vector<Car> &examples, const std::string attr,
                                 const std::string attr_value);
Node *learn_decision_tree(const std::vector<Car> &examples, const std::vector<ATTRIBUTE> &attribute_names,
                          const std::vector<Car> parent_examples);

#endif