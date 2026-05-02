#pragma once

#include <models.hpp>

class IEpisodeProvider {
public:
  virtual ~IEpisodeProvider() = default;

  // interface function all implementations must provide doesnt matter how 
  [[nodiscard]] virtual std::vector<Episode> getEpisodes() const = 0;
};
