#pragma once

#include <charconv>
#include <concepts>
#include <expected>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <tuple>

template <typename T>
concept UnsignedInteger = std::unsigned_integral<T> && !std::same_as<T, bool>;

template <UnsignedInteger T>
std::tuple<T, bool> to_unsigned(std::string_view sval) {
  T value{};
  auto result = std::from_chars(sval.data(), sval.data() + sval.size(), value);
  return {value, result.ec == std::errc{} &&
                     (result.ptr == sval.data() + sval.size())};
}

template <typename ReturnType>
std::expected<ReturnType, bool> readFileByLine(
    const std::filesystem::path &file_name,
    const std::function<bool(std::string_view, ReturnType &accumulate)>
        &readbyline) {
  std::ifstream inputFile(file_name);
  if (inputFile.is_open()) {
    if (readbyline) {
      std::string line;
      ReturnType accumulate{};
      while (std::getline(inputFile, line))
        if (!readbyline(line, accumulate)) {
          return std::unexpected(false);
        }
      return accumulate;
    } else {
      return std::unexpected(false);
    }
  }
  return std::unexpected(false);
}
