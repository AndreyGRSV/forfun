#include "../common/common.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  const std::string input_file = (argc > 1) ? argv[1] : "../Day 4/input";

  auto result = readFileByLine<std::vector<std::string>>(
      input_file,
      [](std::string_view line, std::vector<std::string> &accumulate) {
        accumulate.push_back(std::string(line));
        return true;
      });

  if (!result) {
    std::cerr << "Error reading file" << std::endl;
    return 1;
  }

  using RemoveList = std::vector<std::pair<int, int>>;
  auto calculate_accessible = [=](const std::vector<std::string> &grid)
      -> std::expected<RemoveList, bool> {
    int rows = grid.size();
    if (rows == 0) {
      std::cerr << "Empty grid." << std::endl;
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
    std::cerr << "Error calculating accessible rolls." << std::endl;
    return 1;
  }
  int total_accessed = (*to_remove_result).size();

  auto grid_copy = *result;
  int total_removed = 0;
  while (true) {
    auto to_remove_result = calculate_accessible(grid_copy);
    if (!to_remove_result) {
      std::cerr << "Error calculating accessible rolls." << std::endl;
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

  std::cout << total_accessed << " " << total_removed << std::endl;

  return 0;
}
