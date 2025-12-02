#include "../common/common.h"
#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

bool is_invalid(uint64_t id) {
  std::string s = std::to_string(id);
  if (s.size() % 2 != 0)
    return false;
  size_t half = s.size() / 2;
  return s.substr(0, half) == s.substr(half);
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
  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day 2/input";

  using ResultType = std::tuple<uint64_t, uint64_t>;
  auto result = readFileByLine<ResultType>(
      input_file, [](std::string_view line, ResultType &accum) -> bool {
        std::regex pattern(R"((\s*\d+\s*)-(\s*\d+\s*))");
        auto it = std::cregex_iterator(line.begin(), line.end(), pattern);
        auto end = std::cregex_iterator();
        if (it == end) {
          return false;
        }
        for (; it != end; ++it) {
          auto& match = *it;

          auto [first, ok1] = to_unsigned<uint64_t>(match[1].str());
          auto [last, ok2] = to_unsigned<uint64_t>(match[2].str());
          if (ok1 && ok2) {
            for (auto id = first; id <= last; ++id) {
              if (is_invalid(id)) {
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
    std::cerr << "Error reading file" << std::endl;
    return 1;
  }
  std::cout << std::get<0>(*result) << " " << std::get<1>(*result) << std::endl;
  return 0;
}
