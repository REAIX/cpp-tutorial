cmake_minimum_required(VERSION 3.20)

project(cu C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wno-pedantic)
endif()

set(CU_SOURCES
    src/collection_utils.c
    src/encoding_utils.c
    src/file_utils.c
    src/number_utils.c
    src/process_utils.c
    src/random_utils.c
    src/string_utils.c
    src/log_utils.c
    src/config_utils.c
    src/hash_utils.c
    src/date_time_utils.c
    src/assert_utils.c
)

add_library(cu_static STATIC ${CU_SOURCES})
set_target_properties(cu_static PROPERTIES OUTPUT_NAME cu)
target_include_directories(cu_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if(WIN32)
    target_link_libraries(cu_static PRIVATE advapi32)
endif()

add_library(cu_shared SHARED ${CU_SOURCES})
set_target_properties(cu_shared PROPERTIES OUTPUT_NAME cu)
target_include_directories(cu_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(cu_shared PRIVATE CU_BUILDING_DLL)
target_compile_definitions(cu_shared PUBLIC CU_DLL)
if(WIN32)
    target_link_libraries(cu_shared PRIVATE advapi32)
endif()

add_executable(test_cu tests/test_utils.c)
target_link_libraries(test_cu PRIVATE cu_static)
