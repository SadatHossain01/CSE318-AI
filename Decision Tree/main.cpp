#include "car.h"
#include <iostream>
#include <vector>

std::vector<Car> cars;

// g++ -std=c++17 -O3 car.cpp main.cpp -o main
// ./main < car.data
int main()
{
    while (true)
    {
        std::string str;
        getline(std::cin, str);
        if (std::cin.eof())
            break;
        // std::cout << str << std::endl;
        Car car(str);
        cars.push_back(car);
    }
}