@echo off
setlocal enabledelayedexpansion

set "LIB_NAME=cxxu"
set "LIB_DISPLAY_NAME=CXXU (C++ Utils)"
set "STATIC_TARGET=cxxu_static"
set "SHARED_TARGET=cxxu_shared"
set "TEST_TARGET=test_cxxu"
set "STATIC_OUTPUT=libcxxu.a"
set "SHARED_OUTPUT=cxxu.dll"

call "%~dp0..\build_common.bat" %*
