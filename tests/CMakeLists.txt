# GoogleTest via Conan
find_package(GTest REQUIRED)
include(GoogleTest)

# Add targets for UnitTests
add_executable(ProviderTests test_fandom_provider.cpp)

target_link_libraries(ProviderTests PRIVATE jse-core GTest::gtest_main)

gtest_discover_tests(ProviderTests)
