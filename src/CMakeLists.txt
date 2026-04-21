add_library(jes-core STATIC "FandomProvider.cpp" "apiClient.cpp")

target_link_libraries(jes-core PUBLIC cpr::cpr nlohmann_json::nlohmann_json)
target_include_directories(jes-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(jse main.cpp)
target_link_libraries(jse PRIVATE jes-core)
