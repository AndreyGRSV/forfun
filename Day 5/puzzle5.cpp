/*
 * Puzzle solution for Advent of Code 2025 - Day 5
 * "Day 5: Cafeteria"
 * Problem: Playground - Fresh Ingredient Ranges
 * Determine how many available ingredient IDs fall within given fresh ranges,
 * and the total count of fresh ingredient IDs after merging overlapping ranges.
 * Expected output: 529 344260049617193
 */
#include "../common/common.h"
#include <algorithm>
#include <expected>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace pc = puzzles::common;
struct Range {
  uint64_t start;
  uint64_t end;

  bool contains(uint64_t id) const { return id >= start && id <= end; }

  // Check if ranges are completely separated (have a gap)
  bool rangesAreSeparated(const Range &other) const {
    return end + 1 < other.start || other.end + 1 < start;
  }

  // Check if this range overlaps or is adjacent to another range
  bool overlapsOrAdjacent(const Range &other) const {
    return !rangesAreSeparated(other);
  }

  // Merge this range with another overlapping range
  Range merge(const Range &other) const {
    return {std::min(start, other.start), std::max(end, other.end)};
  }

  // Count of IDs in this range
  uint64_t count() const { return end - start + 1; }

  // For sorting
  auto operator<=>(const Range &other) const = default;
};

std::expected<Range, std::string_view> parseLine(std::string_view line) {

  if (line.empty()) {
    return std::unexpected("Empty line");
  }
  auto dash_pos = line.find('-');
  if (dash_pos == std::string::npos) {
    return std::unexpected(std::format("Invalid range format: {}", line));
  }

  auto start = pc::to_unsigned<uint64_t>(line.substr(0, dash_pos));
  auto end = pc::to_unsigned<uint64_t>(line.substr(dash_pos + 1));

  if (!start || !end) {
    return std::unexpected(std::format("Error parsing range: {}", line));
  }

  if (*start > *end) {
    return std::unexpected(
        std::format("Invalid range (start > end): {}", line));
  }
  return Range{*start, *end};
}

auto mergeRanges(std::vector<Range> ranges) -> std::vector<Range> {
  if (ranges.empty()) {
    return {};
  }

  std::ranges::sort(ranges);

  return std::ranges::fold_left(
      ranges | std::views::drop(1), std::vector<Range>{ranges[0]},
      [](std::vector<Range> merged, const Range &current) {
        Range &last = merged.back();

        if (last.overlapsOrAdjacent(current)) {
          // Merge with the last range
          last = last.merge(current);
        } else {
          // Add as a new separate range
          merged.push_back(current);
        }

        return merged;
      });
}

auto countFreshIngredients(const std::vector<Range> &ranges) -> uint64_t {
  return std::ranges::fold_left(
      ranges, 0ULL,
      [](uint64_t accum, const Range &range) { return accum + range.count(); });
}

int main(int argc, char *argv[]) {
  const std::filesystem::path input_file =
      (argc > 1) ? argv[1] : "../Day 5/input";

  using RangesType = std::vector<Range>;
  using IDsType = std::vector<uint64_t>;

  IDsType available_ids{};

  enum class ReadState { Ranges, IDs };
  ReadState state = ReadState::Ranges;

  auto result = pc::readFileByLine<RangesType>(
      input_file,
      [&available_ids, &state](std::string_view line, RangesType &accumulate) {
        if (line.empty()) {
          // Switch to reading available IDs
          state = ReadState::IDs;
          return true;
        }
        if (state == ReadState::IDs) {
          auto id = pc::to_unsigned<uint64_t>(line);
          if (!id) {
            std::println(stderr, "Error parsing ID: {}", line);
            return false;
          }
          available_ids.push_back(*id);
          return true;
        } else {
          auto parse_result = parseLine(line);
          if (!parse_result) {
            std::println(stderr, "Error: {}", parse_result.error());
            return false;
          }
          accumulate.push_back(*parse_result);
        }
        return true;
      });

  if (!result) {
    std::println(stderr, "Error reading file {}", input_file.string());
    return 1;
  }

  const auto &ranges = result.value();

  auto fresh_count =
      std::ranges::count_if(available_ids, [ranges](uint64_t id) {
        return std::ranges::any_of(
            ranges, [id](const Range &range) { return range.contains(id); });
      });

  // Process using functional pipeline
  auto ranges_copy = std::move(result.value());
  auto merged_ranges = mergeRanges(std::move(ranges_copy));
  auto total_fresh = countFreshIngredients(merged_ranges);

  std::println("{} {}", fresh_count, total_fresh);

  return 0;
}
