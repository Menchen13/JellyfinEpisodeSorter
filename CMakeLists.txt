cmake_minimum_required(VERSION 3.20)
project(jellyfinEpisodeSorter CXX)

find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenCV REQUIRED)
find_package(cppcodec REQUIRED)
find_package(rapidfuzz REQUIRED)
find_package(spdlog REQUIRED)

enable_testing()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

  # Search the system path for the lld executable
  find_program(LLD_EXECUTABLE NAMES ld.lld lld)

  if(LLD_EXECUTABLE)
    message(STATUS "Clang detected and LLD found at: ${LLD_EXECUTABLE}")

    # (Alternatively, to apply it globally to all targets, use:)
    add_link_options("-fuse-ld=lld")
  else()
    message(STATUS "Clang detected, but LLD not found. Using default linker.")
  endif()
endif()

add_subdirectory(src)
add_subdirectory(tests)
