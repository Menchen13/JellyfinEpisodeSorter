#pragma once

#include <format>
#include <ostream>
#include <ranges>
#include <string>

#include <nlohmann/json.hpp>

struct Episode {
  std::string title;
  unsigned int season;
  unsigned int episodeNumber;
};

struct JellyfinSeries {
  std::string id;
  std::string name;

  // only works in cpp20 and up i think
  bool operator==(const JellyfinSeries &) const = default;
};

struct JellyfinEpisode {
  std::string id;
  std::string title;
  std::string seasonId;
  unsigned int season = 0;
  unsigned int episiodeNumber = 0;

  // only works in cpp20 and up i think
  bool operator==(const JellyfinEpisode &) const = default;
};

struct OcrResult {
  std::string jellyfinEpisodeId;
  std::string extractedTitle;
};

inline std::ostream &operator<<(std::ostream &os,
                                const JellyfinSeries &series) {
  return os << "Series{id: \"" << series.id << "\", name: \"" << series.name
            << "\"}";
}

inline std::ostream &operator<<(std::ostream &os,
                                const JellyfinEpisode &episode) {
  return os << "Episode{id: \"" << episode.id << "\", name: \"" << episode.title
            << "\", seasonId: \"" << episode.seasonId << "\", season: \""
            << episode.season << "\", episiodeNumber: \""
            << episode.episiodeNumber << "\"}";
}

inline std::ostream &operator<<(std::ostream &os, const OcrResult &OcrResult) {
  return os << "OcrResult{jellyfinEpisodeId: \"" << OcrResult.jellyfinEpisodeId
            << "\", extractedTitle \"" << OcrResult.extractedTitle << "\"}";
}

// *WARNING* Can throw some sort of json::exception
inline void from_json(const nlohmann::json &j, JellyfinSeries &s) {
  j.at("Id").get_to(s.id);
  j.at("Name").get_to(s.name);
}

// *WARNING* Can throw some sort of json::exception
inline void from_json(const nlohmann::json &j, JellyfinEpisode &e) {
  j.at("Id").get_to(e.id);
  j.at("Name").get_to(e.title);
  j.at("SeasonId").get_to(e.seasonId);
}

// creating format specifiers to try out new modern way
// of making types printable
//
// sinlge struct
template <> struct std::formatter<Episode> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Episode &obj, auto &ctx) const {
    return std::format_to(ctx.out(), "(Season: {}, Episode: {}, Title: {})",
                          obj.season, obj.episodeNumber, obj.title);
  }
};

// container of structs
template <std::ranges::input_range R>
  requires std::same_as<std::ranges::range_value_t<R>, Episode>
struct std::formatter<R> : std::range_formatter<Episode> {
  constexpr formatter() {
    // set brackets and seperator
    this->set_brackets("[\n  ", "\n]");
    this->set_separator(",\n  ");
  }
};
