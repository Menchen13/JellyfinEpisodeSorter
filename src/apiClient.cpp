#include <format>
#include <stdexcept>
#include <string_view>

#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <cpr/parameters.h>
#include <cpr/response.h>
#include <nlohmann/json.hpp>

#include "apiClient.hpp"

using nlohmann::json;

std::string fetchSeriesRaw(const std::string_view &url,
                           const std::string_view &searchString,
                           const std::string_view &apiKey) {

  const cpr::Url itemApi = std::format("{}/Items", url);

  const auto params = cpr::Parameters{{"searchTerm", searchString.data()},
                                      {"recursive", "true"},
                                      {"IncludeItemTypes", "Series"}};
  const auto headers = cpr::Header{{"X-Emby-Token", apiKey.data()}};

  const cpr::Response r = cpr::Get(itemApi, params, headers);

  if (r.error) {
    throw std::runtime_error(
        std::format("Network Error: {}\nIn fetchSeriesRaw", r.error.message));
  } else if (r.status_code >= 400) {
    throw std::runtime_error(
        std::format("HTTP GET Error: {}\nIn fetchSeriesRaw", r.status_line));
  }
  return r.text;
}

std::string fetchEpisodesRaw(const std::string_view &url,
                             const std::string_view &seriesId,
                             const std::string_view &apiKey) {

  const cpr::Url itemApi = std::format("{}/Items", url);

  const auto headers = cpr::Header{{"X-Emby-Token", apiKey.data()}};

  const auto params = cpr::Parameters{{"parentId", seriesId.data()},
                                      {"recursive", "true"},
                                      {"IncludeItemTypes", "Episode"}};
  const cpr::Response r = cpr::Get(itemApi, params, headers);

  if (r.error) {
    throw std::runtime_error(
        std::format("Network Error: {}\nIn fetchEpisodesRaw", r.error.message));
  } else if (r.status_code >= 400) {
    throw std::runtime_error(
        std::format("HTTP GET Error: {}\nIn fetchEpisodesRaw", r.status_line));
  }
  return r.text;
}
