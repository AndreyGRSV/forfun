/*
 * Puzzle solution for Advent of Code 2025 - Day 9
 * Problem: Movie Theater - Largest Rectangle
 *
 * Find the largest rectangle that uses red tiles for two opposite corners.
 * Expected output: 4771532800
 */

#include "../common/common.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Point {
  int64_t x, y;

  bool operator<(const Point &other) const {
    return x > other.x || (x == other.x && y < other.y);
  }

  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }
};

int main(int argc, char *argv[]) {
  const std::filesystem::path input_file{(argc > 1) ? argv[1]
                                                    : "../../Day9/input"};

  // Read red tile positions
  using ResultType = std::vector<Point>;
  auto result_tiles = puzzles::common::readFileByLine<ResultType>(
      input_file, [](std::string_view line, ResultType &tiles) -> bool {
        if (line.empty())
          return true;

        Point point;
        char comma;
        std::istringstream iss{std::string(line)};

        if (iss >> point.x >> comma >> point.y) {
          tiles.push_back(point);
          return true;
        }
        return false;
      });

  if (!result_tiles) {
    std::println(stderr, "Error reading input file");
    return 1;
  }

  auto &tiles = result_tiles.value();

  int64_t max_area = 0;

  // Try all pairs of tiles as opposite corners
  for (size_t i = 0; i < tiles.size(); i++) {
    for (size_t j = i + 1; j < tiles.size(); j++) {
      const auto &p1 = tiles[i];
      const auto &p2 = tiles[j];

      // Skip if they're on the same horizontal or vertical line
      if (p1.x == p2.x || p1.y == p2.y) {
        continue;
      }

      // Calculate area of rectangle with p1 and p2 as opposite corners
      // Add 1 to include both corner tiles
      int64_t width = std::abs(p2.x - p1.x) + 1;
      int64_t height = std::abs(p2.y - p1.y) + 1;
      int64_t area = width * height;

      max_area = std::max(max_area, area);
    }
  }

  std::println("{}", max_area);

  return 0;
}
