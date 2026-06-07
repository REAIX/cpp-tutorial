set(COROUTINE_TARGETS
    cpp_ch25_01_deep_dive_coroutine_mechanism
    cpp_ch25_01_example_coroutine_basics
    cpp_ch25_02_deep_dive_coroutine_patterns
    cpp_ch25_02_example_generator
    cpp_ch25_03_example_task
)

add_executable(cpp_ch25_01_deep_dive_coroutine_mechanism 01_deep_dive_coroutine_mechanism.cpp)
add_executable(cpp_ch25_01_example_coroutine_basics 01_example_coroutine_basics.cpp)
add_executable(cpp_ch25_02_deep_dive_coroutine_patterns 02_deep_dive_coroutine_patterns.cpp)
add_executable(cpp_ch25_02_example_generator 02_example_generator.cpp)
add_executable(cpp_ch25_03_example_task 03_example_task.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        foreach(tgt ${COROUTINE_TARGETS})
            target_compile_options(${tgt} PRIVATE -fcoroutines)
        endforeach()
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
        foreach(tgt ${COROUTINE_TARGETS})
            target_compile_options(${tgt} PRIVATE -fcoroutines-ts)
        endforeach()
    endif()
endif()
