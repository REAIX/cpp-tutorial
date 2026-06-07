# NOTE: These files are for educational/demonstration purposes about
# Makefile concepts. They may not compile as-is since they are meant
# to be built with the accompanying Makefiles, not CMake.

add_executable(c_ch21_01_example_makefile_basics 01_example_makefile_basics.c)
add_executable(c_ch21_02_example_makefile_advanced 02_example_makefile_advanced.c)
add_executable(c_ch21_01_deep_dive_makefile_patterns 01_deep_dive_makefile_patterns.c)

if(UNIX)
    target_link_libraries(c_ch21_02_example_makefile_advanced PRIVATE m)
endif()
