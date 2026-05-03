add_library(jes-core STATIC "FandomProvider.cpp" "apiClient.cpp" "ocr.cpp" "jesLog.cpp")

target_link_libraries(jes-core PUBLIC cpr::cpr nlohmann_json::nlohmann_json opencv::opencv cppcodec::cppcodec spdlog::spdlog)
target_include_directories(jes-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(jes main.cpp)
target_link_libraries(jes PRIVATE jes-core rapidfuzz::rapidfuzz)
