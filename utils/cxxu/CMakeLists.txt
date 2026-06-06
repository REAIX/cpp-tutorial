cmake_minimum_required(VERSION 3.20)

project(cxxu CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

set(CXXU_SOURCES
    src/collection_utils.cpp
    src/config_utils.cpp
    src/date_time_utils.cpp
    src/encoding_utils.cpp
    src/file_utils.cpp
    src/hash_utils.cpp
    src/json_utils.cpp
    src/log_utils.cpp
    src/number_utils.cpp
    src/process_utils.cpp
    src/random_utils.cpp
    src/string_utils.cpp
)

add_library(cxxu_static STATIC ${CXXU_SOURCES})
set_target_properties(cxxu_static PROPERTIES OUTPUT_NAME cxxu)
target_include_directories(cxxu_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if(WIN32)
    target_link_libraries(cxxu_static PRIVATE advapi32 shell32)
endif()

add_library(cxxu_shared SHARED ${CXXU_SOURCES})
set_target_properties(cxxu_shared PROPERTIES OUTPUT_NAME cxxu)
target_include_directories(cxxu_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(cxxu_shared PRIVATE CXXU_BUILDING_DLL)
target_compile_definitions(cxxu_shared PUBLIC CXXU_DLL)
if(WIN32)
    target_link_libraries(cxxu_shared PRIVATE advapi32 shell32)
endif()

add_executable(test_cxxu test/test_utils.cpp)
target_link_libraries(test_cxxu PRIVATE cxxu_static)
