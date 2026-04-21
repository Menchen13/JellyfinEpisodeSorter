#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "models.hpp"

using nlohmann::json;

// concept matching json::parse template input from
// https://json.nlohmann.me/api/basic_json/parse/ to guard function parsing
// their input (All Gemini)
template <typename T>
concept JsonParsableInput =
    std::convertible_to<T, std::string_view> ||
    std::derived_from<std::remove_reference_t<T>, std::istream> ||
    requires(T t) {
      std::begin(t);
      std::end(t);
    };

// returns vector of series
// *WARNING* can throw:
// parse_error.101 -> failed json parsing

std::vector<Series> parseSeriesList(JsonParsableInput auto &&text) {
  json j = json::parse(
      std::forward<decltype(text)>(text)); // can throw need to handle

  auto items = j["Items"];

  std::vector<Series> series;
  series.reserve(items.size());

  for (const auto &item : items) {
    series.emplace_back(item["Id"], item["Name"]);
  }

  return series;
}

// IMPLEMENT
std::vector<Series> parseEpisodesList(JsonParsableInput auto &&Text);
