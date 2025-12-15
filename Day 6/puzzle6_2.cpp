/*
 * Puzzle solution for Advent of Code 2025 - Day 6 Part 2
 * "Day 6: Trash Compactor"
 * Problem: Playground - Vertical Digit Operations
 * Perform operations on groups of vertical digits extracted from input numbers.
 * Expected output: 7996218225744
 */
#include "../common/common.h"
#include <algorithm>
#include <print>
#include <ranges>
#include <regex>
#include <vector>

int main(int argc, char *argv[]) {
  using namespace std;

  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day 6/input";

  auto result = puzzles::common::readFileByLine<std::vector<std::string>>(
      input_file,
      [](std::string_view line, std::vector<std::string> &accumulate) {
        accumulate.push_back(std::string(line));
        return true;
      });

  if (!result) {
    std::println(stderr, puzzles::common::InputFileError);
    return 1;
  }

  const auto &lines = *result;
  if (lines.size() < 2) {
    std::println(stderr, "Not enough lines in input");
    return 1;
  }

  // Find maximum line length and pad all lines for correct column processing
  size_t maxLen = ranges::max(
      lines | views::transform([](const string &s) { return s.length(); }));

  for (auto &l : *result) {
    l.resize(maxLen, ' ');
  }

  // Parse operators from last line using regex
  string operators = lines.back();
  regex opPattern(R"([+*])");
  vector<pair<size_t, char>> opPositions;

  for (sregex_iterator it(operators.begin(), operators.end(), opPattern), end;
       it != end; ++it) {
    opPositions.push_back({it->position(), (*it)[0].str()[0]});
  }

  // Find all columns that are completely empty (all spaces in all rows)
  // Separators
  vector<bool> isSeparatorColumn(maxLen, true);
  for (size_t col = 0; col < maxLen; col++) {
    for (const auto &l : lines) {
      if (l[col] != ' ') {
        isSeparatorColumn[col] = false;
        break;
      }
    }
  }

  // Identify column groups separated by empty columns
  vector<pair<size_t, size_t>> columnGroups;
  size_t i = 0;
  while (i < maxLen) {
    // Skip empty columns
    while (i < maxLen && isSeparatorColumn[i]) {
      i++;
    }

    if (i >= maxLen)
      break;

    // Found start of a column group
    size_t groupStart = i;
    while (i < maxLen && !isSeparatorColumn[i]) {
      i++;
    }
    size_t groupEnd = i - 1;

    columnGroups.push_back({groupStart, groupEnd});
  }

  // Process each column group
  vector<uint64_t> results;

  for (const auto &[groupStart, groupEnd] : columnGroups) {
    // Find operator in this column group
    char op = ' ';
    for (const auto &[opPos, opChar] : opPositions) {
      if (opPos >= groupStart && opPos <= groupEnd) {
        op = opChar;
        break;
      }
    }

    if (op == ' ')
      continue;

    // Get the width of this group
    size_t groupWidth = groupEnd - groupStart + 1;

    // For each row (except operator row), reverse the segment to get positions
    vector<string> reversedRows;
    for (size_t row = 0; row < lines.size() - 1; row++) {
      string segment = lines[row].substr(groupStart, groupWidth);
      ranges::reverse(segment);
      reversedRows.push_back(segment);
    }

    // Now each position (column in reversed rows) forms a number vertically
    vector<uint64_t> numbers;

    for (size_t pos = 0; pos < groupWidth; pos++) {
      string numStr;

      // Read this position from all rows (top to bottom)
      for (const auto &revRow : reversedRows) {
        char ch = revRow[pos];
        if (ch >= '0' && ch <= '9') {
          numStr += ch;
        }
      }

      // Convert to number if we found any digits
      if (!numStr.empty()) {
        auto num = puzzles::common::to_unsigned<uint64_t>(numStr);
        if (num) {
          numbers.push_back(*num);
        } else {
          println(stderr, "Error converting '{}' to number", numStr);
        }
      }
    }

    // Calculate result for this problem
    if (!numbers.empty()) {
      long long result = numbers[0];

      if (op == '+') {
        result = ranges::fold_left(numbers, 0LL, plus<>());
      } else if (op == '*') {
        result = ranges::fold_left(numbers, 1LL, multiplies<>());
      }

      results.push_back(result);
    }
  }

  // Calculate grand total
  uint64_t grandTotal = ranges::fold_left(results, 0LL, plus<>());

  println("Total: {}", grandTotal);

  return 0;
}
