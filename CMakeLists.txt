cmake_minimum_required(VERSION 3.20)

project(open-online-judge
    VERSION 1.0.0
    DESCRIPTION "Online Judge Platform for competitive programming"
    LANGUAGES CXX C
)

# =============================================================================
# Global settings
# =============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# =============================================================================
# Options
# =============================================================================

option(OOJ_BUILD_TESTS "Build tests" OFF)

# =============================================================================
# Third-party dependencies
# =============================================================================

# nlohmann/json (header-only)
add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/json/single_include
)

# cpp-httplib (header-only)
add_library(cpp_httplib INTERFACE)
add_library(cpp_httplib::cpp_httplib ALIAS cpp_httplib)
target_include_directories(cpp_httplib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/web/background/cpp-httplib
)
target_compile_definitions(cpp_httplib INTERFACE
    $<$<NOT:$<BOOL:${MSVC}>>:_WIN32_WINNT=0x0A00>
)

# cmark-gfm (built as subdirectory, static only)
set(CMARK_TESTS OFF CACHE BOOL "Disable cmark-gfm tests" FORCE)
set(CMARK_SHARED OFF CACHE BOOL "Disable cmark-gfm shared library" FORCE)
set(CMARK_STATIC ON CACHE BOOL "Enable cmark-gfm static library" FORCE)
add_subdirectory(core/cmark-gfm)

# Alias cmark-gfm static targets to shorter names
add_library(cmark-gfm::cmark ALIAS libcmark-gfm_static)
add_library(cmark-gfm::extensions ALIAS libcmark-gfm-extensions_static)

# =============================================================================
# Core library
# =============================================================================

add_library(core_lib STATIC
    core/auth.cpp
    core/auth.h
    core/hash.cpp
    core/hash.h
    core/session.cpp
    core/session.h
    core/user.cpp
    core/user.h
    core/contest.cpp
    core/contest.h
)

target_include_directories(core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(core_lib PUBLIC
    nlohmann_json::nlohmann_json
)

# Link cmark-gfm static libraries
target_link_libraries(core_lib PUBLIC
    libcmark-gfm_static
    libcmark-gfm-extensions_static
)

# Add include directories for cmark-gfm generated headers
target_include_directories(core_lib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/core/cmark-gfm/src
    ${CMAKE_CURRENT_SOURCE_DIR}/core/cmark-gfm/src
    ${CMAKE_CURRENT_SOURCE_DIR}/core/cmark-gfm/extensions
)

# cmark-gfm static libs need pthread on some platforms
if(NOT MSVC)
    target_link_libraries(core_lib PUBLIC pthread)
endif()

# =============================================================================
# Main executable
# =============================================================================

add_executable(open-online-judge
    main.cpp
)

target_link_libraries(open-online-judge PRIVATE
    core_lib
    cpp_httplib::cpp_httplib
    $<$<NOT:$<BOOL:${MSVC}>>:ws2_32>
)

# =============================================================================
# Compiler-specific options
# =============================================================================

if(MSVC)
    target_compile_options(core_lib PRIVATE /W4)
    target_compile_options(open-online-judge PRIVATE /W4)
    # Enable segment heap for Windows
    target_link_options(open-online-judge PRIVATE /MANIFEST:EMBED /MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/cmake/manifest.xml)
else()
    target_compile_options(core_lib PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(open-online-judge PRIVATE -Wall -Wextra -pedantic)
endif()

# =============================================================================
# Install rules
# =============================================================================

install(TARGETS open-online-judge
    RUNTIME DESTINATION bin
)

install(DIRECTORY data/
    DESTINATION share/open-online-judge/data
)
