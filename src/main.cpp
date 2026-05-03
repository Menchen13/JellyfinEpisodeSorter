#include <algorithm>
#include <charconv>
#include <cstddef>
#include <exception>
#include <iostream>
#include <print>
#include <ranges>
#include <stdexcept>
#include <unistd.h>
#include <vector>

#include <nlohmann/json.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <rapidfuzz/fuzz.hpp>

#include "FandomProvider.hpp"
#include "apiClient.hpp"
#include "jesLog.hpp"
#include "models.hpp"
#include "ocr.hpp"
#include "parser.hpp"

using nlohmann::json;

JellyfinSeries promptUser(const std::vector<JellyfinSeries> &s);

int main() {
  std::println("Hello World");
  // set opencv log level to error to not clutter output
  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

  // needs to be sanity checked before actual use
  // perhaps make url and apiKey into a FACADE struct
  // maybe add gemini model to use as a parameter
  const std::string url = "https://jellyfin.root.adrean.eu";
  const std::string apiKey = "";
  const std::string title = "Adventure Time";
  const unsigned int targetSecond = 26;
  const std::string googleApiKey = "";
  // Should be put together from url and port. Endpoint is always the same, no
  // need for param TODO
  const std::string ollamaUrl =
      "http://host.docker.internal:11434/api/generate";
  const bool localLLM = false; // need this to be set after param parse so i can
                               // determine which lamda to set as ocrCallback
  // TODO check Jellyfin API-Key capabilties and exit if not enought permissions
  // TODO allow user to rase log-level with -v

  // lamdas for ocrCallback
  // these just wrap the base64ToTitle functions, making sure to
  // keeo ocrProvider signature. Not necessary rn since both have same sig,
  // but was fun learning about and could be good pattern if other OCR providers
  // need more params
  ocrProvider googleOCR = [&googleApiKey](const std::string &base64) {
    return GoogleOCR::base64ToTitle(base64, googleApiKey);
  };

  ocrProvider ollamaOCR = [&ollamaUrl](const std::string &base64) {
    return OllamaOCR::base64ToTitle(base64, ollamaUrl);
  };

  ocrProvider ocrCallback = ollamaOCR; // TODO to be set based on params

  JellyfinSeries series;

  try {
    // Must be made to some sort of option based factory
    // consider making series Structs containing stuff like search term
    // Provider Url and parsing func
    // TODO
    std::vector<Episode> episodesOrder =
        fandomProvider("https://adventuretime.fandom.com/wiki/"
                       "List_of_episodes/Intended_Order",
                       adventureTime)
            .getEpisodes();

    const std::string rawSeries = jellyfin::fetchSeriesRaw(url, title, apiKey);

    const std::vector<JellyfinSeries> seriesVec =
        parseJellyfinResponse<JellyfinSeries>(rawSeries);

    switch (seriesVec.size()) {
    case 0:
      JES_ERROR("No series found for searchterm.\n Perhaps a typo or you dont "
                "actually have the series on jellyfin.");
      return -4;
    case 1:
      series = seriesVec[0];
      break;
    default:
      series = promptUser(seriesVec);
      break;
    }

    const std::string rawEpisodes =
        jellyfin::fetchEpisodesRaw(url, series.id, apiKey);

    std::vector<JellyfinEpisode> episodeVec =
        parseJellyfinResponse<JellyfinEpisode>(rawEpisodes);

    // sort the episodes by id to make lookups way faster at the matching step
    std::ranges::sort(episodeVec, {}, &JellyfinEpisode::id);

    // testing episodeVec remove later TODO
    /* std::vector<Episode> episodeVec = {
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
*/
    JES_INFO("Starting OCR pipeline - this might take a while.");

    const std::vector<OcrResult> ocrResults =
        idToTitlePipeline(episodeVec, url, targetSecond, ocrCallback);

    // matching titles and building final JellyfinEpisodes
    for (const auto &ocr : ocrResults) {

      double bestScore{85.0};
      std::vector<size_t> bestIndexes{};

      for (const auto &[index, episode] :
           std::views::zip(std::views::iota(0), episodesOrder)) {
        // everything under 85.0 will return 0
        const double score = rapidfuzz::fuzz::token_set_ratio(
            ocr.extractedTitle, episode.title, 92.0);

        // new bestScore
        if (score > bestScore) {
          bestScore = score;
          bestIndexes.clear();
          bestIndexes.push_back(index);
        } else if (score == bestScore) {
          bestIndexes.push_back(index);
        }
      }

      if (bestIndexes.size() == 1) {
        // exact match
        // integrate order information into jellyfinEpisodes vector by matching
        // id TODO

        // remove matched episode from matching pool
        std::swap(episodesOrder.at(bestIndexes[0]), episodesOrder.back());
        episodesOrder.pop_back();
      } else if (bestIndexes.size() > 1) {
        // multiple match figure out what to do, probably promt user or smt
      } else {
        // no match bestIndexes is empty
        // figure out what to do, probably prompt user or smt
      }
    }

    // seperate step in main which can also do some error handeling - if the
    // match fails or smt interact with user. fuzzymatch the struct returned by
    // OCR to list from Episode provider on name and on Episode structs by id.
    // Then there is id(EpisodeStruct)->name(OcrResult)->Season&Episode
    // number(map from provider) which can be used for last step

    // fire of the apiCalls to set episode titles with id to name matches
  } catch (const json::exception &e) {
    JES_ERROR("Json exception thrown: {}\n", e.what());
    return -1;
  } catch (const std::runtime_error &e) {
    // thrown by fetch Series
    JES_ERROR("Runtime_error!\n{}", e.what());
    return -2;
  } catch (const std::exception &e) {
    JES_ERROR("Uncaught exception: {}\n", e.what());
    return -3;
  }

  return 0;
}

// can throw Json::exception type
JellyfinSeries promptUser(const std::vector<JellyfinSeries> &s) {

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
