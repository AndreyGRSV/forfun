/*
 * Day 11: Signal Pathways - Part 2
 * "Day 11: Reactor"
 * Count distinct paths from 'svr' to 'out' that pass through both 'dac' and
 * 'fft' Expected output: 390108778818526
 */
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../common/common.h"

using Count = unsigned __int128;

static std::string to_dec(Count v) {
  if (v == 0)
    return "0";
  std::string s;
  while (v > 0) {
    int d = static_cast<int>(v % 10);
    s.push_back(char('0' + d));
    v /= 10;
  }
  std::reverse(s.begin(), s.end());
  return s;
}

// mask bits: bit0 = saw_dac, bit1 = saw_fft
Count dfs_masked(
    const std::string &node,
    const std::unordered_map<std::string, std::vector<std::string>> &g,
    int mask, std::unordered_set<std::string> &onpath,
    std::unordered_map<std::string, std::array<Count, 4>> &memo) {
  if (node == "out") {
    return (mask == 3) ? 1 : 0;
  }

  if (onpath.find(node) != onpath.end())
    return 0; // break cycles

  auto &memo_row = memo[node];
  if (memo_row[mask] != static_cast<Count>(-1))
    return memo_row[mask];

  auto git = g.find(node);
  if (git == g.end()) {
    memo_row[mask] = 0;
    return 0;
  }

  onpath.insert(node);
  Count sum = 0;
  for (const auto &nbr : git->second) {
    int next_mask = mask;
    if (nbr == "dac")
      next_mask |= 1;
    if (nbr == "fft")
      next_mask |= 2;
    sum += dfs_masked(nbr, g, next_mask, onpath, memo);
  }
  onpath.erase(node);

  memo_row[mask] = sum;
  return sum;
}

int main(int argc, char **argv) {
  std::filesystem::path input_path =
      (argc > 1) ? argv[1] : std::filesystem::path("../Day11/input");

  using Graph = std::unordered_map<std::string, std::vector<std::string>>;
  auto res = puzzles::common::readFileByLine<Graph>(
      input_path, [](std::string_view line, Graph &g) -> bool {
        std::string_view s = line;
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
          s.remove_suffix(1);
        if (s.empty())
          return true;
        size_t colon = s.find(':');
        if (colon == std::string_view::npos)
          return true;
        std::string name(std::string(s.substr(0, colon)));
        std::string_view rest = s.substr(colon + 1);
        std::vector<std::string> outs;
        std::istringstream iss{std::string(rest)};
        std::string tok;
        while (iss >> tok)
          outs.push_back(tok);
        g[name] = std::move(outs);
        return true;
      });

  if (!res) {
    std::cerr << "Failed to open input: " << input_path << '\n';
    return 2;
  }

  Graph g = *res;
  std::unordered_set<std::string> onpath;
  std::unordered_map<std::string, std::array<Count, 4>> memo;

  // initialize memo with -1 sentinel
  for (const auto &p : g)
    memo[p.first].fill(static_cast<Count>(-1));
  // ensure special nodes exist in memo map even if no outgoing edges
  memo["out"].fill(static_cast<Count>(-1));

  Count answer = dfs_masked("svr", g, 0, onpath, memo);

  std::cout << to_dec(answer) << '\n';
  return 0;
}
