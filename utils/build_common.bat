@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM  Common Build Script
REM  Called by cu/build.bat and cxxu/build.bat
REM
REM  Required environment variables (set by caller):
REM    LIB_NAME         - Library name (e.g. "cu" or "cxxu")
REM    LIB_DISPLAY_NAME - Display name (e.g. "CU (C Utils)")
REM    STATIC_TARGET    - Static library CMake target (e.g. "cu_static")
REM    SHARED_TARGET    - Shared library CMake target (e.g. "cu_shared")
REM    TEST_TARGET      - Test CMake target (e.g. "test_cu")
REM    STATIC_OUTPUT    - Static library output filename (e.g. "libcu.a")
REM    SHARED_OUTPUT    - Shared library output filename (e.g. "cu.dll")
REM ============================================================

set "BUILD_TYPE=Debug"
set "LIB_TYPE=static"
set "RUN_TEST=0"
set "CLEAN=0"

:parse_args
if "%1"=="" goto :done_args
if /i "%1"=="static" set "LIB_TYPE=static"
if /i "%1"=="shared" set "LIB_TYPE=shared"
if /i "%1"=="release" set "BUILD_TYPE=Release"
if /i "%1"=="debug" set "BUILD_TYPE=Debug"
if /i "%1"=="test" set "RUN_TEST=1"
if /i "%1"=="clean" set "CLEAN=1"
if /i "%1"=="-h" goto :help
if /i "%1"=="--help" goto :help
shift
goto :parse_args

:done_args

echo ========================================
echo  %LIB_DISPLAY_NAME% Build Script
echo ========================================
echo  Build Type : %BUILD_TYPE%
echo  Library    : %LIB_TYPE%
echo ========================================
echo.

if "%CLEAN%"=="1" (
    if exist build rmdir /s /q build
    echo Cleaned.
    if "%RUN_TEST%"=="0" exit /b 0
)

if exist build rmdir /s /q build
mkdir build
cd build

if defined CMAKE_GENERATOR (
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
) else if defined VSINSTALLDIR (
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
) else (
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
)

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] CMake configuration failed!
    cd ..
    exit /b 1
)

echo.
echo Building %LIB_NAME% (%LIB_TYPE%)...
if "%LIB_TYPE%"=="shared" (
    cmake --build . --target %SHARED_TARGET% --parallel
) else (
    cmake --build . --target %STATIC_TARGET% --parallel
)

if %ERRORLEVEL% neq 0 (
    echo.
    echo [ERROR] Build failed!
    cd ..
    exit /b 1
)

if "%RUN_TEST%"=="1" (
    echo.
    echo Building %TEST_TARGET%...
    cmake --build . --target %TEST_TARGET% --parallel
    echo.
    echo --- Running Tests ---
    %TEST_TARGET%.exe
)

cd ..
echo.
echo [SUCCESS] Build complete!
echo  Output: build\%STATIC_OUTPUT% (static) or build\%SHARED_OUTPUT% (shared)
exit /b 0

:help
echo Usage: build.bat [options]
echo.
echo Options:
echo   static      Build static library (default)
echo   shared      Build shared/dynamic library
echo   debug       Build in Debug mode (default)
echo   release     Build in Release mode
echo   test        Build and run tests
echo   clean       Remove build directory
echo   -h, --help  Show this help message
echo.
echo Examples:
echo   build.bat                Build static library (Debug)
echo   build.bat shared         Build shared library (Debug)
echo   build.bat static release Build static library (Release)
echo   build.bat shared test    Build shared library and run tests
exit /b 0
