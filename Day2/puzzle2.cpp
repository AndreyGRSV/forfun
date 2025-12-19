/*
 * Puzzle solution for Advent of Code 2025 - Day 2
 * "Day 2: Gift Shop"
 * Problem: Playground - ID Number Validation
 * Validate ID numbers based on specific digit patterns.
 * Expected output: 12850231731 24774350322
 */

#include "../common/common.h"
#include <iostream>
#include <print>
#include <regex>

// Simple variant using string conversion
// Check if a number is invalid (pattern repeated in the two halves)
bool is_invalid(uint64_t id) {
  std::string s = std::to_string(id);
  if (s.size() % 2 != 0)
    return false;
  size_t half = s.size() / 2;
  return s.substr(0, half) == s.substr(half);
}

// Without string conversion
// Check if a number is valid (no digit repeated in corresponding positions)
// Example: 1234 is valid, 1212 is invalid (12 repeated), 123123 is invalid
// TODO: Try to optimize with constevaluation and folding
constexpr auto DIGITS_STEP = 100;
constexpr auto DIVID_STEP = 10;

bool is_valid(uint64_t id, uint64_t max = DIGITS_STEP,
              uint64_t min = DIVID_STEP, uint64_t divider = DIVID_STEP) {
  if (divider > max)
    return true;
  if (id < max && id >= min)
    return id / divider != id % divider;
  return is_valid(id, max * DIGITS_STEP, min * DIGITS_STEP,
                  divider * DIVID_STEP);
}

// Check if a number is invalid (pattern repeated at least twice)
bool is_invalid2(uint64_t id) {
  std::string s = std::to_string(id);
  size_t len = s.size();

  // Try all possible pattern lengths from 1 to len/2
  for (size_t pattern_len = 1; pattern_len <= len / 2; ++pattern_len) {
    // Check if the length is divisible by pattern_len
    if (len % pattern_len != 0)
      continue;

    // Extract the pattern
    std::string pattern = s.substr(0, pattern_len);

    // Check if the entire string is made of this pattern repeated
    bool is_repeated = true;
    for (size_t i = pattern_len; i < len; i += pattern_len) {
      if (s.substr(i, pattern_len) != pattern) {
        is_repeated = false;
        break;
      }
    }

    if (is_repeated) {
      return true;
    }
  }

  return false;
}

int main(int argc, char *argv[]) {

  namespace pc = puzzles::common;

  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day2/input";

  using ResultType = std::tuple<uint64_t, uint64_t>;
  auto result = pc::readFileByLine<ResultType>(
      input_file, [](std::string_view line, ResultType &accum) -> bool {
        std::regex pattern(R"((\s*\d+\s*)-(\s*\d+\s*))");
        auto it = std::cregex_iterator(&line.data()[0],
                                       &line.data()[0] + line.size(), pattern);
        auto end = std::cregex_iterator();
        if (it == end) {
          return false;
        }
        for (; it != end; ++it) {
          auto &match = *it;

          auto first = pc::to_unsigned<uint64_t>(match[1].str());
          auto last = pc::to_unsigned<uint64_t>(match[2].str());
          if (first && last) {
            for (auto id = *first; id <= *last; ++id) {
              if (!is_valid(id)) {
                std::get<0>(accum) += id;
              }
              if (is_invalid2(id)) {
                std::get<1>(accum) += id;
              }
            }
          } else {
            return false;
          }
        }
        return true;
      });

  if (!result) {
    std::println(stderr, pc::InputFileError);
    return 1;
  }

  std::println("{} {}", std::get<0>(*result), std::get<1>(*result));
  return 0;
}
