add_library(jes-core STATIC "FandomProvider.cpp" "apiClient.cpp" "ocr.cpp")

target_link_libraries(jes-core PUBLIC cpr::cpr nlohmann_json::nlohmann_json opencv::opencv cppcodec::cppcodec)
target_include_directories(jes-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(jse main.cpp)
target_link_libraries(jse PRIVATE jes-core rapidfuzz::rapidfuzz)
