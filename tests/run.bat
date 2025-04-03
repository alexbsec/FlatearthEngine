@echo on

:: ----------------------------------------------------------------------------
::  Setup
:: ----------------------------------------------------------------------------

set buildDir=.\build

rmdir /s /q %buildDir%

echo ############################### BUILDING TESTS ###############################
echo ###################### Checking if build directory exists #####################

if not exist %buildDir% (
    echo Making build directory...
    mkdir %buildDir%
) else (
    echo Build directory exists!
)

echo Running "cmake .."
pushd %buildDir%
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug ..
popd

echo ############################# CMAKE STEP ######################################

echo Running CMake build...
cmake --build build --config Debug

:: Move the final test executable into ../bin
echo Moving flatearth_tests.exe into ../bin
move /Y ".\build\Debug\flatearth_tests.exe" "..\bin\flatearth_tests.exe"

echo ############################# FINISHED ########################################
echo Tests executable created!

