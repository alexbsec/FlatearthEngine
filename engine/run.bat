@echo on

REM ----------------------------------------------------------------------------
REM  Setup
REM ----------------------------------------------------------------------------

set buildDir=.\build

rmdir /s /q %buildDir%

echo ############################### BUILDING ENGINE ###############################
if not exist %buildDir% (
    echo Making build directory...
    mkdir %buildDir%
)

pushd %buildDir%
echo Running "cmake .."
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug ..
popd

echo ############################## CMAKE STEP ####################################


echo Running CMake build...
cmake --build build --config Debug

echo ############################# COPYING / LINKING STEP ##########################
echo Removing old DLL and LIB links...
del ..\bin\flatearth.dll
del ..\bin\flatearth.lib

set enginePath=%cd%
echo enginePath is %enginePath%

echo Copying dlls...

copy %enginePath%\build\Debug\flatearth.dll ..\bin\flatearth.dll
copy %enginePath%\build\Debug\flatearth.lib ..\bin\flatearth.lib

echo ############################# FINISHED #######################################
echo Flatearth library (DLL) created!