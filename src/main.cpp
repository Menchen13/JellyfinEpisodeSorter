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

Series promptUser(const std::vector<Series> &s);

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
    // consider making series Structs containing stuff like search term
    // Provider Url an parsing func
    episodeMap = fandomProvider("https://adventuretime.fandom.com/wiki/"
                                "List_of_episodes/Intended_Order",
                                adventureTime)
                     .getEpisodes();

    const std::string rawSeries = fetchSeriesRaw(url, title, apiKey);

    const std::vector<Series> seriesVec = parseJellyfinResponse<Series>(rawSeries);

    switch (seriesVec.size()) {
    case 0:
      std::print("No series found for searchterm.\n Perhaps a typo or you dont "
                 "actually have the series on jellyfin.");
      break;
    case 1:
      series = seriesVec[0];
      break;
    default:
      series = promptUser(seriesVec);
      break;
    }

    const std::string rawEpisodes = fetchEpisodesRaw(url, series.id, apiKey);

    const std::vector<Episode> episodeVec = parseJellyfinResponse<Episode>(rawEpisodes);

    for (const auto &e :episodeVec) {
      std::cout << e << "\n";
    }
    std::cout << std::endl;



  } catch (const json::exception &e) {
    std::print(stderr, "Json exception thrown: {}\n", e.what());
    return -2;
  } catch (const std::invalid_argument &e) {
    std::print(stderr, "{}\n", e.what());
    return -1;
  } catch (const std::runtime_error &e) {
    // thrown by fetch Series
    std::print(stderr, "Runtime_error!\n{}", e.what());
  }

  // helperPrintMaps(episodeMap);
  // std::cout << series << std::endl;

  // asynchronously get the titleframe for each episode and ocr it.
  // this one is gonna be tuff both have heavy IO delays
  //
  // Make Use of Cloud-API for ease and performance. Free tier has huge bottleneck tho
  // create a single function which will be run on multiple threads at once and each completes a whole OCR-Cycle
  // Use async and futures to manage easily

  // fire of the apiCalls to set episode titles with id to name matches

  return 0;
}

Series promptUser(const std::vector<Series> &s) {

  while (true) {
    std::println("Found multiple series matching searchterm.");
    for (const auto &[number, item] :
         std::views::zip(std::views::iota(1), s)) {
      std::println("{}: {}", number, item.name);
    }

    std::print("Please select the correct one: ");
    std::string input;
    std::getline(std::cin, input);
    int selection{};

    const auto [_, ec] =
        std::from_chars(input.data(), input.data() + input.size(), selection);

    if (ec == std::errc() && selection > 0 && selection <= s.size()) {
      println("Selected {}: {}", selection, s[selection - 1].name);
      return s.at(selection - 1);
    } else {
      std::println("Incorrect selection!");
    }
  }
}
