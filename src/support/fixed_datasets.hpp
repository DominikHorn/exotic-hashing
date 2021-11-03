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
   template<class Key>
   std::vector<Key> load_fixed_size(const std::string& filepath) {
      std::cout << "loading dataset " << filepath << std::endl;

      // parsing helper functions
      auto read_little_endian_8 = [](const std::vector<unsigned char>& buffer, uint64_t offset) {
         return static_cast<uint64_t>(buffer[offset + 0]) | (static_cast<uint64_t>(buffer[offset + 1]) << 8) |
            (static_cast<uint64_t>(buffer[offset + 2]) << (2 * 8)) |
            (static_cast<uint64_t>(buffer[offset + 3]) << (3 * 8)) |
            (static_cast<uint64_t>(buffer[offset + 4]) << (4 * 8)) |
            (static_cast<uint64_t>(buffer[offset + 5]) << (5 * 8)) |
            (static_cast<uint64_t>(buffer[offset + 6]) << (6 * 8)) |
            (static_cast<uint64_t>(buffer[offset + 7]) << (7 * 8));
      };
      auto read_little_endian_4 = [](const std::vector<unsigned char>& buffer, uint64_t offset) {
         return buffer[offset + 0] | (buffer[offset + 1] << 8) | (buffer[offset + 2] << (2 * 8)) |
            (buffer[offset + 3] << (3 * 8));
      };

      // Read file into memory from disk. Directly map file for more performance
      std::ifstream input(filepath, std::ios::binary | std::ios::ate);
      std::streamsize size = input.tellg();
      input.seekg(0, std::ios::beg);
      if (!input.is_open()) {
         std::cerr << "file '" + filepath + "' does not exist" << std::endl;
         return {};
      }

      const auto max_num_elements = (size - sizeof(std::uint64_t)) / sizeof(Key);
      std::vector<uint64_t> dataset(max_num_elements, 0);
      {
         std::vector<unsigned char> buffer(size);
         if (!input.read(reinterpret_cast<char*>(buffer.data()), size))
            throw std::runtime_error("Failed to read dataset '" + filepath + "'");

         // Parse file
         uint64_t num_elements = read_little_endian_8(buffer, 0);
         assert(num_elements <= max_num_elements);
         switch (sizeof(Key)) {
            case sizeof(std::uint64_t):
               for (uint64_t i = 0; i < num_elements; i++) {
                  // 8 byte header, 8 bytes per entry
                  uint64_t offset = i * 8 + 8;
                  dataset[i] = read_little_endian_8(buffer, offset);
               }
               break;
            case sizeof(std::uint32_t):
               for (uint64_t i = 0; i < num_elements; i++) {
                  // 8 byte header, 4 bytes per entry
                  uint64_t offset = i * 4 + 8;
                  dataset[i] = read_little_endian_4(buffer, offset);
               }
               break;
            default:
               throw std::runtime_error("unimplemented amount of bytes per value in dataset: " +
                                        std::to_string(sizeof(Key)));
         }
      }

      return dataset;
   }
}; // namespace dataset
