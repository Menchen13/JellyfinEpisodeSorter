#pragma once

#include <ostream>
#include <string>

#include <nlohmann/json.hpp>

struct Series {
  std::string id;
  std::string name;

  // only works in cpp20 and up i think
  bool operator==(const Series &) const = default;
};

struct Episode {
  std::string id;
  std::string name;
  std::string seasonId;
  unsigned int season = 0;
  unsigned int episiodeNumber = 0;

  // only works in cpp20 and up i think
  bool operator==(const Episode &) const = default;
};

inline std::ostream &operator<<(std::ostream &os, const Series &series) {
  return os << "Series{id: \"" << series.id << "\", name: \"" << series.name
            << "\"}";
}

inline std::ostream &operator<<(std::ostream &os, const Episode &episode) {
  return os << "Episode{id: \"" << episode.id << "\", name: \"" << episode.name
            << "\", seasonId: \"" << episode.seasonId << "\", season: \""
            << episode.season << "\", episiodeNumber: \""
            << episode.episiodeNumber << "\"}";
}

// *WARNING* Can throw some sort of json::exception
inline void from_json(const nlohmann::json &j, Series &s) {
  j.at("Id").get_to(s.id);
  j.at("Name").get_to(s.name);
}

// *WARNING* Can throw some sort of json::exception
inline void from_json(const nlohmann::json &j, Episode &e) {
  j.at("Id").get_to(e.id);
  j.at("Name").get_to(e.name);
  j.at("SeasonId").get_to(e.seasonId);
}
