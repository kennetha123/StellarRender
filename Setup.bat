@echo off

REM Create and navigate into "build" directory
mkdir build 2>nul
cd build

REM Generate build files
if "%~1"=="" (
    echo Generating build files...
    cmake -G "Visual Studio 17 2022" ..
)

REM Build the project if --build or -b argument is provided
if "%~1"=="--build" (
    echo Building project...
    cmake --build .
) else if "%~1"=="-b" (
    echo Building project...
    cmake --build .
)
cd ..
