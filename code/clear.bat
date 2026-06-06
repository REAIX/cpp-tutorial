@echo off
setlocal

if exist build (
    rmdir /s /q build
    echo Build directory cleaned.
) else (
    echo No build directory found.
)

endlocal
