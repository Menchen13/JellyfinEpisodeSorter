#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <print>
#include <providers/FandomProvider.hpp>

#include <regex>

using nlohmann::json;
using std::map;
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

map<unsigned int, map<unsigned int, string>>
fandomProvider::getEpisodes() const {

  cpr::Response r = cpr::Get(apiUrl, urlParameters);

  json j = json::parse(r.text);

  const std::string &wikitext =
      j["parse"]["wikitext"]["*"].get_ref<std::string &>();

  return parsingFunc(wikitext);
}

void helperPrintMaps(
    std::map<unsigned int, std::map<unsigned int, std::string>> A) {
  for (const auto &[seasonNum, episodesMap] : A) {
    std::println("===STAFFEL {}===", seasonNum);

    for (const auto &[episodesNum, title] : episodesMap) {
      std::println("S{:02}E{:02} - {}", seasonNum, episodesNum, title);
    }
    std::print("\n");
  }
};

std::map<unsigned int, std::map<unsigned int, std::string>>
adventureTime(const std::string &wikitext) {
  map<unsigned int, map<unsigned int, string>> episodes;

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

      // add Title to map and increment episode counter
      episodes[currentSeason][currentEpisode++] = title;
    }
  }
  return episodes;
}
