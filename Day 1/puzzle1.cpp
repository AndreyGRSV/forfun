/*
 * Puzzle solution for Advent of Code 2025 - Day 1
 * "Secret Entrance"
 * Problem: Playground - Circular Track Navigation
 * Navigate a circular track based on input commands,
 * counting zero crossings and total rotations.
 * Expected output: 1026 5923
 */

#include "../common/common.h"
#include <iostream>

namespace {
constexpr int TRACK_SIZE = 100;
constexpr int START_POSITION = 50;
constexpr int MAX_DISTANCE = 1000;
} // namespace

int main(int argc, char *argv[]) {

  namespace pc = puzzles::common;

  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day 1/input";

  using ResultType = std::tuple<int, int>; // (zero crossings, total rotations)

  auto result = pc::readFileByLine<ResultType>(
      input_file, [](std::string_view line, ResultType &accumulate) -> bool {
        static int position = START_POSITION;
        if (line.empty())
          return true;

        char direction = line[0];
        auto distance = pc::to_unsigned<unsigned>(line.substr(1));
        if (!distance || *distance > MAX_DISTANCE)
          return false;

        auto rotations = *distance / TRACK_SIZE;
        auto remainder = *distance % TRACK_SIZE;
        auto last_position = position;
        if (direction == 'L') {
          position = (position - remainder + TRACK_SIZE) % TRACK_SIZE;
          if ((position > last_position || position == 0) &&
              last_position != 0) {
            rotations++;
          }
        } else if (direction == 'R') {
          position = (position + remainder) % TRACK_SIZE;
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
    std::cout << std::get<0>(*result) << " " << std::get<1>(*result)
              << std::endl;
  } else {
    std::cerr << pc::InputFileError << std::endl;
    return 1;
  }

  return 0;
}
