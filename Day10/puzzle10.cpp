/*
 * Day 10: Joltage Adapters - Part 1
 * "Day 10: Factory"
 * Using Gaussian elimination over GF(2) to minimize button presses
 * Expected output: 517
 */
#include "../common/common.h"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <print>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

namespace ranges = std::ranges;
namespace views = std::views;

constexpr size_t MAX_LIGHTS = 64;

struct Machine {
  std::bitset<MAX_LIGHTS> target;
  std::vector<std::bitset<MAX_LIGHTS>> buttons;
  size_t num_lights;
};

// Parse a single machine line
std::expected<Machine, bool> parseMachine(std::string_view line) {
  Machine machine;

  // Extract target configuration [.##.#...]
  std::regex target_regex(R"(\[([.#]+)\])");
  std::regex buttons_regex(R"(\(([0-9,]+)\))");

  std::string line_str(line);
  std::smatch match;

  // Parse target
  if (std::regex_search(line_str, match, target_regex)) {
    std::string target_str = match[1].str();
    machine.num_lights = target_str.size();
    for (size_t i = 0; i < target_str.size(); ++i) {
      if (target_str[i] == '#') {
        machine.target.set(i);
      }
    }
  } else {
    return std::unexpected(false);
  }

  // Parse buttons
  auto buttons_begin =
      std::sregex_iterator(line_str.begin(), line_str.end(), buttons_regex);
  auto buttons_end = std::sregex_iterator();

  for (auto it = buttons_begin; it != buttons_end; ++it) {
    std::smatch button_match = *it;
    std::string button_str = button_match[1].str();

    std::bitset<MAX_LIGHTS> button;
    std::istringstream iss(button_str);
    std::string num;
    while (std::getline(iss, num, ',')) {
      size_t idx = std::stoull(num);
      button.set(idx);
    }
    machine.buttons.push_back(button);
  }

  return machine;
}

// Solve using Gaussian elimination over GF(2) with optimization for minimum
// presses
size_t solveMachine(const Machine &machine) {
  size_t n = machine.num_lights;
  size_t m = machine.buttons.size();

  // Create augmented matrix [A|b] where A is the button matrix and b is target
  std::vector<std::bitset<MAX_LIGHTS + 1>> matrix(n);

  // Fill matrix: each row represents a light, each column represents a button
  for (size_t light = 0; light < n; ++light) {
    for (size_t btn = 0; btn < m; ++btn) {
      if (machine.buttons[btn].test(light)) {
        matrix[light].set(btn);
      }
    }
    // Set augmented column (target)
    if (machine.target.test(light)) {
      matrix[light].set(m);
    }
  }

  // Gaussian elimination to reduced row echelon form
  std::vector<int> pivot_col(n, -1);
  size_t current_row = 0;

  for (size_t col = 0; col < m && current_row < n; ++col) {
    // Find pivot
    size_t pivot_row = SIZE_MAX;
    for (size_t row = current_row; row < n; ++row) {
      if (matrix[row].test(col)) {
        pivot_row = row;
        break;
      }
    }

    if (pivot_row == SIZE_MAX)
      continue;

    // Swap rows
    if (pivot_row != current_row) {
      std::swap(matrix[pivot_row], matrix[current_row]);
    }

    pivot_col[current_row] = col;

    // Eliminate
    for (size_t row = 0; row < n; ++row) {
      if (row != current_row && matrix[row].test(col)) {
        matrix[row] ^= matrix[current_row];
      }
    }

    ++current_row;
  }

  // Check for inconsistency
  for (size_t row = current_row; row < n; ++row) {
    if (matrix[row].test(m)) {
      return SIZE_MAX; // No solution
    }
  }

  // Identify free variables (buttons not used as pivots)
  std::vector<size_t> free_vars;
  std::vector<bool> is_pivot(m, false);

  for (size_t row = 0; row < current_row; ++row) {
    if (pivot_col[row] >= 0) {
      is_pivot[pivot_col[row]] = true;
    }
  }

  for (size_t col = 0; col < m; ++col) {
    if (!is_pivot[col]) {
      free_vars.push_back(col);
    }
  }

  // Try all combinations of free variables to find minimum
  size_t min_presses = SIZE_MAX;
  size_t num_free = free_vars.size();

  for (size_t mask = 0; mask < (1ULL << num_free); ++mask) {
    std::bitset<MAX_LIGHTS> solution;

    // Set free variables according to mask
    for (size_t i = 0; i < num_free; ++i) {
      if (mask & (1ULL << i)) {
        solution.set(free_vars[i]);
      }
    }

    // Determine pivot variables
    for (size_t row = 0; row < current_row; ++row) {
      if (pivot_col[row] >= 0) {
        bool val = matrix[row].test(m);
        // XOR with free variables in this row
        for (size_t col = 0; col < m; ++col) {
          if (!is_pivot[col] && matrix[row].test(col) && solution.test(col)) {
            val = !val;
          }
        }
        solution.set(pivot_col[row], val);
      }
    }

    // Count presses
    size_t presses = solution.count();
    min_presses = std::min(min_presses, presses);
  }

  return min_presses;
}

int main(int argc, char *argv[]) {
  std::string input_file = (argc > 1) ? argv[1] : "../Day 10/input";

  namespace pc = puzzles::common;
  auto result = pc::readFileByLine<size_t>(
      input_file, [](std::string_view line, size_t &total) -> bool {
        if (line.empty())
          return true;

        auto machine_result = parseMachine(line);
        if (!machine_result) {
          std::println(stderr, "Failed to parse machine");
          return false;
        }

        size_t presses = solveMachine(*machine_result);
        if (presses == SIZE_MAX) {
          std::println(stderr, "No solution found for machine");
          return false;
        }

        total += presses;
        return true;
      });

  if (result) {
    std::println("Total button presses: {}", *result);
  } else {
    std::println(stderr, pc::InputFileError);
    return 1;
  }

  return 0;
}
