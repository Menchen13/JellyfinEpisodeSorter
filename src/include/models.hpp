#pragma once

#include <ostream>
#include <string>

struct Series {
  std::string id;
  std::string name;

  // only works in cpp20 and up i think
  bool operator==(const Series&) const = default;
};

struct Episode {
  std::string id;
  std::string name;
  unsigned int season;
  unsigned int episiodeNumber;
};


inline std::ostream& operator<<(std::ostream& os, const Series& series){
  return os << "Series{id: \"" << series.id << "\", name: \"" << series.name << "\"}";
}
