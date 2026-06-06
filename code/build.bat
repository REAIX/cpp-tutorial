@echo off
setlocal

if "%1"=="help" (
    echo Usage: build.bat [help]
    echo.
    echo Examples:
    echo   build.bat        - Build all
    echo   build.bat help   - Show this help
    echo.
    echo Available C chapters:
    for /d %%d in (c\chapter*) do echo   %%d
    echo.
    echo Available C++ chapters:
    for /d %%d in (cpp\chapter*) do echo   %%d
    exit /b 0
)

if exist build rmdir /s /q build
mkdir build
cd build

if defined CMAKE_GENERATOR (
    cmake .. -G "%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=Debug
) else if defined VSINSTALLDIR (
    cmake .. -DCMAKE_BUILD_TYPE=Debug
) else (
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
)
cmake --build . --parallel

cd ..
endlocal
