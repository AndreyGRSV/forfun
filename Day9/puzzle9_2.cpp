/*
 * Puzzle solution for Advent of Code 2025 - Day 9 Part 2
 * Problem: Movie Theater - Largest Rectangle with Red/Green Tiles
 *
 * Find the largest rectangle that uses red tiles for two opposite corners
 * and only contains red or green tiles inside.
 * Uses OpenMP to parallelize the point-in-polygon checks.
 * Command line argument 2 can be used to skip a number of largest areas
 *  ./puzzle9_2 ../../Day9/input 49062
 * Expected output:
 *  4771532800 1544362560
 */

#include "../common/common.h"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <print>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Point {
  int64_t x, y;

  bool operator<(const Point &other) const {
    return x < other.x || (x == other.x && y < other.y);
  }

  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }
};

// Check if a point is inside the polygon using ray casting algorithm
bool isInsidePolygon(const Point &p, const std::vector<Point> &polygon) {
  int n = (int)polygon.size();
  bool inside = false;

  for (int i = 0, j = n - 1; i < n; j = i++) {
    const auto &pi = polygon[i];
    const auto &pj = polygon[j];
    int64_t yi = pi.y, yj = pj.y;

    // check if edge crosses horizontal ray at p.y
    if ((yi > p.y) != (yj > p.y)) {
      int64_t dy = yj - yi; // non-zero
      int64_t dx = pj.x - pi.x;

      // val = (pi.x - p.x) * dy + dx * (p.y - pi.y)
      // original condition p.x < pi.x + dx*(p.y-pi.y)/dy
      // becomes val*dy > 0 (handles sign of dy)
      __int128 val = (__int128)(pi.x - p.x) * (__int128)dy +
                     (__int128)dx * (__int128)(p.y - pi.y);
      if (val * (__int128)dy > 0)
        inside = !inside;
    }
  }

  return inside;
}

// Check if a point is inside the polygon using ray casting algorithm
bool isInsidePolygon1(const Point &p, const std::vector<Point> &polygon) {
  int n = polygon.size();
  bool inside = false;

  for (int i = 0, j = n - 1; i < n; j = i++) {
    const auto &pi = polygon[i];
    const auto &pj = polygon[j];

    if ((pj.y == pi.y && pi.y == p.y) || (pj.x == pi.x && pi.x == p.x))
      return true; // on vertex

    if (((pi.y > p.y) != (pj.y > p.y)) &&
        (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y) + pi.x)) {
      inside = !inside;
    }
  }

  return inside;
}

int main(int argc, char *argv[]) {
  namespace pc = puzzles::common;
  const std::filesystem::path input_file{
      (argc > 1) ? argv[1] : "../../Day9/test_input.txt"};
  const int START_POSITION = (argc > 2) ? std::stoi(argv[2]) : 0;
  std::println("File: {} Start position: {}", input_file.string(),
               START_POSITION);

  // Set number of threads
  int num_threads = omp_get_max_threads();
  std::println("Using {} OpenMP threads", num_threads);

  // Read red tile positions
  using ResultType = std::vector<Point>;
  auto result_tiles = pc::readFileByLine<ResultType>(
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
    std::println(stderr, pc::InputFileError);
    return 1;
  }

  auto &red_tiles = result_tiles.value();

  int64_t max_area = 0;

  const int AREA = 0;
  const int MIN_X = 1;
  const int MAX_X = 2;
  const int MIN_Y = 3;
  const int MAX_Y = 4;
  // area, min_x,max_x,min_y,max_y
  std::vector<std::tuple<int64_t, int64_t, int64_t, int64_t, int64_t>> areas{};

  // Generate all possible rectangles defined by pairs of red tiles
  for (size_t i = 0; i < red_tiles.size(); i++) {
    for (size_t j = i + 1; j < red_tiles.size(); j++) {
      const auto &p1 = red_tiles[i];
      const auto &p2 = red_tiles[j];

      // Skip if they're on the same horizontal or vertical line
      if (p1.x == p2.x || p1.y == p2.y) {
        continue;
      }

      // Check if all tiles in the rectangle are red or green
      int64_t min_x = std::min(p1.x, p2.x);
      int64_t max_x = std::max(p1.x, p2.x);
      int64_t min_y = std::min(p1.y, p2.y);
      int64_t max_y = std::max(p1.y, p2.y);

      int64_t width = max_x - min_x + 1;
      int64_t height = max_y - min_y + 1;
      int64_t area = width * height;
      areas.push_back(std::make_tuple(area, min_x, max_x, min_y, max_y));
    }
  }

  std::sort(areas.rbegin(), areas.rend(), [](const auto &a, const auto &b) {
    return std::get<AREA>(a) < std::get<AREA>(b);
  });

  int drop = START_POSITION; // Skip first N areas already checked
  int area_count = drop;
  for (const auto &area : areas | std::views::drop(drop)) {
    std::println("Trying area {} from {}: {}", area_count++, areas.size(),
                 std::get<AREA>(area));
    std::atomic<bool> all_valid{true};
    const int64_t min_x = std::get<MIN_X>(area);
    const int64_t max_x = std::get<MAX_X>(area);
    const int64_t min_y = std::get<MIN_Y>(area);
    const int64_t max_y = std::get<MAX_Y>(area);

#pragma omp parallel for schedule(dynamic)
    for (int64_t x = min_x; x <= max_x; x++) {
      for (int64_t y = min_y; y <= max_y && all_valid; y++) {
        if (!isInsidePolygon({x, y}, red_tiles)) {
          all_valid = false;
        }
      }
    }

    if (all_valid) {
      max_area = std::get<AREA>(area);
      break;
    }
  }

  std::println("{} {}", std::get<0>(areas.front()), max_area);

  return 0;
}
