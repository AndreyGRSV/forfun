/*
 * Day 12: Shape Packing - Part 1
 * "Day 12: Christmas Tree Farm"
 * Count how many regions can be exactly packed with given shapes
 * Expected output: 490
 */
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <print>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "../common/common.h"

namespace {
using Coord = std::pair<int64_t, int64_t>;

std::vector<std::string> trim_empty_tail(std::vector<std::string> s) {
  while (!s.empty() && s.back().empty())
    s.pop_back();
  return s;
}

std::vector<std::vector<std::string>>
parse_shapes(const std::vector<std::string> &lines) {
  std::vector<std::vector<std::string>> shapes;
  size_t i = 0;
  while (i < lines.size()) {
    std::string line = lines[i];
    if (line.empty()) {
      ++i;
      continue;
    }
    // shape header like "0:" or "5:"
    bool header = false;
    for (char c : line)
      if (c == ':')
        header = true;
    if (!header)
      break;
    // skip header
    ++i;
    std::vector<std::string> block;
    while (i < lines.size() && !lines[i].empty()) {
      block.push_back(lines[i]);
      ++i;
    }
    shapes.push_back(trim_empty_tail(block));
  }
  return shapes;
}

std::vector<std::tuple<int, int, std::vector<int>>>
parse_regions(const std::vector<std::string> &lines) {
  std::vector<std::tuple<int, int, std::vector<int>>> regions;
  for (auto &ln : lines) {
    if (ln.empty())
      continue;
    // match WxH: counts...
    auto pos = ln.find(":");
    if (pos == std::string::npos)
      continue;
    std::string lhs = ln.substr(0, pos);
    std::string rhs = ln.substr(pos + 1);
    auto xpos = lhs.find('x');
    if (xpos == std::string::npos)
      continue;
    int W = std::stoi(lhs.substr(0, xpos));
    int H = std::stoi(lhs.substr(xpos + 1));
    // parse counts
    std::vector<int> counts;
    {
      std::istringstream iss(rhs);
      int v;
      while (iss >> v)
        counts.push_back(v);
    }
    regions.emplace_back(W, H, counts);
  }
  return regions;
}

std::vector<std::string> rotate90(const std::vector<std::string> &g) {
  int h = (int)g.size();
  int w = h ? (int)g[0].size() : 0;
  std::vector<std::string> out(w, std::string(h, '.'));
  for (int r = 0; r < h; ++r)
    for (int c = 0; c < w; ++c)
      out[c][h - 1 - r] = g[r][c];
  return out;
}
std::vector<std::string> flip_h(const std::vector<std::string> &g) {
  auto out = g;
  for (auto &row : out)
    std::reverse(row.begin(), row.end());
  return out;
}

std::vector<Coord> coords_from_grid(const std::vector<std::string> &g) {
  std::vector<Coord> res;
  for (int r = 0; r < (int)g.size(); ++r)
    for (int c = 0; c < (int)g[r].size(); ++c)
      if (g[r][c] == '#')
        res.emplace_back(c, r);
  return res;
}

std::vector<Coord> normalize_coords(std::vector<Coord> v) {
  if (v.empty())
    return v;
  int64_t minx = v[0].first, miny = v[0].second;
  for (auto &p : v) {
    minx = std::min(minx, p.first);
    miny = std::min(miny, p.second);
  }
  for (auto &p : v) {
    p.first -= minx;
    p.second -= miny;
  }
  std::sort(v.begin(), v.end());
  return v;
}

std::vector<std::vector<Coord>>
generate_orientations(const std::vector<std::string> &shape) {
  std::vector<std::vector<Coord>> out;
  std::vector<std::string> g = shape;
  for (int r = 0; r < 4; ++r) {
    auto coords = normalize_coords(coords_from_grid(g));
    out.push_back(coords);
    auto fh = flip_h(g);
    out.push_back(normalize_coords(coords_from_grid(fh)));
    g = rotate90(g);
  }
  // unique
  std::vector<std::vector<Coord>> uniq;
  std::set<std::vector<Coord>> seen;
  for (auto &v : out) {
    if (seen.insert(v).second)
      uniq.push_back(v);
  }
  return uniq;
}

// Attempt to place all pieces using bitmask (if W*H<=64) or board array
// fallback
bool can_pack_region(
    int W, int H,
    const std::vector<std::vector<std::vector<Coord>>> &shape_orients,
    const std::vector<int> &piece_list) {
  int cells = W * H;
  bool use_mask = (cells <= 64);

  // precompute placements per shape index
  int S = (int)shape_orients.size();
  std::vector<std::vector<uint64_t>> placements_mask(S);
  std::vector<std::vector<std::vector<int>>> placements_vec(S);

  for (int s = 0; s < S; ++s) {
    auto &orients = shape_orients[s];
    std::vector<uint64_t> pm;
    std::vector<std::vector<int>> pv;
    pv.reserve(cells);
    pm.reserve(cells);
    for (auto &coords : orients) {
      int maxx = 0, maxy = 0;
      for (auto &c : coords) {
        maxx = std::max<int64_t>(maxx, c.first);
        maxy = std::max<int64_t>(maxy, c.second);
      }
      if (maxx >= W || maxy >= H)
        continue;
      for (int oy = 0; oy + maxy < H; ++oy)
        for (int ox = 0; ox + maxx < W; ++ox) {
          if (use_mask) {
            uint64_t m = 0;
            for (auto &c : coords) {
              int pos = (oy + c.second) * W + (ox + c.first);
              m |= (uint64_t(1) << pos);
            }
            pm.push_back(m);
          } else {
            std::vector<int> posv;
            posv.reserve(coords.size());
            for (auto &c : coords)
              posv.push_back((oy + c.second) * W + (ox + c.first));
            pv.push_back(posv);
          }
        }
    }
    placements_mask[s] = std::move(pm);
    placements_vec[s] = std::move(pv);
    // if no placements for a shape at all, early fail
    if ((use_mask && placements_mask[s].empty()) ||
        (!use_mask && placements_vec[s].empty()))
      return false;
  }

  // build pieces order by placements count (most constrained first)
  int N = (int)piece_list.size();
  std::vector<int> idxs(N);
  for (int i = 0; i < N; ++i)
    idxs[i] = i;
  std::vector<int> shape_for_piece(N);
  for (int i = 0; i < N; ++i)
    shape_for_piece[i] = piece_list[i];
  std::sort(idxs.begin(), idxs.end(), [&](int a, int b) {
    int sa = shape_for_piece[a], sb = shape_for_piece[b];
    int ca = use_mask ? (int)placements_mask[sa].size()
                      : (int)placements_vec[sa].size();
    int cb = use_mask ? (int)placements_mask[sb].size()
                      : (int)placements_vec[sb].size();
    return ca < cb; // ascending (fewest options first)
  });
  std::vector<int> order(N);
  for (int i = 0; i < N; ++i)
    order[i] = shape_for_piece[idxs[i]];

  if (use_mask) {
    uint64_t used = 0;
    std::function<bool(int)> dfs = [&](int pos) -> bool {
      if (pos == N)
        return true;
      int s = order[pos];
      auto &cand = placements_mask[s];
      for (auto m : cand) {
        if ((m & used) == 0) {
          used |= m;
          if (dfs(pos + 1))
            return true;
          used ^= m;
        }
      }
      return false;
    };
    return dfs(0);
  } else {
    std::vector<char> used(cells, 0);
    std::function<bool(int)> dfs = [&](int pos) -> bool {
      if (pos == N)
        return true;
      int s = order[pos];
      auto &cand = placements_vec[s];
      for (auto &pv : cand) {
        bool ok = true;
        for (int p : pv)
          if (used[p]) {
            ok = false;
            break;
          }
        if (!ok)
          continue;
        for (int p : pv)
          used[p] = 1;
        if (dfs(pos + 1))
          return true;
        for (int p : pv)
          used[p] = 0;
      }
      return false;
    };
    return dfs(0);
  }
}
} // namespace

