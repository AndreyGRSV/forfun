#include <algorithm>
#include <charconv>
#include <cstdint>
#include <expected>
#include <fstream>
#include <iostream>
#include <ostream>
#include <print>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

int main() {

  std::ifstream input("../Day 6/input");
  if (!input.is_open()) {
    std::cerr << "Error opening input file" << std::endl;
    return 1;
  }

  // Read all lines from the input
  std::vector<std::vector<int>> d_groups{};
  std::vector<char> o_line{};
  std::string line;
  std::regex number_pattern{R"(\d{1,4})"};

  // Read all number rows
  while (getline(input, line)) {
    // Check if this is the operator line
    if (line.find('*') != std::string::npos ||
        line.find('+') != std::string::npos) {
      // Parse operators
      std::regex op_pattern{R"([+*])"};
      for (std::sregex_iterator it(line.begin(), line.end(), op_pattern), end;
           it != end; ++it) {
        o_line.push_back((*it).str()[0]);
      }
      break; // Stop reading after operator line
    }

    // Parse numbers from current row
    std::vector<int> current_row{};
    for (std::sregex_iterator it(line.begin(), line.end(), number_pattern), end;
         it != end; ++it) {
      current_row.push_back(std::stoll(it->str()));
    }
    if (!current_row.empty()) {
      if (d_groups.empty())
        d_groups.resize(current_row.size());
      for (const auto [idx, value] : current_row | std::views::enumerate) {
        d_groups[idx].push_back(value);
      }
    }
  }

  input.close();

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
