find_package(Threads REQUIRED)

add_executable(cpp_ch31_01_deep_dive_network_patterns 01_deep_dive_network_patterns.cpp)
target_link_libraries(cpp_ch31_01_deep_dive_network_patterns Threads::Threads)
if(WIN32)
    target_link_libraries(cpp_ch31_01_deep_dive_network_patterns ws2_32)
endif()

add_executable(cpp_ch31_01_example_socket_tcp 01_example_socket_tcp.cpp)
target_link_libraries(cpp_ch31_01_example_socket_tcp Threads::Threads)
if(WIN32)
    target_link_libraries(cpp_ch31_01_example_socket_tcp ws2_32)
endif()

add_executable(cpp_ch31_02_deep_dive_network_advanced 02_deep_dive_network_advanced.cpp)
target_link_libraries(cpp_ch31_02_deep_dive_network_advanced Threads::Threads)
if(WIN32)
    target_link_libraries(cpp_ch31_02_deep_dive_network_advanced ws2_32)
endif()
