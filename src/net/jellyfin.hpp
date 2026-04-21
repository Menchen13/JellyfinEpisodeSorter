#pragma once

#include <charconv>
#include <concepts>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

#include <nlohmann/json.hpp>
namespace net {

std::expected<std::string, std::string>
getSeriesId(std::string_view &url, std::string_view &searchString,
            std::string_view &apiKey);
}

// concept matching json::parse template input from https://json.nlohmann.me/api/basic_json/parse/
// to guard function parsing their input
// (All Gemini)
template <typename T>
concept JsonCompatibleInput =
    std::convertible_to<T, std::string_view> ||
    std::derived_from<std::remove_reference<T>, std::istream> || requires(T t) {
      std::begin(t);
      std::end(t);
    };

using nlohmann::json;

template <typename JsonCompatipleInput>
std::string processSearch (JsonCompatipleInput&& text){
  json j = json::parse(std::forward<JsonCompatipleInput>(text)); // can throw need to handle

  int itemsFound = j["TotalRecordCount"];
  auto items = j["Items"];

  switch (itemsFound) {
  case 0:
    // nothing found with the searchterm. handle error
    std::println("processSearch case 0: Not implemented yet!!!");
    exit(-3);
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

      auto [_, ec] =
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
