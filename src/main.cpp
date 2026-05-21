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

struct MatchResult {
  std::vector<const OcrResult *> revisit;
  std::vector<const OcrResult *> unmatched;
};

JellyfinSeries promptUser(const std::vector<JellyfinSeries> &s);
MatchResult processFuzzyMatches(const std::vector<const OcrResult *> &ocrBatch,
                                std::vector<Episode> &episodesOrder,
                                std::vector<JellyfinEpisode> &episodeVec);

int main() {
  std::println("Hello World");
  // set opencv log level to error to not clutter output
  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

  // needs to be sanity checked before actual use
  // perhaps make url and apiKey into a FACADE struct
  // maybe add gemini model to use as a parameter
  const std::string jellyfinUrl = "https://jellyfin.root.adrean.eu";
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
  // keep ocrProvider signature. Not necessary rn since both have same sig,
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

    const std::string rawSeries =
        jellyfin::fetchSeriesRaw(jellyfinUrl, title, apiKey);

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
        jellyfin::fetchEpisodesRaw(jellyfinUrl, series.id, apiKey);

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
        idToTitlePipeline(episodeVec, jellyfinUrl, targetSecond, ocrCallback);

    // prepare initial batch of pointers
    // transform creates a lazy view of the transformed elements
    // and ranges to created a new vector from it in a single step
    std::vector<const OcrResult *> currentBatch =
        ocrResults |
        std::views::transform([](const auto &ocr) { return &ocr; }) |
        std::ranges::to<std::vector<const OcrResult *>>();

    // matching loop
    MatchResult currentResult;
    bool keepRunning{true};
    while (keepRunning) {
      size_t previousRevisitSize = currentBatch.size();

      // Run the matching function
      currentResult =
          processFuzzyMatches(currentBatch, episodesOrder, episodeVec);

      // If there's nothing to revisit, we are completely done!
      if (currentResult.revisit.empty()) {
        break;
      }

      // If the revisit list size didn't shrink, it means removing the
      // exact matches didn't help resolve the ambiguities. We are stuck.
      if (currentResult.revisit.size() == previousRevisitSize) {
        JES_ERROR("Stalemate reached. Cannot resolve {} uncertain matches.",
                  currentResult.revisit.size());
        break;
      }

      // Otherwise, the pool shrunk! Feed the unresolved items back in for
      // another pass.
      JES_INFO("Re-evaluating {} inconclusive matches with narrowed pool...",
               currentResult.revisit.size());
      currentBatch = currentResult.revisit;
    } // while (keeprunning)

    // Inform user about failure to match everything.
    if (!currentResult.unmatched.empty() || !currentResult.revisit.empty()) {
      const auto deref = std::views::transform(
          [](const OcrResult *ptr) -> const OcrResult & { return *ptr; });

      std::println("Requires manual intervention for {} unmatched "
                   "OcrResults!\nDumping...",
                   currentResult.unmatched.size() +
                       currentResult.revisit.size());
      std::println("{}\n{}", currentResult.unmatched | deref,
                   currentResult.revisit | deref);
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

MatchResult processFuzzyMatches(const std::vector<const OcrResult *> &ocrBatch,
                                std::vector<Episode> &episodesOrder,
                                std::vector<JellyfinEpisode> &episodeVec) {
  MatchResult result;

  for (const OcrResult *ocrPtr : ocrBatch) {

    auto applyMatch = [&](const size_t winningIndex) {
      // match the id of the OcrResult to the id JellyfinEpisode Vector
      // iterator i points to the object that can now be enriched with Ocr
      // Data. Use lower_bound for this to take advantage of the sorting
      // earlier
      const auto i = std::ranges::lower_bound(
          episodeVec, ocrPtr->jellyfinEpisodeId, {}, &JellyfinEpisode::id);

      // safety check the found iterator is valid and correct
      // technically second check shouldnt be necessary since jellyfinIDs have
      // to be unique for the system to work, but i have been wrong before
      if (i != episodeVec.end() && i->id == ocrPtr->jellyfinEpisodeId) {
        // enrich the JellyfinEpisode with oder information based on Title
        // match
        i->seasonNumber = episodesOrder[winningIndex].seasonNumber;
        i->episodeNumber = episodesOrder[winningIndex].episodeNumber;
        i->title = ocrPtr->extractedTitle;

        // log it
        JES_INFO("Successfully enriched JellyfinEpisode: {}", *i);

        // remove matched episode from matching pool
        std::swap(episodesOrder[winningIndex], episodesOrder.back());
        episodesOrder.pop_back();
      } else {
        // Log error and continue with the rest
        // this should never realisticly be called since it needs a missmatch of
        // JellyfinIDs which should be impossible
        result.unmatched.push_back(ocrPtr);
        JES_ERROR("Unable to match OcrResult: {} to JellyfinEpisodes by ID - "
                  "skipping this JellyfinEpisode in metadataSet routine!",
                  *ocrPtr);
      }
    };

    double bestScore{92.0};
    std::vector<size_t> bestMatches{};

    for (size_t j = 0; j < episodesOrder.size(); j++) {
      // everything under 92.0 will return 0
      const double score = rapidfuzz::fuzz::token_set_ratio(
          ocrPtr->extractedTitle, episodesOrder[j].title, 92.0);

      if (score > bestScore) {
        // new bestScore
        bestScore = score;
        bestMatches.clear();
        bestMatches.push_back(j);
      } else if (score == bestScore) {
        // matches existing bestScore
        bestMatches.push_back(j);
      }
    } // for loop over episodeOrder

    if (bestMatches.size() == 1) {
      // exact match
      applyMatch(bestMatches[0]);

    } else if (bestMatches.size() > 1) {
      // run tie-breaker with stricter matching
      double strictBestScore{0.0};
      std::vector<size_t> strictWinners{};

      // loop over the indices that tied
      for (const size_t &tiedIndex : bestMatches) {
        // Use the highly strict algorithm
        double strictScore = rapidfuzz::fuzz::ratio(
            ocrPtr->extractedTitle, episodesOrder[tiedIndex].title);

        if (strictScore > strictBestScore) {
          strictBestScore = strictScore;
          strictWinners.clear();
          strictWinners.push_back(tiedIndex);
        } else if (strictScore == strictBestScore) {
          strictWinners.push_back(tiedIndex);
        }
      }

      if (strictWinners.size() == 1) {
        // SUCCESS! We broke the tie.
        applyMatch(strictWinners[0]);

      } else {
        // The tie-breaker failed. It is genuinely a stalemate.
        JES_INFO("Inconclusive match even after tie-breaker for OcrResult: {}",
                 *ocrPtr);
        result.revisit.push_back(ocrPtr);
      }
    } else {
      // no match bestIndexes is empty
      JES_ERROR("No match for OcrResult: {}", *ocrPtr);
      result.unmatched.push_back(ocrPtr);
    }
  }

  return result;
}
