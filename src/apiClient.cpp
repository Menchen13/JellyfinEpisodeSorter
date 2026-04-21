#include <cpr/cprtypes.h>
#include <format>
#include <stdexcept>
#include <string_view>

#include <cpr/api.h>
#include <cpr/parameters.h>
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
  const auto headers = cpr::Header{{"X-Emby-Token", "c5d2356a93834aa5bba1de582eb1ecc0"}};

  const cpr::Response r = cpr::Get(itemApi, params, headers);

  if(r.error) {
    throw std::runtime_error(std::format("Network Error: {}", r.error.message));
  } else if(r.status_code >= 400) {
    throw std::runtime_error(std::format("HTTP GET Error: {}", r.status_line));
  }
  return r.text;
}
