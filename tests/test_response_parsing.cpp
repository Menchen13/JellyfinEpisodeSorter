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

  std::vector<Series> result = parseSeriesList(response);

  std::vector<Series> known(
      {{"026193846bf3f910617f1878634f306a",
        "Adventure Time - Abenteuerzeit mit Finn und Jake"},
       {"10e409d2d34b38e270c1dfdfefef4fda", "ADVENTURE TIME EXTRAS"},
       {"25003ee1620a178c68ac974766bef952", "ADVENTURE TIME EXTRAS"},
       {"4ef40aa51b5e0e32d4277fe243b71a63", "ADVENTURE TIME EXTRAS"},
       {"7b0db52d21e230a8bc0bc4539a8543b8", "ADVENTURE TIME EXTRAS"}});

  ASSERT_EQ(known, result);
}

TEST(ResponseParsing, Episode) {}
