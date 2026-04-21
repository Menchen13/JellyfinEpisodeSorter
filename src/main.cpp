#include <charconv>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <print>
#include <stdexcept>
#include <vector>

#include "FandomProvider.hpp"
#include "apiClient.hpp"
#include "models.hpp"
#include "parser.hpp"

using nlohmann::json;

int main() {
  std::println("Hello World");
  std::map<unsigned int, std::map<unsigned int, std::string>> episodeMap;

  // needs to be sanity checked before actual use
  std::string url = "https://jellyfin.root.adrean.eu";
  std::string apiKey = "c5d2356a93834aa5bba1de582eb1ecc0";
  std::string title = "Adventure Time";
  Series series;

  try {
    // Must be made to some sort of option based factory
    episodeMap = fandomProvider("https://adventuretime.fandom.com/wiki/"
                                "List_of_episodes/Intended_Order",
                                adventureTime)
                     .getEpisodes();

    std::string raw = fetchSeriesRaw(url, title, apiKey);

    std::vector<Series> seriesVec = parseSeriesList(raw);

    // turn into helper function to save space TODO
    switch (seriesVec.size()) {
    case 0:
      std::print("No series found for searchterm.\n Perhaps a typo or you dont "
                 "actually have the series on jellyfin.");
      break;
    case 1:
      series = seriesVec[0];
      break;
    default:
      while (true) {
        std::println("Found multiple series matching searchterm.");
        for (const auto &[number, item] :
             std::views::zip(std::views::iota(1), seriesVec)) {
          std::println("{}: {}", number, item.name);
        }

        std::print("Please select the correct one: ");
        std::string input;
        std::getline(std::cin, input);
        int selection{};

        const auto [_, ec] = std::from_chars(
            input.data(), input.data() + input.size(), selection);

        if (ec == std::errc() && selection > 0 &&
            selection <= seriesVec.size()) {
          println("Selected {}: {}", selection, seriesVec[selection - 1].name);
          series = seriesVec[selection - 1];
          break;
        } else {
          std::println("Incorrect selection!");
        }
      }
      break;
    }

  } catch (const json::exception &e) {
    std::print(stderr, "JSON Magic failed: {}\n", e.what());
    return -2;
  } catch (const std::invalid_argument &e) {
    std::print(stderr, "{}\n", e.what());
    return -1;
  } catch (const std::runtime_error &e) {
    // thrown by fetch Series
    std::print(stderr, "Runtime_error!\n{}", e.what());
  }

  // helperPrintMaps(episodeMap);
  std::cout << series << std::endl;

  // asynchronously get the titleframe for each episode and ocr it.
  // this one is gonna be tuff both have heavy IO delays

  // fire of the apiCalls to set episode titles with id to name matches

  return 0;
}
