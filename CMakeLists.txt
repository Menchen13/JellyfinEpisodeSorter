cmake_minimum_required(VERSION 3.20)
project(jellyfinEpisodeSorter CXX)

find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenCV REQUIRED)
find_package(cppcodec REQUIRED)

enable_testing()


add_subdirectory(src)
add_subdirectory(tests)
