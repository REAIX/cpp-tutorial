# NOTE: C++20 module compilation requires special compiler support.
# These targets may NOT compile with standard CMake. Module files typically
# need compiler-specific flags and build system support that CMake provides
# only experimentally. See: https://cmake.org/cmake/help/latest/prop_sf/CXX_MODULE_SET.html

add_executable(cpp_ch27_01_deep_dive_module_migration 01_deep_dive_module_migration.cpp)
add_executable(cpp_ch27_01_example_module_basics 01_example_module_basics.cpp)
add_executable(cpp_ch27_02_deep_dive_module_patterns 02_deep_dive_module_patterns.cpp)
add_executable(cpp_ch27_02_example_module_partition 02_example_module_partition.cpp)
