@echo off
setlocal

if not exist build (
    echo No build directory found.
    goto :end
)

echo Cleaning build directory...

rem Try normal removal first
rmdir /s /q build 2>nul
if not exist build (
    echo Build directory cleaned.
    goto :end
)

rem If still exists, kill processes that may lock files
echo Some files are locked. Terminating build processes...
taskkill /f /im mingw32-make.exe >nul 2>&1
taskkill /f /im cc1.exe >nul 2>&1
taskkill /f /im cc1plus.exe >nul 2>&1
taskkill /f /im ld.exe >nul 2>&1
taskkill /f /im as.exe >nul 2>&1

rem Wait for processes to release files
timeout /t 2 /nobreak >nul

rem Retry removal
rmdir /s /q build 2>nul
if not exist build (
    echo Build directory cleaned.
) else (
    echo WARNING: Could not fully remove build directory.
    echo Some files may still be locked. Please close any running programs and try again.
)

:end
endlocal
