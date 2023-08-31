#include "decision_tree.h"
#include <cassert>
#include <iostream>
#include <math.h>

// Returns a vector of all possible values for a given attribute
std::vector<std::string> get_attribute_values(const std::string attr)
{
    if (attr == buy_attr)
        return buy_attr_values;
    else if (attr == maint_attr)
        return maint_attr_values;
    else if (attr == door_attr)
        return door_attr_values;
    else if (attr == person_attr)
        return person_attr_values;
    else if (attr == lug_attr)
        return lug_attr_values;
    else if (attr == safety_attr)
        return safety_attr_values;
    else
        assert(false);
}

// Assert that the attribute string is one of the specified attribute names
bool attribute_validity(const std::string &attr)
{
    return (attr == buy_attr || attr == maint_attr || attr == door_attr || attr == person_attr || attr == lug_attr ||
            attr == safety_attr || attr == class_attr);
}

// Assert that the attribute value is one of the specified attribute values
bool attribute_value_validity(const std::string &attr, const std::string &&attr_value)
{
    if (!attribute_validity(attr))
        return false;
    std::vector<std::string> value_vector = get_attribute_values(attr);

    for (auto value : value_vector)
        if (value == attr_value)
            return true;
    return false;
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

// Returns the information gain for a certain attribute attr
double get_remainder(const std::string &attr, const std::vector<Car> &examples)
{
    const int example_size = examples.size();

    std::map<std::string, int> map = count_values(examples, attr);
    double remainder = 0.0;
    for (auto they : map)
    {
        // they.first is an attribute value for attribute attr.
        // they.second is the number of occurences of that attribute value in examples
        if (they.second == 0)
            continue;
        double p = (double)they.second / example_size;

        // filtered_examples is a vector of examples that have attr_value for attribute attr
        std::vector<Car> filtered_examples = filter_examples(examples, attr, they.first);
        std::map<std::string, int> filtered_map = count_values(filtered_examples, class_attr);
        double entropy = 0.0;
        for (auto filtered_they : filtered_map)
        {
            double q = (double)filtered_they.second / they.second;
            if (q == 0)
                continue;
            entropy -= q * log2(q);
        }
        remainder += p * entropy;
    }

    return remainder;
}

Node *learn_decision_tree(const std::vector<Car> &examples, const std::vector<std::string> &attributes,
                          const std::vector<Car> parent_examples)
{
    if (examples.empty())
    {
        Node *tree = new Node(true, plurality_value(parent_examples).first);
        return tree;
    }
    std::pair<std::string, int> pv = plurality_value(examples);
    if (pv.second == examples.size() || attributes.empty())
    {
        // all examples have the same classification
        // or no attributes left to check
        Node *tree = new Node(true, pv.first);
        return tree;
    }

    // find the attribute with the highest importance (min_remainder)
    double min_remainder = 1e9;
    std::string max_important_attr;
    for (auto attr : attributes)
    {
        double rem = get_remainder(attr, examples);
        if (rem < min_remainder)
        {
            min_remainder = rem;
            max_important_attr = attr;
        }
    }

    // now create a new decision tree with root as max_important_attr
    Node *tree = new Node();
    tree->attribute_to_check = max_important_attr;
    std::vector<std::string> new_attributes;
    for (auto attr : attributes)
        if (attr != max_important_attr)
            new_attributes.push_back(attr);

    std::vector<std::string> attr_values = get_attribute_values(max_important_attr);
    for (auto attr_value : attr_values)
    {
        std::vector<Car> filtered_examples = filter_examples(examples, max_important_attr, attr_value);
        Node *subtree = learn_decision_tree(filtered_examples, new_attributes, examples);
        tree->child[attr_value] = subtree;
    }
    return tree;
}

Node::~Node()
{
    if (is_leaf)
        return;
    for (auto they : child)
        if (they.second != nullptr)
            delete they.second;
}

std::string Tree::get_classification(Car car)
{
    Node *node = root;
    while (!node->is_leaf)
    {
        std::string attr_value = car.attributes[node->attribute_to_check];
        node = node->child[attr_value];
    }
    return node->classification;
}

void Tree::print_tree(Node *node, int depth)
{
    if (node->is_leaf)
    {
        for (int i = 0; i < depth; i++)
            std::cout << "\t";
        std::cout << node->classification << std::endl;
        return;
    }
    for (auto they : node->child)
    {   
        std::cout << std::endl;
        for (int i = 0; i < depth; i++)
            std::cout << "\t";
        std::cout << node->attribute_to_check << " = " << they.first << " : ";
        print_tree(they.second, depth + 1);
    }
}