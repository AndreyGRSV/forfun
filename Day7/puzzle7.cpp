/*
 * Puzzle solution for Advent of Code 2025 - Day 7
 * "Day 7: Laboratories"
 * Problem: Tachyon Manifold Beam Splitting
 *
 * A tachyon beam enters at 'S' and moves downward.
 * When it hits a splitter '^', it stops and creates two new beams
 * that start from the immediate left and right of the splitter
 * and continue moving downward.
 * Count the total number of times the beam is split.
 * Expected output: 1602 135656430050438
 */

#include "../common/common.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <print>
#include <queue>
#include <set>
#include <string>
#include <vector>

struct Beam {
  int row;
  int col;
};

int main(int argc, char *argv[]) {
  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day7/input";

  // Read the grid
  using ResultType = std::vector<std::string>;
  int start_row = -1, start_col = -1;
  auto result = puzzles::common::readFileByLine<ResultType>(
      input_file, [&](std::string_view line, ResultType &grid) {
        if (!line.empty()) {
          // Find starting position 'S'
          size_t pos = line.find('S');
          if (pos != std::string::npos) {
            start_row = grid.size();
            start_col = pos;
          }
          grid.push_back(std::string(line));
          return true;
        }
        return false;
      });

  if (!result) {
    std::println(stderr, "Error reading input file {}", input_file.string());
    return 1;
  }

  if (start_row == -1) {
    std::println(stderr, "Starting position 'S' not found");
    return 1;
  }

  const ResultType &grid = result.value();

  int rows = grid.size();
  int cols = grid[0].size();

  //  ============= Part I ==============
  // BFS to simulate beam propagation
  // All beams move downward, we just track their column position
  std::queue<Beam> beams;
  std::set<std::pair<int, int>>
      visited; // (row, col) - track which positions have been visited
  int split_count = 0;

  // Start with initial beam at S
  beams.push({start_row, start_col});
  visited.insert({start_row, start_col});

  while (!beams.empty()) {
    Beam current = beams.front();
    beams.pop();

    // Move beam downward
    int next_row = current.row + 1;
    int next_col = current.col;

    // Check bounds
    if (next_row >= rows) {
      continue; // Beam exits the manifold
    }

    char cell = grid[next_row][next_col];

    if (cell == '^') {
      // Hit a splitter - beam stops and splits
      split_count++;

      // Create left beam starting from immediate left of splitter
      int left_col = next_col - 1;
      if (left_col >= 0 &&
          visited.find({next_row, left_col}) == visited.end()) {
        visited.insert({next_row, left_col});
        beams.push({next_row, left_col});
      }

      // Create right beam starting from immediate right of splitter
      int right_col = next_col + 1;
      if (right_col < cols &&
          visited.find({next_row, right_col}) == visited.end()) {
        visited.insert({next_row, right_col});
        beams.push({next_row, right_col});
      }
    } else if (cell == '.' || cell == 'S') {
      // Continue downward
      if (visited.find({next_row, next_col}) == visited.end()) {
        visited.insert({next_row, next_col});
        beams.push({next_row, next_col});
      }
    }
  }

  //  ============= Part II ==============
  // Use dynamic programming: count[row][col] = number of timelines reaching
  // this position
  std::map<std::pair<int, int>, uint64_t> count;
  count[{start_row, start_col}] = 1;

  // Process row by row from top to bottom
  for (int row = start_row; row < rows; row++) {
    std::map<std::pair<int, int>, uint64_t> next_count;

    for (const auto &[pos, num_timelines] : count) {
      if (pos.first != row)
        continue;

      int curr_row = pos.first;
      int curr_col = pos.second;

      // Move downward
      int next_row = curr_row + 1;

      // Check if we've exited the manifold
      if (next_row >= rows) {
        // These timelines exit here - add to final count
        next_count[{next_row, curr_col}] += num_timelines;
        continue;
      }

      char cell = grid[next_row][curr_col];

      if (cell == '^') {
        // Hit a splitter - particle takes BOTH paths
        // Each timeline splits into two timelines

        // Left path
        int left_col = curr_col - 1;
        if (left_col >= 0) {
          next_count[{next_row, left_col}] += num_timelines;
        }

        // Right path
        int right_col = curr_col + 1;
        if (right_col < cols) {
          next_count[{next_row, right_col}] += num_timelines;
        }
      } else if (cell == '.' || cell == 'S') {
        // Continue downward in all timelines
        next_count[{next_row, curr_col}] += num_timelines;
      }
    }

    // Merge next_count into count
    for (const auto &[pos, num] : next_count) {
      count[pos] = num;
    }
  }

  // Sum up all timelines that exited the manifold
  uint64_t total_timelines = 0;
  for (const auto &[pos, num_timelines] : count) {
    if (pos.first >= rows) {
      total_timelines += num_timelines;
    }
  }

  std::println("Total timelines exiting the manifold: {}\n"
               "Split counter: {}",
               total_timelines, split_count);

  return 0;
}
