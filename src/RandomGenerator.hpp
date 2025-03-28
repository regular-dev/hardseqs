#pragma once

#include <iostream>
#include <random>

class RandomGenerator 
{
public:
    RandomGenerator() : engine(std::random_device{}()) {}

    int randomPercent(int percent) {
        if (percent < 0 || percent > 100) {
            throw std::invalid_argument("Percentage must be between 0 and 100.");
        }

        std::uniform_int_distribution<int> distribution(0, 99);
        int randomValue = distribution(engine);

        return randomValue < percent ? 1 : 0;
    }

private:
    std::mt19937 engine;
};