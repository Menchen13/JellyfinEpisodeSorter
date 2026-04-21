#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <vector>

#include "models.hpp"
#include "parser.hpp"

TEST(ResponseParsing, Series) {
  // TEST_DATA_DIR was defined in CMake
  std::filesystem::path dataPath =
      std::filesystem::path(TEST_DATA_DIR) / "AdventureTimeSearchMultiple.json";

  std::ifstream response(dataPath);
  ASSERT_TRUE(response.is_open()) << "Could not find: " << dataPath;

  std::vector<Series> result = parseJellyfinResponse<Series>(response);

  std::vector<Series> known(
      {{"026193846bf3f910617f1878634f306a",
        "Adventure Time - Abenteuerzeit mit Finn und Jake"},
       {"10e409d2d34b38e270c1dfdfefef4fda", "ADVENTURE TIME EXTRAS"},
       {"25003ee1620a178c68ac974766bef952", "ADVENTURE TIME EXTRAS"},
       {"4ef40aa51b5e0e32d4277fe243b71a63", "ADVENTURE TIME EXTRAS"},
       {"7b0db52d21e230a8bc0bc4539a8543b8", "ADVENTURE TIME EXTRAS"}});

  ASSERT_EQ(known, result);
}

TEST(ResponseParsing, Episode) {
  // TEST_DATA_DIR was defined in CMake
  std::filesystem::path dataPath =
      std::filesystem::path(TEST_DATA_DIR) / "GetEpisodesFromSeriesID.json";

  std::ifstream response(dataPath);
  ASSERT_TRUE(response.is_open())
      << "Could not find: " << dataPath << "Could not find: " << dataPath;

  std::vector<Episode> result = parseJellyfinResponse<Episode>(response);

  std::vector<Episode> known(
      {{"764d68737212c54fec655863ae9c96ed", "The Enchiridion!",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"f962d726e9adeb2d2bb6f5e415eb828c", "Evicted!",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"5fff254f25ee6ef4029123225fd5bb63", "Prisoners of Love",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"552ee07428469f3c19ec75a0cb3b2861", "Freaky City",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"d55a152e6620a34f106b3886b74d7806", "Ricardio the Heart Guy",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"b4b5490f7c93b0a94d47171d29f3eeb8", "Panik auf der Pyjamaparty",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"2ce3cca5a9de4b23336402fd37d60df6", "Memories of Boom Boom Mountain",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"dd7ccd518cd8cc06e402cc32fd5c7faf", "The Jiggler",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"a587cac83aae594541be3c706813e527", "City of Thieves",
        "b7a1bd178672f559ce36517f9d91613a"},
       {"e69a1a0b30e512eae5eb5e82dfa7d707", "When Wedding Bells Thaw",
        "b7a1bd178672f559ce36517f9d91613a"}});

  ASSERT_EQ(known, result);
}
