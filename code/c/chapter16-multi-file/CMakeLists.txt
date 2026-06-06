add_executable(c_ch16_01_example_basic
    01_example_basic/main.c
    01_example_basic/math_utils.c
)

target_include_directories(c_ch16_01_example_basic PRIVATE 01_example_basic)

add_executable(c_ch16_01_deep_dive_opaque_pointer
    01_deep_dive_opaque_pointer/main.c
    01_deep_dive_opaque_pointer/point.c
)

target_include_directories(c_ch16_01_deep_dive_opaque_pointer PRIVATE 01_deep_dive_opaque_pointer)

add_executable(c_ch16_02_example_header_guard
    02_example_header_guard/main.c
    02_example_header_guard/module_a.c
)

target_include_directories(c_ch16_02_example_header_guard PRIVATE 02_example_header_guard)

add_executable(c_ch16_01_deep_dive_project_structure 01_deep_dive_project_structure.c)
add_executable(c_ch16_02_deep_dive_linking_issues 02_deep_dive_linking_issues.c)

if(UNIX)
    target_link_libraries(c_ch16_01_deep_dive_opaque_pointer PRIVATE m)
endif()
