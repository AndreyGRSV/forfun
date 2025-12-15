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
  glp_add_rows(lp, num_counters);
  for (size_t i = 0; i < num_counters; ++i) {
    glp_set_row_name(lp, i + 1, ("counter_" + std::to_string(i)).c_str());
    // Constraint: sum of button presses = target value
    glp_set_row_bnds(lp, i + 1, GLP_FX, machine.target_joltage[i],
                     machine.target_joltage[i]);
  }

  // Add columns (variables) - one for each button
  glp_add_cols(lp, num_buttons);
  for (size_t i = 0; i < num_buttons; ++i) {
    glp_set_col_name(lp, i + 1, ("button_" + std::to_string(i)).c_str());
    glp_set_col_bnds(lp, i + 1, GLP_LO, 0.0, 0.0); // Lower bound: 0
    glp_set_col_kind(lp, i + 1, GLP_IV);           // Integer variable
    glp_set_obj_coef(lp, i + 1, 1.0);              // Objective coefficient: 1
  }

  // Build constraint matrix
  // Count total non-zero elements
  size_t num_elements = 0;
  for (const auto &button : machine.buttons) {
    num_elements += button.size();
  }

  // GLPK uses 1-based indexing
  std::vector<int> ia(num_elements + 1);
  std::vector<int> ja(num_elements + 1);
  std::vector<double> ar(num_elements + 1);

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

  std::ifstream file(input_file);
  if (!file.is_open()) {
    std::println(stderr, "Error: Could not open file '{}'", input_file);
    return 1;
  }

  size_t total_presses = 0;
  size_t machine_num = 0;
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    ++machine_num;

    auto machine_result = parseMachine(line);
    if (!machine_result) {
      std::println(stderr, "Failed to parse machine {}", machine_num);
      return 1;
    }

    auto presses = solveMachine(*machine_result);
    if (!presses) {
      std::println(stderr, "No solution found for machine {}", machine_num);
      return 1;
    }

    std::println("Machine {} requires {} presses", machine_num, *presses);
    total_presses += *presses;
  }

  std::println("Total button presses: {}", total_presses);
  return 0;
}
