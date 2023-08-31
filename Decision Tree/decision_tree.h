#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include "car.h"
#include <map>
#include <string>
#include <vector>

struct Node
{
    std::string attribute_to_check;
    bool is_leaf;
    std::string classification;          // applicable iff is_leaf is true
    std::map<std::string, Node *> child; // attribute value -> child node

    Node(bool is_leaf = false, std::string classification = "") : is_leaf(is_leaf), classification(classification)
    {
    }
    ~Node();
};

struct Tree
{
    Node *root;
    Tree(Node *root) : root(root)
    {
    }
    ~Tree()
    {
        delete root;
    }
    std::string get_classification(Car car);
    void print_tree(Node *node, int depth = 0);
};

std::vector<std::string> get_attribute_values(const std::string attr);
bool attribute_validity(const std::string &attr);
bool attribute_value_validity(const std::string &attr, const std::string &attr_value);
std::map<std::string, int> count_values(const std::vector<Car> &examples, const std::string &attr);
std::pair<std::string, int> plurality_value(const std::vector<Car> &examples);
std::vector<Car> filter_examples(const std::vector<Car> &examples, const std::string attr,
                                 const std::string attr_value);
double get_remainder(const std::string &attr, const std::vector<Car> &examples);
Node *learn_decision_tree(const std::vector<Car> &examples, const std::vector<std::string> &attribute_names,
                          const std::vector<Car> parent_examples);

#endif