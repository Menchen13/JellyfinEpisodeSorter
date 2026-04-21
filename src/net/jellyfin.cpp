#include <expected>
#include <format>
#include <string_view>

#include <cpr/api.h>
#include <cpr/parameters.h>
#include <nlohmann/json.hpp>

#include "net/jellyfin.hpp"
using nlohmann::json;
using std::expected;

std::expected<std::string, std::string>
net::getSeriesId(std::string_view &url, std::string_view& searchString,
            std::string_view &apiKey) {

  const cpr::Url itemApi = std::format("{}/Items", url);

  const auto params = cpr::Parameters{{"searchTerm", searchString.data()},
                                      {"recursive", "true"},
                                      {"IncludeItemTypes", "Series"}};

  const cpr::Response r = cpr::Get(itemApi, params);

  return "GOOD";
}

