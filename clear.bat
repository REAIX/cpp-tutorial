@echo off
setlocal

if exist code\build (
    rmdir /s /q code\build
    echo code\build cleaned.
) else (
    echo No code\build directory found.
)

if exist utils\cu\build (
    rmdir /s /q utils\cu\build
    echo utils\cu\build cleaned.
) else (
    echo No utils\cu\build directory found.
)

if exist utils\cxxu\build (
    rmdir /s /q utils\cxxu\build
    echo utils\cxxu\build cleaned.
) else (
    echo No utils\cxxu\build directory found.
)

echo Done.
endlocal
