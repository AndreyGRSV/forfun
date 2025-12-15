/*
 * Puzzle solution for Advent of Code 2025 - Day 4
 * " Day 4: Printing Department"
 * Problem: Playground - Roller Coaster Accessibility
 * Determine accessible roller coasters in a grid layout
 * based on adjacent roll counts.
 * Expected output: 1411 8557
 */
#include "../common/common.h"
#include <iostream>
#include <print>
#include <string>
#include <vector>

namespace {
constexpr auto CalculationError = "Error calculating accessible rolls.";
}

int main(int argc, char *argv[]) {
  namespace pc = puzzles::common;
  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day 4/input";

  auto result = pc::readFileByLine<std::vector<std::string>>(
      input_file,
      [](std::string_view line, std::vector<std::string> &accumulate) {
        accumulate.push_back(std::string(line));
        return true;
      });

  if (!result) {
    std::println(stderr, pc::InputFileError);
    return 1;
  }

  using RemoveList = std::vector<std::pair<int, int>>;
  auto calculate_accessible = [=](const std::vector<std::string> &grid)
      -> std::expected<RemoveList, bool> {
    int rows = grid.size();
    if (rows == 0) {
      std::println(stderr, "Empty grid.");
      return std::unexpected(false);
    }
    int cols = grid[0].size();

    auto check_accessible = [=](int x, int y) -> bool {
      return x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == '@';
    };

    RemoveList to_remove{};
    // Direction vectors for 8 adjacent positions
    const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        if (grid[i][j] == '@') {
          // Count adjacent rolls
          int adjacent_rolls = 0;
          for (int d = 0; d < 8; ++d) {
            if (check_accessible(i + dx[d], j + dy[d])) {
              adjacent_rolls++;
            }
          }
          // A roll can be accessed if there are fewer than 4 adjacent rolls
          if (adjacent_rolls < 4) {
            to_remove.push_back({i, j});
          }
        }
      }
    }
    return to_remove;
  };

  auto to_remove_result = calculate_accessible(*result);
  if (!to_remove_result) {
    std::println(stderr, CalculationError);
    return 1;
  }
  int total_accessed = (*to_remove_result).size();

  auto grid_copy = *result;
  int total_removed = 0;
  while (true) {
    auto to_remove_result = calculate_accessible(grid_copy);
    if (!to_remove_result) {
      std::println(stderr, CalculationError);
      return 1;
    }
    // Remove all accessible rolls
    if (!to_remove_result->empty()) {
      total_removed += to_remove_result->size();
      for (const auto &pos : *to_remove_result) {
        grid_copy[pos.first][pos.second] = '.';
      }
    } else {
      break; // No more accessible rolls to remove
    }
  }

  std::println("{} {}", total_accessed, total_removed);
  return 0;
}
