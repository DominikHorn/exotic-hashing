#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/convenience/builtins.hpp"

namespace dataset {

   /**
    * Loads the datasets values into memory
    * @return a sorted and deduplicated list of all members of the dataset
    */
   std::vector<std::string> load_strings(const std::string& filepath) {
      std::cout << "loading dataset " << filepath << std::endl;

      // open file
      std::ifstream input(filepath, std::ios::in);

      // read file line by line
      std::vector<std::string> dataset;
      for (std::string line; std::getline(input, line);)
         dataset.push_back(line);

      return dataset;
   }
}; // namespace dataset
