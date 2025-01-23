@echo on

:: ----------------------------------------------------------------------------
::  Setup
:: ----------------------------------------------------------------------------

set buildDir=.\build

rmdir /s /q %buildDir%

echo ############################### BUILDING TESTBED ###############################
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
echo Moving flatearth_testsuite.exe into ../bin
move ".\build\Debug\flatearth_testsuite.exe" "..\bin\flatearth_testsuite.exe"

echo ############################# FINISHED ########################################
echo Testbed executable created!