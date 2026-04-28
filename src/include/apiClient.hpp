#pragma once

#include <string>

#include <cpr/cprtypes.h>
#include <cpr/http_version.h>
#include <cpr/session.h>

struct OcrSessionConfig {
  cpr::Session session;

  explicit OcrSessionConfig(std::string_view url, const cpr::Header &headers) {
    session.SetUrl(cpr::Url{url});
    session.SetTimeout(std::chrono::seconds(30));
    session.SetHeader(headers);
  }
};

namespace jellyfin {

// takes in jellyfin url, APIKEY and searchString and returns
// raw http response for search
std::string fetchSeriesRaw(const std::string &url,
                           const std::string &searchString,
                           const std::string &apiKey);

// takes in jellyfin url, APIKEY and seriesID and returns
// raw http response for lising all episodes of the series, via parentID
std::string fetchEpisodesRaw(const std::string &url,
                             const std::string &seriesId,
                             const std::string &apiKey);
} // namespace jellyfin

namespace GoogleOCR {
// takes in base64 string representing Titlecard image and googleApiKey
// returns the title googleOCR found in the image
std::string base64ToTitle(const std::string &base64,
                          const std::string &googleApiKey);
} // namespace GoogleOCR

namespace OllamaOCR {
// takes in base64 string representing Titlecard image
// returns the title googleOCR found in the image
std::string base64ToTitle(const std::string &base64, const std::string &url);
} // namespace OllamaOCR
