#pragma once

#include <charconv>
#include <concepts>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <nlohmann/json.hpp>

std::string getSeriesId(const std::string_view &url,
                        const std::string_view &searchString,
                        const std::string_view &apiKey);

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

using nlohmann::json;

// returns the Jellyfin-ID for the series found with Search.
// if multiple entries are found it prompts for selection.
// *WARNING* can throw:
// parse_error.101 -> failed json parsing
// runtime_error -> no items in response or
//                  TotalRecordCount and size of item array dont match
// probably shouldnt catch any of em since they are fatal

template <JsonParsableInput T> std::string processSearch(T &&text) {
  json j = json::parse(std::forward<T>(text)); // can throw need to handle

  const int itemsFound = j["TotalRecordCount"];
  auto items = j["Items"];

  if (items.size() != itemsFound)
    throw std::runtime_error("TotalRecordCount and Items arraysize missmatch "
                             "in series search response - "
                             "this could result in undefined behavior!");

  switch (itemsFound) {
  case 0:
    throw std::runtime_error("Found no items with searchterm!");
    break;
  case 1:
    return items[0]["Id"];
    break;
  default:
    std::println("Found multiple series for searchterm:");
    for (const auto &[number, item] :
         std::views::zip(std::views::iota(1), items)) {
      println("{}: {}", number, item["Name"].get_ref<std::string &>());
    }

    while (true) {
      std::print("Please select the correct one: ");

      std::string input;
      std::getline(std::cin, input);
      int selection{};

      const auto [_, ec] =
          std::from_chars(input.data(), input.data() + input.size(), selection);

      if (ec == std::errc() && selection > 0 && selection <= itemsFound) {
        println("Selected {}: {}", selection,
                items[selection - 1]["Name"].get_ref<std::string &>());
        return items[selection - 1]["Id"];
      } else {
        std::println("Incorrect selection!");
      }
    }
    break;
  }

  // all switch branches return to throw
  std::unreachable();
}
