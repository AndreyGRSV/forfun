/*
 * Puzzle solution for Advent of Code 2025 - Day 3
 * Problem: Playground - Maximum Joltage from Digit Banks
 * Calculate maximum joltage values from digit banks
 * using different digit lengths.
 * Expected output: 16858 167549941654721
 */
#include "../common/common.h"
#include <iostream>
#include <print>
#include <string>

using MaxPositionData = std::tuple<int, int>;
MaxPositionData get_max_from(int pos, std::string_view bank, size_t max) {
  int max_val = 0;
  size_t max_pos = pos;
  for (size_t i = pos; i < max; ++i) {
    if (bank[i] - '0' > max_val) {
      max_val = bank[i] - '0';
      max_pos = i;
    }
    if (max_val == 9) {
      break;
    }
  }
  return std::make_tuple(max_val, max_pos);
}

uint64_t get_max_joltage(std::string_view bank, int max_digits = 12) {
  uint64_t max_joltage = 0;
  MaxPositionData max_pos = std::make_tuple(0, 0);
  for (int i = max_digits - 1; i >= 0; --i) {
    max_pos = get_max_from(std::get<1>(max_pos) + (i == max_digits - 1 ? 0 : 1),
                           bank, bank.size() - i);
    max_joltage = max_joltage * 10 + std::get<0>(max_pos);
  }
  return max_joltage;
}

int main(int argc, char *argv[]) {
  namespace pc = puzzles::common;

  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day3/input";

  using ResultType = std::tuple<uint64_t, uint64_t>;

  auto result = pc::readFileByLine<ResultType>(
      input_file, [](std::string_view line, ResultType &accum) -> bool {
        std::get<0>(accum) += get_max_joltage(line, 2);
        std::get<1>(accum) += get_max_joltage(line, 12);
        return true;
      });

  if (!result) {
    std::println(stderr, pc::InputFileError);
    return 1;
  }
  std::println("{} {}", std::get<0>(*result), std::get<1>(*result));
  return 0;
}
