/*
 * Puzzle solution for Advent of Code 2025 - Day 8
 * Problem: Playground - Junction Box Circuits
 *
 * Connect junction boxes in 3D space by shortest distances.
 * Use Union-Find to track circuits and find the product of the three largest.
 */

#include "../common/common.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

struct Point3D {
  int x, y, z;
};

struct Edge {
  int from, to;
  double distance;

  bool operator<(const Edge &other) const { return distance < other.distance; }
};

// Union-Find (Disjoint Set Union) data structure
class UnionFind {
private:
  std::vector<int> parent;
  std::vector<int> size;
  int num_components;

  int find(int x) {
    if (parent[x] != x) {
      parent[x] = find(parent[x]); // Path compression
    }
    return parent[x];
  }

public:
  ~UnionFind() = default;
  UnionFind(const UnionFind &) = delete;
  UnionFind(UnionFind &&) = default;
  UnionFind &operator=(const UnionFind &) = delete;
  UnionFind &operator=(UnionFind &&) = default;

  UnionFind(int n) : parent(n), size(n, 1), num_components(n) {
    for (int i = 0; i < n; i++) {
      parent[i] = i;
    }
  }

  bool unite(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);

    if (rootX == rootY) {
      return false; // Already in same set
    }

    // Union by size
    if (size[rootX] < size[rootY]) {
      parent[rootX] = rootY;
      size[rootY] += size[rootX];
    } else {
      parent[rootY] = rootX;
      size[rootX] += size[rootY];
    }

    return true;
  }

  int getSize(int x) { return size[find(x)]; }

  std::vector<int> getAllSizes() {
    std::vector<int> result;
    for (int i = 0; i < parent.size(); i++) {
      if (find(i) == i) { // Root of a component
        result.push_back(size[i]);
      }
    }
    return result;
  }

  int getNumComponents() const { return num_components; }

  bool isFullyConnected() const { return num_components == 1; }
};

double calculateDistance(const Point3D &a, const Point3D &b) {
  uint64_t dx = static_cast<uint64_t>(a.x) - b.x;
  uint64_t dy = static_cast<uint64_t>(a.y) - b.y;
  uint64_t dz = static_cast<uint64_t>(a.z) - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

int main(int argc, char *argv[]) {
  const std::filesystem::path input_file{(argc > 1) ? argv[1]
                                                    : "../Day 8/input"};
  const int TARGET_CONNECTIONS = (argc > 2) ? std::stoi(argv[2]) : 1000;

  // Read junction box positions
  using ResultType = std::vector<Point3D>;
  auto result_boxes = puzzles::common::readFileByLine<ResultType>(
      input_file, [](std::string_view line, ResultType &boxes) -> bool {
        if (line.empty())
          return false;

        std::istringstream iss(line.data());
        Point3D point;
        char comma;

        if (iss >> point.x >> comma >> point.y >> comma >> point.z) {
          boxes.push_back(point);
        }
        return true;
      });

  if (!result_boxes) {
    std::println(stderr, "Error reading input file");
    return 1;
  }

  const auto &boxes = result_boxes.value();

  int n = boxes.size();

  // Generate all possible edges with distances
  std::vector<Edge> edges;
  for (const auto &[idx1, box1] : boxes | std::views::enumerate) {
    for (const auto &[idx2, box2] :
         boxes | std::views::enumerate | std::views::drop(idx1 + 1)) {
      edges.emplace_back(Edge{.from = static_cast<int>(idx1),
                              .to = static_cast<int>(idx2),
                              .distance = calculateDistance(box1, box2)});
    }
  }

  // Sort edges by distance (shortest first)
  std::sort(edges.begin(), edges.end());

  UnionFind uf(n);

  // ========== Part 1 ==========
  // Use Union-Find to connect boxes
  for (const auto &edge : edges | std::views::take(TARGET_CONNECTIONS)) {
    uf.unite(edge.from, edge.to); // Try to unite, even if already connected
  }

  // Get all circuit sizes
  std::vector<int> circuit_sizes = uf.getAllSizes();

  // Sort in descending order to find the three largest
  std::sort(circuit_sizes.begin(), circuit_sizes.end(), std::greater<int>());

  // Calculate product of three largest circuits
  uint64_t result = std::ranges::fold_left(circuit_sizes | std::views::take(3),
                                           1ULL, std::multiplies<uint64_t>{});

  std::println("Part 1: {}", result);

  // ========== Part 2 ==========
  int last_from = -1, last_to = -1;

  for (const auto &edge : edges) {
    if (uf.unite(edge.from, edge.to)) {
      last_from = edge.from;
      last_to = edge.to;

      if (uf.isFullyConnected()) {
        break;
      }
    }
  }

  // Calculate product of X coordinates of the last connection
  if (last_from != -1 && last_to != -1) {
    int64_t result = static_cast<int64_t>(boxes[last_from].x) *
                     static_cast<int64_t>(boxes[last_to].x);
    std::println("Part 2: {}", result);
  } else {
    std::println(stderr, "Error: Could not find last connection");
    return 1;
  }

  return 0;
}
