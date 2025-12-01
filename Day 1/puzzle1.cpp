/*
* Puzzle solution for Advent of Code 2025 - Day 1
*/

#include "../common/common.h"
#include <iostream>

int main(int argc, char *argv[]) {
  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day 1/input";

  auto result = readFileByLine<std::tuple<int,int>>(
      input_file, [](std::string_view line, std::tuple<int,int> &accumulate) -> bool {
        static int position = 50; // Start at position 50
        if (line.empty())
          return true;

        char direction = line[0];
        auto [distance, ok] = to_unsigned<unsigned>(line.substr(1));
        if (!ok || distance > 1000)
          return false;

        auto rotations = distance / 100;
        auto remainder = distance % 100;
        auto last_position = position;
        if (direction == 'L') {
          position = (position - remainder + 100) % 100;
          if ((position > last_position || position == 0) &&
              last_position != 0) {
            rotations++;
          }
        } else if (direction == 'R') {
          position = (position + remainder) % 100;
          if (position < last_position) {
            rotations++;
          }
        } else {
          return false; // Invalid direction
        }
        std::get<0>(accumulate) += (position == 0) ? 1 : 0;
        std::get<1>(accumulate) += rotations;
        return true;
      });

  if (result) {
    std::cout << std::get<0>(*result) << " " << std::get<1>(*result) << std::endl;
  } else {
    std::cerr << "Error reading input file." << std::endl;
    return 1;
  }

  return 0;
}
