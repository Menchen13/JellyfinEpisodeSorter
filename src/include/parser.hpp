#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "models.hpp"

using nlohmann::json;

// concept matching json::parse template input from
// https://json.nlohmann.me/api/basic_json/parse/ to guard functions parsing
// their input (All Gemini)
template <typename T>
concept JsonParsableInput =
    std::convertible_to<T, std::string_view> ||
    std::derived_from<std::remove_reference_t<T>, std::istream> ||
    requires(T t) {
      std::begin(t);
      std::end(t);
    };

// concept for my own structs
template <typename T>
concept MediaItem = std::same_as<T, Series> || std::same_as<T, Episode>;

// returns vector of jellyfin structs
// initalises Episode.season and Episode.episodeNumber to 0
// *WARNING* Can throw some sort of json::exception
template <MediaItem T>
std::vector<T> parseJellyfinResponse(JsonParsableInput auto &&text) {
  json j = json::parse(std::forward<decltype(text)>(text));

  // return vector created from Items array via nlohmann::json api
  return j.at("Items").get<std::vector<T>>();
}
