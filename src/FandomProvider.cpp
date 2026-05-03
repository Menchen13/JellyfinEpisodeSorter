#include <regex>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <FandomProvider.hpp>
#include <jesLog.hpp>
#include <models.hpp>

using nlohmann::json;
using std::string;

fandomProvider::fandomProvider(const string &listUrl, fandomParser func)
    : parsingFunc(std::move(func)) {

  // capture information from Url
  // group one baseurl
  // group two anything after wiki
  std::regex urlPattern(R"(^(https?://[^/]+)/wiki/(.+)$)");

  std::smatch match_results;

  if (std::regex_match(listUrl, match_results, urlPattern)) {

    std::string baseUrl = match_results[1].str();
    std::string pageName = match_results[2].str();

    apiUrl = baseUrl + "/api.php";

    urlParameters = cpr::Parameters{{"action", "parse"},
                                    {"page", pageName},
                                    {"prop", "wikitext"},
                                    {"format", "json"}};

  } else {
    // If the URL doesn't look like a Fandom Wiki URL, we should fail loud and
    // early.
    throw std::invalid_argument(
        "Invalid URL format. Expected a Fandom Wiki /wiki/ URL.\nExample: "
        "'https://adventuretime.fandom.com/wiki/List_of_episodes/"
        "Intended_Order'");
  }
};

std::vector<Episode> fandomProvider::getEpisodes() const {

  cpr::Response r = cpr::Get(apiUrl, urlParameters);

  if (r.error) {
    throw std::runtime_error(
        std::format("Network Error: {}\nIn getEpisodes", r.error.message));
  } else if (r.status_code >= 400) {
    throw std::runtime_error(
        std::format("HTTP GET Error: {}\nIn getEpisodes", r.status_line));
  }

  JES_DEBUG("{} returned {}", r.url.c_str(), r.status_line);

  json j = json::parse(r.text);

  const std::string &wikitext =
      j["parse"]["wikitext"]["*"].get_ref<std::string &>();

  return parsingFunc(wikitext);
}

std::vector<Episode> adventureTime(const std::string &wikitext) {
  std::vector<Episode> episodes;

  // Matches on either Season <number>o and captures number
  // the o is to only get the table ones not freetext
  // OR (|)
  // [[File: ... | <maybe link> <anything but(|m])>]]
  // should match on "My Two Favorite People" for:
  // [[File:Titlecard S1E9 mytwofavoritepeople.jpg|140px|link=My Two Favorite
  // People]] or "The Enchiridion!" for:
  // [[File:Titlecard S1E5 theenchiridion.jpg|140px|link=The Enchiridion!
  // (episode)|The Enchiridion!]]
  std::regex bigBoyRegex(
      R"(Season\s+(\d+)o|\[\[File:.*?\|(?:link=)?([^\|\]]+)\]\])");

  // create iterator over regex matches
  auto words_begin =
      std::sregex_iterator(wikitext.begin(), wikitext.end(), bigBoyRegex);
  auto words_end = std::sregex_iterator();

  unsigned int currentSeason = 1;
  unsigned int currentEpisode = 1;

  for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;

    // Season matched
    if (match[1].matched) {
      // set season
      currentSeason = std::stoi(match[1].str());
      // reset EpisodeCount for new season
      currentEpisode = 1;
    }
    // Title matched
    else if (match[2].matched) {
      std::string title = match[2].str();

      // remove (episode) suffix from parts where regex cant filter it
      std::string suffix = " (episode)";
      if (title.ends_with(suffix)) {
        title.erase(title.length() - suffix.length());
      }

      // add to vector and increment episode counter
      episodes.emplace_back(title, currentSeason, currentEpisode++);
    }
  }

  JES_DEBUG("Retrieved Episode Order:\n {}", episodes);
  return episodes;
}
