#include <charconv>
#include <exception>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <print>
#include <ranges>
#include <stdexcept>
#include <unistd.h>
#include <vector>

#include "FandomProvider.hpp"
#include "apiClient.hpp"
#include "models.hpp"
#include "ocr.hpp"
#include "parser.hpp"

using nlohmann::json;

Series promptUser(const std::vector<Series> &s);

int main() {
  std::println("Hello World");
  std::map<unsigned int, std::map<unsigned int, std::string>> episodeMap;

  // needs to be sanity checked before actual use
  // perhaps make url and apiKey into a FACADE struct
  // maybe add gemini model to use as a parameter
  const std::string url = "https://jellyfin.root.adrean.eu";
  const std::string apiKey = "";
  const std::string title = "Adventure Time";
  const unsigned int targetSecond = 26;
  const std::string googleApiKey = "";
  const bool localLLM = false; // need this to be set after param parse so i can
                               // determine which lamda to set as ocrCallback
  // TODO check Jellyfin API-Key capabilties and exit if not enought permissions

  // lamdas for ocrCallback
  ocrProvider googleOCR = [&googleApiKey](const std::string &base64) {
    return GoogleOCR::base64ToTitle(base64, googleApiKey);
  };

  ocrProvider localOCR; // TODO

  ocrProvider ocrCallback = googleOCR; // TODO to be set based on params

  Series series;

  try {
    // Must be made to some sort of option based factory
    // consider making series Structs containing stuff like search term
    // Provider Url and parsing func
    // TODO
    /* episodeMap = fandomProvider("https://adventuretime.fandom.com/wiki/"
                                "List_of_episodes/Intended_Order",
                                adventureTime)
                     .getEpisodes();

    const std::string rawSeries = jellyfin::fetchSeriesRaw(url, title, apiKey);

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

    const std::string rawEpisodes =
        jellyfin::fetchEpisodesRaw(url, series.id, apiKey);

    const std::vector<Episode> episodeVec =
        parseJellyfinResponse<Episode>(rawEpisodes); */

    std::vector<Episode> episodeVec = {
        {"764d68737212c54fec655863ae9c96ed", "", ""},
        {"f962d726e9adeb2d2bb6f5e415eb828c", "", ""},
        {"5fff254f25ee6ef4029123225fd5bb63", "", ""},
        {"552ee07428469f3c19ec75a0cb3b2861", "", ""},
        {"d55a152e6620a34f106b3886b74d7806", "", ""},
        {"b4b5490f7c93b0a94d47171d29f3eeb8", "", ""},
        {"2ce3cca5a9de4b23336402fd37d60df6", "", ""},
        {"dd7ccd518cd8cc06e402cc32fd5c7faf", "", ""},
        {"a587cac83aae594541be3c706813e527", "", ""},
        {"166511186ee7545605ff3619823d4c70", "", ""},
        {"e69a1a0b30e512eae5eb5e82dfa7d707", "", ""}};

    std::println("Starting OCR pipeline - this might take a while.");

    const std::vector<OcrResult> ocrResults =
        idToTitlePipeline(episodeVec, url, targetSecond, ocrCallback);

    for (const auto &o : ocrResults){
      std::cout << o << "\n";
    }
    std::cout << std::endl;

    // pass the Episode vector and filled function-obj to a pipeline function or
    // lambda which returns a vector of OCR results This function encapsulates
    // the OCR and error-handeling for it, not the matching - this is still in
    // main.
    //
    // In this function run through the pipeline using the function-obj as a
    // callback to contact the API for OCR. Have this be wrapped in try catch
    // and catch a special exception-obj thrown by googleAPI-Callback function
    // when limit is reached. If this happends just wait a minute and restart,
    // that way no matter which tier the user api-key has he will get the
    // maximum rate. problem with this is that local function only needs one
    // param and google needs two with the api key and this makes the functions
    // very stiff...

    // seperate step in main which can also do some error handeling - if the
    // match fails or smt interact with user. fuzzymatch the struct returned by
    // OCR to list from Episode provider on name and on Episode structs by id.
    // Then there is id(EpisodeStruct)->name(OcrResult)->Season&Episode
    // number(map from provider) which can be used for last step

    // fire of the apiCalls to set episode titles with id to name matches

  } catch (const json::exception &e) {
    std::print(stderr, "Json exception thrown: {}\n", e.what());
    return -1;
  } catch (const std::runtime_error &e) {
    // thrown by fetch Series
    std::print(stderr, "Runtime_error!\n{}", e.what());
    return -2;
  } catch (const std::exception &e) {
    std::print(stderr, "Uncaught exception: {}\n", e.what());
    return -3;
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
