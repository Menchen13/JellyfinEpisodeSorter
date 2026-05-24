add_library(jes-core STATIC "FandomProvider.cpp" "apiClient.cpp" "ocr.cpp" "jesLog.cpp")

target_link_libraries(jes-core PUBLIC cpr::cpr nlohmann_json::nlohmann_json opencv::opencv cppcodec::cppcodec spdlog::spdlog rapidfuzz::rapidfuzz)
target_include_directories(jes-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# set CPP std for lib
# needs to be public since the public include dir contains cpp23 headers
target_compile_features(jes-core PUBLIC cxx_std_23)
set_target_properties(jes-core PROPERTIES CXX_STANDARD_REQUIRED ON)

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


target_link_libraries(jes PRIVATE jes-core)

#set Wall and Wextra for targets
foreach(target_name jes-core jes)
  target_compile_options(${target_name} PRIVATE
    $<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall;-Wextra>
)
endforeach()

# broken with my setup: clang22 and ninja
# activate IPO if possible
# include(CheckIPOSupported)
# check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
#
# if(ipo_supported)
#   message(STATUS "IPO/LTO supported. Enabling for Release builds.")
#   # The _RELEASE suffix ensures this ONLY applies to Release builds
#   set_target_properties(jes-core jes PROPERTIES
#         INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
#     )
# else()
#   message(WARNING "IPO/LTO not supported by this compiler/linker: ${ipo_error}")
# endif()
