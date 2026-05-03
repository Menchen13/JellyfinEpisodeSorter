add_library(jes-core STATIC "FandomProvider.cpp" "apiClient.cpp" "ocr.cpp" "jesLog.cpp")

target_link_libraries(jes-core PUBLIC cpr::cpr nlohmann_json::nlohmann_json opencv::opencv cppcodec::cppcodec spdlog::spdlog rapidfuzz::rapidfuzz)
target_include_directories(jes-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# For Debug builds: Set macro to TRACE (or DEBUG)
target_compile_definitions(jes-core PUBLIC 
    "$<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>"
)

# For Release builds: Set macro to INFO
target_compile_definitions(jes-core PUBLIC 
    "$<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>"
)


add_executable(jes main.cpp)
target_link_libraries(jes PRIVATE jes-core)
