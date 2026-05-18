add_library(jes-core STATIC "FandomProvider.cpp" "apiClient.cpp" "ocr.cpp" "jesLog.cpp")

target_link_libraries(jes-core PUBLIC cpr::cpr nlohmann_json::nlohmann_json opencv::opencv cppcodec::cppcodec spdlog::spdlog rapidfuzz::rapidfuzz)
target_include_directories(jes-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# set CPP std for lib
target_compile_features(jes-core PRIVATE cxx_std_23)
set_target_properties(jes-core PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)

# For Debug builds: Set macro to TRACE (or DEBUG)
target_compile_definitions(jes-core PUBLIC 
    "$<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>"
)

# For Release builds: Set macro to INFO
target_compile_definitions(jes-core PUBLIC 
    "$<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>"
)


add_executable(jes main.cpp)

target_compile_features(jes PRIVATE cxx_std_23)
set_target_properties(jes PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(jes PRIVATE jes-core)
