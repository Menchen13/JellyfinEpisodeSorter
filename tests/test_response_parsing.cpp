#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "net/jellyfin.hpp"

// Helper struct to safely redirect and restore std::cin
// needed for meunu unitTests in MultipleItems
struct CinRedirector {
    std::streambuf* original;
    CinRedirector(std::streambuf* new_buf) {
        original = std::cin.rdbuf(new_buf);
    }
    ~CinRedirector() {
        std::cin.rdbuf(original);
    }
};

TEST(ResponseParsing, SingleItem) {
  // TEST_DATA_DIR was defined in CMake
  std::filesystem::path dataPath =
      std::filesystem::path(TEST_DATA_DIR) / "AdventureTimeSearch.json";

  std::ifstream response(dataPath);
  ASSERT_TRUE(response.is_open()) << "Could not find: " << dataPath;

  std::string result = processSearch(response);

  EXPECT_EQ(result, "026193846bf3f910617f1878634f306a");
}

TEST(ResponseParsing, MultipleItems) {
  // TEST_DATA_DIR was defined in CMake
  std::filesystem::path dataPath =
      std::filesystem::path(TEST_DATA_DIR) / "AdventureTimeSearchMultiple.json";

  // testcase works with 1 option being picked.
  // doesnt really mater as long as id matches in EXPECT_EQ.
  std::istringstream simulatedInput("1\n");

  // fills cin buffer with "1\n" for the testcase
  CinRedirector redirector(simulatedInput.rdbuf());

  std::ifstream response(dataPath);
  ASSERT_TRUE(response.is_open()) << "Could not find: " << dataPath;

  std::string result = processSearch(response);

  EXPECT_EQ(result, "026193846bf3f910617f1878634f306a");
}


//CREATE UNIT TEST FOR NOITEMS error handeling
