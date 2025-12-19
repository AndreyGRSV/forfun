/*
 * Puzzle solution for Advent of Code 2025 - Day 6 Part 1
 * "Day 6: Trash Compactor"
 * Problem: Playground - Vertical Digit Operations
 * Perform operations on groups of vertical digits extracted from input numbers.
 * Expected output: 5784380717354
 */
#include "../common/common.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <print>
#include <ranges>
#include <regex>
#include <vector>

int main(int argc, char *argv[]) {

  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day6/input";

  using ResultType = std::vector<std::vector<int>>;

  std::vector<char> o_line{};

  auto result = puzzles::common::readFileByLine<ResultType>(
      input_file, [&](std::string_view line, ResultType &groups) {
        // Check if this is the operator line
        if (line.find('*') != std::string::npos ||
            line.find('+') != std::string::npos) {
          // Parse operators
          std::regex op_pattern{R"([+*])"};
          for (std::cregex_iterator it(
                   &line.data()[0], &line.data()[0] + line.size(), op_pattern),
               end;
               it != end; ++it) {
            o_line.push_back((*it).str()[0]);
          }
        } else {

          // Parse numbers from current row
          std::vector<int> current_row{};
          std::regex number_pattern{R"(\d{1,4})"};
          for (std::cregex_iterator
                   it(&line.data()[0], &line.data()[0] + line.size(),
                      number_pattern),
               end;
               it != end; ++it) {
            current_row.push_back(std::stoll(it->str()));
          }
          if (!current_row.empty()) {
            if (groups.empty())
              groups.resize(current_row.size());
            for (const auto [idx, value] :
                 current_row | std::views::enumerate) {
              groups[idx].push_back(value);
            }
          }
        }
        return true;
      });

  if (!result) {
    std::println(stderr, puzzles::common::InputFileError);
    return 1;
  }

  const auto &d_groups = *result;

  auto numberOfDigits10 = [](int value) {
    int number_counter = 0;
    while (value) {
      number_counter++;
      value /= 10;
    }
    return number_counter;
  };

  // First part
  uint64_t sum = 0;
  for (const auto &[op, gr] : std::views::zip(o_line, d_groups)) {
    if (gr.empty())
      break;
    if (op == '+')
      sum += std::ranges::fold_left(gr, 0, std::plus<uint64_t>{});
    else if (op == '*') {
      sum += std::ranges::fold_left(gr, 1, std::multiplies<uint64_t>{});
    } else {
      std::println(stderr, "Error of input data operation {}", op);
      return 1;
    }

    auto d_d = numberOfDigits10(
        *std::ranges::max_element(gr, [](int a, int b) { return a < b; }));

    auto group_copy{gr};
    for (auto &value : group_copy) {
      std::vector<std::optional<int>> vertical_digit{};
      for (int i = 0; i < d_d; ++i) {
        int div = 0;
        // auto d = value / div;
        if (value > div) {
          if (op == '+') { // align left
            div = std::pow(10, d_d - i - 1);
            vertical_digit.push_back(value / div);
          }
          if (op == '*') { // align right
            vertical_digit.push_back(value % 10);
          }
        } else
          vertical_digit.push_back(std::nullopt);
        value /= value > 10 ? 10 : 1;
      }
    }
  }

  std::println("Result {}", sum);

  return 0;
}
