cmake_minimum_required(VERSION 3.20)

find_package(Threads REQUIRED)

# 第0节: 网络编程进阶概述
add_executable(adv_ch01_00_example_socket_patterns 00_example_socket_patterns.cpp)
target_link_libraries(adv_ch01_00_example_socket_patterns Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_00_example_socket_patterns ws2_32)
endif()

# 第1节: IO多路复用深入
add_executable(adv_ch01_01_example_select_server 01_example_select_server.cpp)
target_link_libraries(adv_ch01_01_example_select_server Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_01_example_select_server ws2_32)
endif()

add_executable(adv_ch01_01_deep_dive_poll_comparison 01_deep_dive_poll_comparison.cpp)
target_link_libraries(adv_ch01_01_deep_dive_poll_comparison Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_01_deep_dive_poll_comparison ws2_32)
endif()

add_executable(adv_ch01_01_deep_dive_epoll_concepts 01_deep_dive_epoll_concepts.cpp)
target_link_libraries(adv_ch01_01_deep_dive_epoll_concepts Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_01_deep_dive_epoll_concepts ws2_32)
endif()

# 第2节: Reactor模式
add_executable(adv_ch01_02_example_reactor_simple 02_example_reactor_simple.cpp)
target_link_libraries(adv_ch01_02_example_reactor_simple Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_02_example_reactor_simple ws2_32)
endif()

add_executable(adv_ch01_02_deep_dive_reactor_eventloop 02_deep_dive_reactor_eventloop.cpp)
target_link_libraries(adv_ch01_02_deep_dive_reactor_eventloop Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_02_deep_dive_reactor_eventloop ws2_32)
endif()

# 第3节: Proactor模式与异步IO
add_executable(adv_ch01_03_example_proactor_concepts 03_example_proactor_concepts.cpp)
target_link_libraries(adv_ch01_03_example_proactor_concepts Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_03_example_proactor_concepts ws2_32)
endif()

add_executable(adv_ch01_03_deep_dive_overlapped_io 03_deep_dive_overlapped_io.cpp)
target_link_libraries(adv_ch01_03_deep_dive_overlapped_io Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_03_deep_dive_overlapped_io ws2_32)
endif()

# 第4节: 零拷贝与高效数据传输
add_executable(adv_ch01_04_example_zero_copy_concepts 04_example_zero_copy_concepts.cpp)
target_link_libraries(adv_ch01_04_example_zero_copy_concepts Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_04_example_zero_copy_concepts ws2_32)
endif()

# 第5节: 高性能协议设计
add_executable(adv_ch01_05_example_binary_protocol 05_example_binary_protocol.cpp)
target_link_libraries(adv_ch01_05_example_binary_protocol Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_05_example_binary_protocol ws2_32)
endif()

add_executable(adv_ch01_05_deep_dive_protocol_design 05_deep_dive_protocol_design.cpp)
target_link_libraries(adv_ch01_05_deep_dive_protocol_design Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_05_deep_dive_protocol_design ws2_32)
endif()

# 第6节: 高并发服务器架构
add_executable(adv_ch01_06_example_thread_pool_server 06_example_thread_pool_server.cpp)
target_link_libraries(adv_ch01_06_example_thread_pool_server Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_06_example_thread_pool_server ws2_32)
endif()

add_executable(adv_ch01_06_deep_dive_connection_manager 06_deep_dive_connection_manager.cpp)
target_link_libraries(adv_ch01_06_deep_dive_connection_manager Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_06_deep_dive_connection_manager ws2_32)
endif()

# 第7节: 网络性能调优
add_executable(adv_ch01_07_example_tcp_tuning 07_example_tcp_tuning.cpp)
target_link_libraries(adv_ch01_07_example_tcp_tuning Threads::Threads)
if(WIN32)
    target_link_libraries(adv_ch01_07_example_tcp_tuning ws2_32)
endif()
