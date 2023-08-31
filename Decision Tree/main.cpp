#include "car.h"
#include "decision_tree.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <math.h>
#include <time.h>
#include <vector>

const int ITER = 20;
const double train_percentage = 0.8;
std::vector<Car> cars;

/*
    g++ -std=c++14 -O3 car.cpp decision_tree.cpp main.cpp -o main
    ./main "car evaluation dataset/car.data"
*/
int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc < 2)
    {
        std::cout << "Usage: ./main <input_file>" << std::endl;
        return 1;
    }
    std::string input_file = std::string(argv[1]);
    std::ifstream in(input_file);

    while (true)
    {
        std::string str;
        std::getline(in, str);
        if (in.eof())
            break;
        // std::cout << str << std::endl;
        Car car(str);
        cars.push_back(car);
    }

    std::vector<std::string> attributes = {buy_attr, maint_attr, door_attr, person_attr, lug_attr, safety_attr};
    double total_accuracy = 0, total_accuracy_squared = 0;

    for (int i = 0; i < ITER; i++)
    {
        std::random_shuffle(cars.begin(), cars.end());

        std::vector<Car> training_set(cars.begin(), cars.begin() + cars.size() * train_percentage); 
        std::vector<Car> test_set(cars.begin() + cars.size() * train_percentage, cars.end());

        Tree tree(learn_decision_tree(training_set, attributes, training_set));
        // std::cout << "Tree Root: " << tree.root->attribute_to_check << std::endl;
        // tree.print_tree(tree.root);
        std::string predicted, actual;
        int correct = 0;
        for (auto car : test_set)
        {
            predicted = tree.get_classification(car);
            actual = car.attributes[class_attr];
            // std::cout << car.detail << " " << predicted << " " << actual << (predicted == actual ? " correct" : "
            // wrong") << std::endl;
            if (predicted == actual)
                correct++;
        }
        double accuracy = (double)correct / test_set.size();
        std::cout << "Iteration " << i + 1 << " Accuracy:\t" << accuracy << std::endl;
        total_accuracy += accuracy;
        total_accuracy_squared += accuracy * accuracy;
    }

    double mean = total_accuracy / ITER;
    double stdev = sqrt(total_accuracy_squared / ITER - mean * mean);

    std::cout << "Mean Accuracy: " << mean << std::endl;
    std::cout << "Standard Deviation of Accuracy: " << stdev << std::endl;

    in.close();
    return 0;
}