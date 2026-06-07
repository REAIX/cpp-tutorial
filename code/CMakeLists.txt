cmake_minimum_required(VERSION 3.20)

project(cpp-tutorial-all C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-pedantic>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>)
elseif(MSVC)
    add_compile_options(/W4)
endif()

add_subdirectory(c)
add_subdirectory(cpp)
add_subdirectory(advanced)

# 工具库 (独立构建, 可选)
# 取消以下注释以将工具库纳入统一构建:
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../utils/cu ${CMAKE_CURRENT_BINARY_DIR}/cu)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../utils/cxxu ${CMAKE_CURRENT_BINARY_DIR}/cxxu)
