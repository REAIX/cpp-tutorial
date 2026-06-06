@echo off
setlocal enabledelayedexpansion

set "LIB_NAME=cu"
set "LIB_DISPLAY_NAME=CU (C Utils)"
set "STATIC_TARGET=cu_static"
set "SHARED_TARGET=cu_shared"
set "TEST_TARGET=test_cu"
set "STATIC_OUTPUT=libcu.a"
set "SHARED_OUTPUT=cu.dll"

call "%~dp0..\build_common.bat" %*
