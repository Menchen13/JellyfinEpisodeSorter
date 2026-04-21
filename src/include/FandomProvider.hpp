#pragma once
#include <cpr/cprtypes.h>
#include <cpr/parameters.h>
#include <functional>
#include <map>
#include <string>

#include "IEpisodeProvider.hpp"

using fandomParser =
    std::function<std::map<unsigned, std::map<unsigned int, std::string>>(
        const std::string &)>;

/**
 * @class fandomProvider
 * @brief Interface provider for fandom-API
 *
 */
class fandomProvider : public IEpisodeProvider {
private:
  cpr::Url apiUrl;
  cpr::Parameters urlParameters;
  fandomParser parsingFunc;

public:
  /**
   * @brief Basic ctor for fandomWikiProvider
   *
   * @param listUrl Url to list of episodes. Example:
   * 'https://adventuretime.fandom.com/wiki/List_of_episodes/Intended_Order'
   *
   * @param func Function which parses the output from the fandom-API into
   * nested maps Just read check out the adventureTime example func if you wanna
   * get into it (cant recommend it).
   *
   * @throw std::invalid_argument Thrown if Urlschema does not match
   */
  fandomProvider(const std::string &listUrl, fandomParser func);

  [[nodiscard]] std::map<unsigned int, std::map<unsigned int, std::string>>
  getEpisodes() const override;
};

/**
 * @brief Prints out the nested maps of Seasons, Episodes and Titles. Usefull
 * for debugging parse func
 *
 * @param A Nested maps to be printed
 */
void helperPrintMaps(
    std::map<unsigned int, std::map<unsigned int, std::string>> A);

std::map<unsigned int, std::map<unsigned int, std::string>>
adventureTime(const std::string &wikitext);
