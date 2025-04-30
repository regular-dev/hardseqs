/*
 * This file is part of HardSeqs.
 *
 * HardSeqs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HardSeqs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HardSeqs. If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2025 regular-dev team
 */

#pragma once

#include <iostream>
#include <random>

class RandomGenerator 
{
public:
    RandomGenerator() : engine(std::random_device{}()) {}

    // the more percent the more chance for "1"
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