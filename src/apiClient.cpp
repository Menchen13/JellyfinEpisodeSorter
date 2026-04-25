#include <format>
#include <stdexcept>
#include <string_view>

#include <cpr/body.h>
#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <cpr/parameters.h>
#include <cpr/response.h>
#include <nlohmann/json.hpp>

#include "apiClient.hpp"

using nlohmann::json;

std::string jellyfin::fetchSeriesRaw(std::string_view url,
                                     std::string_view searchString,
                                     std::string_view apiKey) {

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

std::string jellyfin::fetchEpisodesRaw(std::string_view url,
                                       std::string_view seriesId,
                                       std::string_view apiKey) {

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

std::string GoogleOCR::base64ToTitle(std::string_view base64,
                                     std::string googleApiKey) {
  // send request to googleOCR API
  cpr::Url ApiEndpoint("https://generativelanguage.googleapis.com/v1beta/"
                          "models/gemini-2.5-flash:generateContent");

  const auto headers = cpr::Header{{"x-goog-api-key", googleApiKey},
                                   {"Content-Type", "application/json"}};

  // Gemini made Gemini-prompt
  // OH THE IRONY
  nlohmann::json payload = {
      {"contents",
       {{{"parts",
          {// Part 1: The Text Prompt
           {{"text", "Read the text on this cartoon title card. Respond ONLY "
                     "with the exact episode title. Do not add any "
                     "punctuation, markdown, or conversational text."}},
           // Part 2: The Base64 Image
           {{"inline_data",
             {{"mime_type", "image/jpeg"}, {"data", base64}}}}}}}}},
      // 3. Configure the Model Output
      {"generationConfig",
       {
           {"temperature", 0.0} // 0.0 means "Be factual, don't be creative"
       }}};

  cpr::Response r = cpr::Post(headers, ApiEndpoint, cpr::Body{payload.dump()});

  if (r.error) {
    throw std::runtime_error(
        std::format("Network Error: {}\nIn base64ToTitle", r.error.message));
  } else if (r.status_code >= 400) {
    throw std::runtime_error(
        std::format("HTTP POST Error: {}\nIn base64ToTitle", r.status_line));
  }


  // parse response and return text 
  json j = json::parse(r.text);
  
  return j.at("candidates").at(0).at("content").at("parts").at(0).at("text").get<std::string>();
}
