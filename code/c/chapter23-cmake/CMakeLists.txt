# NOTE: These files are for educational/demonstration purposes about
# CMake concepts. They may not compile as-is since they are meant
# to demonstrate CMake features, not to be built by this CMakeLists.txt.

add_executable(c_ch23_01_example_cmake_basics 01_example_cmake_basics.c)
add_executable(c_ch23_02_example_cmake_advanced 02_example_cmake_advanced.c)
add_executable(c_ch23_01_deep_dive_cmake_patterns 01_deep_dive_cmake_patterns.c)

if(UNIX)
    target_link_libraries(c_ch23_02_example_cmake_advanced PRIVATE m)
endif()
