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
  // perhaps make url and apiKey into a FACADE struct
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

    const std::vector<Series> seriesVec =
        parseJellyfinResponse<Series>(rawSeries);

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

    const std::vector<Episode> episodeVec =
        parseJellyfinResponse<Episode>(rawEpisodes);

    // asynchronously get the titleframe for each episode and ocr it.
    // this one is gonna be tuff both have heavy IO delays
    //
    // Make Use of Cloud-API for ease and performance. Free tier has huge
    // bottleneck tho create a single function which will be run on multiple
    // threads at once and each completes a whole OCR-Cycle Use async and
    // futures to manage easily
    // main manages task-count and delay based on if paid-tier api-key is specified
    // make use of expected values in threads to avoid exceptions crashing threads, or ending main function on single bad call
    //
    // in main:
    // std::expected<OcrResult, std::string> IdToTitle(std::string_view url, std::string_view apiKey, std::string_view id, std::string_view googleApiKey);
    // returns the Titlecard text for the episode passed by id. This is the threadfunction to be called in main
    // 
    // uses in order:
    // opencv return-type getTitlecard(std::string_view streamUrl, unsigned int targetSecond);
    // uses opencv to connect to the video url and only get the single frame for the titlecard. Am i correct in assuming this does not need cpr?
    //
    //
    // [opencv b64 in mem]-type processTitlecard(opencv return type& image);
    // takes a reference to the image does some math on it to prepare it for ocr and then encodes it in b64 for Google-OCR-API.
    // Two steps bunched together since they have no dependency to wait on anything.
    // Unit-Testable!
    //
    // std::string GoogleOCR(light parameter to b64 opencv obj. maybe string_view if possible, std::string_view googleApiKey);
    // calls google OCR-API and returns the string found in the image.
    // depending on the information the API can return (confidence values and what not) might make sense to include em in return and thread func return,
    // for processing in main.
    //
    // Finally return OcrResult is just string:jellyfinId and string::titlecardText
    //
    // if no paid-tier is provided main will create 15 tasks and start a 1minute timer and only start the next batch when timer is up to comply with
    // free tier.
    // If paid-tier is provided no timer is needed once a task is done a new one is started in its place. Might also be possible to up task number decently
    // as long as jellyfin can handle it - google should withstand virtually everthing

    // seperate step in main which can also do some error handeling - if the match fails or smt interact with user.
    // fuzzymatch the struct returned by OCR to list from Episode provider on name and on Episode structs by id.
    // Then there is id(EpisodeStruct)->name(OcrResult)->Season&Episode number(map from provider)
    // which can be used for last step

    // fire of the apiCalls to set episode titles with id to name matches

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

  return 0;
}

// can throw Json::exception type
Series promptUser(const std::vector<Series> &s) {

  while (true) {
    std::println("Found multiple series matching searchterm.");
    for (const auto &[number, item] : std::views::zip(std::views::iota(1), s)) {
      std::println("{}: {}", number, item.name);
    }

    std::print("Please select the correct one: ");
    std::string input;
    std::getline(std::cin, input);
    int selection{};

    const auto [_, ec] =
        std::from_chars(input.data(), input.data() + input.size(), selection);

    if (ec == std::errc() && selection > 0 && selection <= s.size()) {
      println("Selected {}: {}", selection, s.at(selection - 1).name);
      return s.at(selection - 1);
    } else {
      std::println("Incorrect selection!");
    }
  }
}
