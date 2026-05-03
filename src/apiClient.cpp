#include <format>
#include <stdexcept>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "apiClient.hpp"
#include "jesLog.hpp"
#include "ocr.hpp"

using nlohmann::json;

std::string jellyfin::fetchSeriesRaw(const std::string &url,
                                     const std::string &searchString,
                                     const std::string &apiKey) {

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
  JES_INFO("Fetched series raw from {}", r.url.c_str());
  return r.text;
}

std::string jellyfin::fetchEpisodesRaw(const std::string &url,
                                       const std::string &seriesId,
                                       const std::string &apiKey) {

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
  JES_INFO("Fetched episodes raw from {}", r.url.c_str());
  return r.text;
}

std::string GoogleOCR::base64ToTitle(const std::string &base64,
                                     const std::string &googleApiKey) {

  // one session for the all the calls to this function
  // saves bunch of networking time (hopefully - never actually tested that)
  // this requires a wrapper struct to configure the session-obj at creation
  // time. this way when the static obj is created it is immediatly ready to
  // roll
  static CprSessionConfig config(
      "https://generativelanguage.googleapis.com/v1beta/"
      "models/gemini-2.5-flash-lite:generateContent",
      cpr::Header{{"x-goog-api-key", googleApiKey},
                  {"Content-Type", "application/json"}});

  // Gemini made Gemini-prompt
  // OH THE IRONY
  json payload = {
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

  config.session.SetBody(cpr::Body{payload.dump()});
  cpr::Response r = config.session.Post();

  if (r.error) {
    throw std::runtime_error(
        std::format("Network Error: {}\nIn base64ToTitle", r.error.message));

  } else if (r.status_code == 503 || r.status_code == 429) {
    throw RateLimitException();

  } else if (r.status_code >= 400) {
    throw std::runtime_error(
        std::format("HTTP POST Error: {}\nIn base64ToTitle", r.status_line));
  }

  // parse response and return text
  const json j = json::parse(r.text);

  return j.at("candidates")
      .at(0)
      .at("content")
      .at("parts")
      .at(0)
      .at("text")
      .get<std::string>();
}

std::string OllamaOCR::base64ToTitle(const std::string &base64,
                                     const std::string &url) {

  static CprSessionConfig config(
      url, cpr::Header{{"Content-Type", "application/json"}});

  // could proably make model a parameter in case people wanna use something
  // else but then again, if they can spin up anything in an ollama container,
  // might as well spin this up real quick
  json payload = {
      {"model", "llama3.2-vision"},
      {"prompt",
       "Extract only the textual title of the episode from this image. The "
       "text may be heavily stylized, hidden in the background, or written in "
       "unusual fonts. Output ONLY the exact text of the title, with no "
       "conversational filler, no punctuation unless it's in the title, and no "
       "descriptions of the image."},
      {"stream", false},
      {"images", {base64}}};

  config.session.SetBody(cpr::Body{payload.dump()});

  cpr::Response r = config.session.Post();

  if (r.error) {
    throw std::runtime_error(
        std::format("Network Error: {}\nIn base64ToTitle", r.error.message));

  } else if (r.status_code >= 400) {
    throw std::runtime_error(
        std::format("HTTP POST Error: {}\nIn base64ToTitle", r.status_line));
  }

  const json j = json::parse(r.text);

  return j.at("response").get<std::string>();
}
