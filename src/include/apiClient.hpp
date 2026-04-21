#pragma once

#include <string>

std::string fetchSeriesRaw(const std::string_view &url,
                           const std::string_view &searchString,
                           const std::string_view &apiKey);

std::string fetchEpisodesRaw(const std::string_view &url,
                           const std::string_view &seriesId,
                           const std::string_view &apiKey);
