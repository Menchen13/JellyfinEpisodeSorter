#pragma once

#include <map>
#include <string>

class IEpisodeProvider {
public:
  virtual ~IEpisodeProvider() = default;

  // interface function all implementations must provide doesnt matter how 
  [[nodiscard]] virtual std::map<unsigned int, std::map<unsigned int, std::string>> getEpisodes() const = 0;
};
