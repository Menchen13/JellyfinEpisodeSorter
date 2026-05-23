add_library(jes-core STATIC "FandomProvider.cpp" "apiClient.cpp" "ocr.cpp" "jesLog.cpp")

target_link_libraries(jes-core PUBLIC cpr::cpr nlohmann_json::nlohmann_json opencv::opencv cppcodec::cppcodec spdlog::spdlog rapidfuzz::rapidfuzz)
target_include_directories(jes-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# set CPP std for lib
# needs to be public since the public include dir contains cpp23 headers
target_compile_features(jes-core PUBLIC cxx_std_23)
set_target_properties(jes-core PROPERTIES CXX_STANDARD_REQUIRED ON)

#set Wall and Wextra for lib
target_compile_options(jes-core PRIVATE
    $<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall;-Wextra>
)

# For Debug builds: Set macro to TRACE (or DEBUG)
target_compile_definitions(jes-core PUBLIC
    "$<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>"
)

# For Release builds: Set macro to INFO
target_compile_definitions(jes-core PUBLIC
    "$<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>"
)


add_executable(jes main.cpp)

# set CPP std for app
target_compile_features(jes PRIVATE cxx_std_23)
set_target_properties(jes PROPERTIES CXX_STANDARD_REQUIRED ON)

#set Wall and Wextra for app
target_compile_options(jes PRIVATE
    $<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall;-Wextra>
)

target_link_libraries(jes PRIVATE jes-core)
