#pragma once

#include <string>

std::string fetchSeriesRaw(std::string_view url,
                           std::string_view searchString,
                           std::string_view apiKey);

std::string fetchEpisodesRaw(std::string_view url,
                           std::string_view seriesId,
                           std::string_view apiKey);