int main(int argc, char **argv) {
  const std::filesystem::path input_file{(argc > 1) ? argv[1]
                                                    : "../Day12/input"};
  namespace pc = puzzles::common;
  // Read whole file into lines
  auto result = pc::readFileByLine<std::vector<std::string>>(
      input_file, [](std::string_view line, std::vector<std::string> &acc) {
        acc.emplace_back(line);
        return true;
      });
  if (!result) {
    std::println(stderr, pc::InputFileError);
    return 2;
  }
  const auto &lines = *result;

  // separate into shape section and region section: find first region line
  // containing 'x' and ':'
  size_t split = 0;
  for (size_t i = 0; i < lines.size(); ++i) {
    auto ln = lines[i];
    // trim
    auto s = ln;
    if (s.find('x') != std::string::npos && s.find(':') != std::string::npos) {
      split = i;
      break;
    }
  }

  // collect shape lines from top until split
  std::vector<std::string> shape_lines(lines.begin(), lines.begin() + split);
  std::vector<std::string> region_lines(lines.begin() + split, lines.end());

  auto shapes_grid = parse_shapes(shape_lines);
  auto regions = parse_regions(region_lines);

  int fit_count = 0;
  for (auto &r : regions) {
    int W, H;
    std::vector<int> counts;
    std::tie(W, H, counts) = r;
    // expand shapes into piece list
    int S = (int)shapes_grid.size();
    std::vector<int> pieces;
    int total_cells = 0;
    for (int s = 0; s < S; ++s) {
      // count '#' in shape
      int c = 0;
      for (auto &row : shapes_grid[s])
        for (char ch : row)
          if (ch == '#')
            ++c;
      for (int k = 0; k < (int)counts.size() && k <= s; ++k) {
      } // noop
      int need = (s < (int)counts.size()) ? counts[s] : 0;
      for (int i = 0; i < need; ++i) {
        pieces.push_back(s);
        total_cells += c;
      }
    }
    if (total_cells > W * H) { /* impossible */
      continue;
    }

    // precompute orientations for each shape
    std::vector<std::vector<std::vector<Coord>>> shape_orients;
    for (auto &g : shapes_grid)
      shape_orients.push_back(generate_orientations(g));

    bool ok = can_pack_region(W, H, shape_orients, pieces);
    if (ok)
      ++fit_count;
  }

  std::println("{}", fit_count);
  return 0;
}
