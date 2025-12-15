/*
 * Day 10: Joltage Adapters - Part 2
 * "Day 10: Factory"
 * Using GLPK Integer Linear Programming to minimize button presses
 * Expected output: Total button presses: 21469
 */
#include <algorithm>
#include <expected>
#include <fstream>
#include <glpk.h>
#include <iostream>
#include <print>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "../common/common.h"

struct Machine {
  std::vector<size_t> target_joltage;
  std::vector<std::vector<size_t>> buttons;
};

// Parse a single machine line
std::expected<Machine, bool> parseMachine(std::string_view line) {
  Machine machine;

  std::string line_str(line);

  // Parse buttons (0,1,2) format
  std::regex buttons_regex(R"(\(([0-9,]+)\))");
  auto buttons_begin =
      std::sregex_iterator(line_str.begin(), line_str.end(), buttons_regex);
  auto buttons_end = std::sregex_iterator();

  for (auto it = buttons_begin; it != buttons_end; ++it) {
    std::smatch button_match = *it;
    std::string button_str = button_match[1].str();

    std::vector<size_t> button;
    std::istringstream iss(button_str);
    std::string num;
    while (std::getline(iss, num, ',')) {
      button.push_back(std::stoull(num));
    }
    machine.buttons.push_back(button);
  }

  // Parse target joltage {3,5,4,7} format
  std::regex target_regex(R"(\{([0-9,]+)\})");
  std::smatch match;

  if (std::regex_search(line_str, match, target_regex)) {
    std::string target_str = match[1].str();
    std::istringstream iss(target_str);
    std::string num;
    while (std::getline(iss, num, ',')) {
      machine.target_joltage.push_back(std::stoull(num));
    }
  } else {
    return std::unexpected(false);
  }

  return machine;
}

struct GLP {
  glp_prob *lp = nullptr;
  GLP(const GLP &right) {
    lp = glp_create_prob();
    glp_copy_prob(lp, right.lp, GLP_ON);
  }
  GLP(GLP &&right) noexcept : lp(right.lp) { right.lp = nullptr; }
  GLP &operator=(const GLP &right) {
    if (this != &right) {
      if (lp) {
        glp_delete_prob(lp);
      }
      lp = glp_create_prob();
      glp_copy_prob(lp, right.lp, GLP_ON);
    }
    return *this;
  }
  GLP &operator=(GLP &&right) noexcept {
    if (this != &right) {
      if (lp) {
        glp_delete_prob(lp);
      }
      lp = right.lp;
      right.lp = nullptr;
    }
    return *this;
  }
  GLP() { lp = glp_create_prob(); }
  ~GLP() {
    if (lp) {
      glp_delete_prob(lp);
    }
  }
  operator glp_prob *() const { return lp; }

  void add_rows(size_t n, std::string_view prefix,
                const std::vector<size_t> &targets, int type = GLP_FX) {
    glp_add_rows(lp, n);
    for (size_t i = 0; i < n; ++i) {
      glp_set_row_name(lp, i + 1, (prefix.data() + std::to_string(i)).c_str());
      glp_set_row_bnds(lp, i + 1, type, targets[i], targets[i]);
    }
  }

  void add_cols(size_t n, std::string_view prefix, int type = GLP_IV,
                double lb = 0.0, double coef = 1.0) {
    glp_add_cols(lp, n);
    for (size_t i = 0; i < n; ++i) {
      glp_set_col_name(lp, i + 1, (prefix.data() + std::to_string(i)).c_str());
      glp_set_col_bnds(lp, i + 1, GLP_LO, lb, 0.0); // Lower bound: 0
      glp_set_col_kind(lp, i + 1, type);            // Integer variable
      glp_set_obj_coef(lp, i + 1, coef);            // Objective coefficient: 1
    }
  }
};

// Solve using GLPK Integer Linear Programming
std::expected<size_t, bool> solveMachine(const Machine &machine) {
  size_t num_counters = machine.target_joltage.size();
  size_t num_buttons = machine.buttons.size();

  // Create GLPK problem
  GLP lp{};
  if (!lp.lp) {
    return std::unexpected(false);
  }
  glp_set_prob_name(lp, "JoltageConfiguration");
  glp_set_obj_dir(lp, GLP_MIN); // Minimize

  // Add rows (constraints) - one for each counter
  lp.add_rows(num_counters, "counter_", machine.target_joltage);

  // Add columns (variables) - one for each button
  lp.add_cols(num_buttons, "button_");

  size_t num_elements = std::ranges::fold_left(
      machine.buttons, size_t{0},
      [](size_t sum, const auto &btn) { return sum + btn.size(); });

  // GLPK uses 1-based indexing
  std::vector<int> ia(num_elements + 1);
  std::vector<int> ja(num_elements + 1);
  std::vector<double> ar(num_elements + 1);

  // Fill constraint matrix
  // Each button affects certain counters by +1
  // So for each button j and each counter i it affects, we set A[i][j] = 1
  // Note: counters and buttons are 0-based in our structures, but GLPK is
  // 1-based.
  // clang-format off
  // [.####.#.] (3,4,5,7) (2,4,5,6,7) (1,4,7) (1,3,4,7) (1,2,3,4,5,7) (7) (1,2,3,6) (0,1,3,6,7) {4,59,39,250,242,220,26,250}
  // [1] = 4 + 1  [2] = 59 + 1 [3] = 39 + 1  ...
  // [1] = 0 + 1  [2] = 1 + 1  [3] = 2 + 1   ...
  // [1] = 1.0    [2] = 1.0    [3] = 1.0     ...
  // clang-format on

  size_t idx = 1;
  for (size_t btn = 0; btn < num_buttons; ++btn) {
    for (size_t counter : machine.buttons[btn]) {
      ia[idx] = counter + 1; // Row (counter) - 1-based
      ja[idx] = btn + 1;     // Column (button) - 1-based
      ar[idx] = 1.0;         // Coefficient: 1
      ++idx;
    }
  }

  glp_load_matrix(lp, num_elements, ia.data(), ja.data(), ar.data());

  // Solve the ILP problem
  glp_iocp parm;
  glp_init_iocp(&parm);
  parm.presolve = GLP_ON;
  parm.msg_lev = GLP_MSG_OFF; // Suppress output

  int ret = glp_intopt(lp, &parm);

  size_t result = SIZE_MAX;
  if (ret == 0) {
    int status = glp_mip_status(lp);
    if (status == GLP_OPT || status == GLP_FEAS) {
      result = static_cast<size_t>(glp_mip_obj_val(lp));
    }
  }
  if (result == SIZE_MAX) {
    return std::unexpected(false);
  }

  return result;
}

int main(int argc, char *argv[]) {
  std::string input_file = (argc > 1) ? argv[1] : "../../Day10/input";

  namespace pc = puzzles::common;

  auto result = pc::readFileByLine<size_t>(
      input_file, [](std::string_view line, size_t &total) -> bool {
        size_t machine_num = 0;

        if (line.empty())
          return true;

        ++machine_num;

        auto machine_result = parseMachine(line);
        if (!machine_result) {
          std::println(stderr, "Failed to parse machine {}", machine_num);
          return false;
        }

        auto presses = solveMachine(*machine_result);
        if (!presses) {
          std::println(stderr, "No solution found for machine {}", machine_num);
          return false;
        }

        std::println("Machine {} requires {} presses", machine_num, *presses);
        total += *presses;
        return true;
      });
  if (!result) {
    std::println(stderr, pc::InputFileError);
  }

  std::println("Total button presses: {}", *result);
  return 0;
}
