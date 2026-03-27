@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%examples\windows\build"

echo Building Windows example...

cmake -B "%BUILD_DIR%" -S "%SCRIPT_DIR%examples\windows"
if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)

cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

echo.
echo Running Windows example...
"%BUILD_DIR%\Release\campello_input_windows_example.exe"
