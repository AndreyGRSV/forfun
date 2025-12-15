/*
 * Day 11: Signal Pathways - Part 1
 * "Day 11: Reactor"
 * Count distinct paths from 'you' to 'out', avoiding cycles
 * Expected output: 701
 */
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stack>
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

// Optimized recursive DFS with local cycle tracking
Count dfs_count(
    const std::string &node,
    const std::unordered_map<std::string, std::vector<std::string>> &g,
    std::unordered_map<std::string, Count> &memo,
    std::unordered_set<std::string> &visiting) {
  if (node == "out")
    return 1;
  if (visiting.find(node) != visiting.end())
    return 0; // cycle detected
  auto mit = memo.find(node);
  if (mit != memo.end())
    return mit->second;
  auto git = g.find(node);
  if (git == g.end())
    return 0;

  visiting.insert(node);
  Count sum = 0;
  for (const auto &nbr : git->second) {
    sum += dfs_count(nbr, g, memo, visiting);
  }
  visiting.erase(node);
  memo[node] = sum;
  return sum;
}

int main(int argc, char **argv) {
  std::filesystem::path input_path =
      (argc > 1) ? argv[1] : std::filesystem::path("../Day 11/input");

  using Graph = std::unordered_map<std::string, std::vector<std::string>>;
  auto res = puzzles::common::readFileByLine<Graph>(
      input_path, [](std::string_view line, Graph &g) -> bool {
        // skip empty lines
        std::string_view s = line;
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
          s.remove_suffix(1);
        if (s.empty())
          return true;
        // format: name: out1 out2 ...
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
  std::unordered_map<std::string, Count> memo;
  std::unordered_set<std::string> visiting;
  Count answer = dfs_count("you", g, memo, visiting);

  std::cout << to_dec(answer) << '\n';
  return 0;
}
