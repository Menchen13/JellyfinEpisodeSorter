#include "providers/FandomProvider.hpp"
#include <nlohmann/json.hpp>
#include <print>

using nlohmann::json;

int main() {
  std::println("Hello World");
  std::map<unsigned int, std::map<unsigned int, std::string>> episodeMap;

  try {
    fandomProvider tmp(
        "https://adventuretime.fandom.com/wiki/List_of_episodes/Intended_Order",
        adventureTime);
    episodeMap = tmp.getEpisodes();

  } catch (const json::exception &e) {
    std::print(stderr, "JSON Magic failed: {}\n", e.what());
    return -2;
  } catch (const std::invalid_argument &e) {
    std::print(stderr, "{}\n", e.what());
    return -1;
  }

  helperPrintMaps(episodeMap);
  return 0;
}
