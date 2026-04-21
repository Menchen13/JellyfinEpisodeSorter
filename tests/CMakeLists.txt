# GoogleTest via Conan
find_package(GTest REQUIRED)
include(GoogleTest)

# link libs to all test targets
link_libraries(jes-core GTest::gtest_main GTest::gtest)

# Add targets for UnitTests
add_executable(ProviderTests test_fandom_provider.cpp)
gtest_discover_tests(ProviderTests)

add_executable(ResponseParsingTests test_response_parsing.cpp)
target_compile_definitions(ResponseParsingTests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../API/") 
gtest_discover_tests(ResponseParsingTests)
