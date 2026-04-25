#pragma once

#include <string>

namespace jellyfin {

// takes in jellyfin url, APIKEY and searchString and returns
// raw http response for search
std::string fetchSeriesRaw(std::string_view url, std::string_view searchString,
                           std::string_view apiKey);

// takes in jellyfin url, APIKEY and seriesID and returns 
// raw http response for lising all episodes of the series, via parentID
std::string fetchEpisodesRaw(std::string_view url, std::string_view seriesId,
                             std::string_view apiKey);
} // namespace jellyfin

namespace GoogleOCR {
// takes in base64 string representing Titlecard image and googleApiKey
// returns the title googleOCR found in the image
std::string base64ToTitle(std::string_view base64, std::string googleApiKey);
}
