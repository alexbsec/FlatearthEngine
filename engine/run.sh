#!/bin/bash

set echo on

buildDir=./build

echo "############################### BUILDING ENGINE ###############################"

echo "###################### Checking if build directory exists #####################"

if [ ! -d "$buildDir" ]; then
    echo "Making build directory..."
    mkdir build
    pushd build
    echo "Running 'cmake ..'"
    cmake ..
    popd
else
    echo "Build directory exists!"
fi

echo "############################# CMAKE STEP ######################################"

if [ -f compile_commands.json ]; then
    echo "Cleaning old compile commands..."
    rm compile_commands.json
fi

linkPath=$(pwd)
echo "Creating compile_commands.json..."

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build
ln -s "${linkPath}/build/compile_commands.json" compile_commands.json 

echo "Running CMake..."

cmake --build build

pushd build
echo "Running make inside build..."
make
popd

echo "############################# COPYING STEP ####################################"

echo "Cleaning old library links..."
rm -rf ../bin/libflatearth.so*

enginePath=$(pwd)

echo "Making new links..."
ln -s "${enginePath}/build/libflatearth.so" ../bin/libflatearth.so
ln -s "${enginePath}/build/libflatearth.so.0" ../bin/libflatearth.so.0
ln -s "${enginePath}/build/libflatearth.so.0.1" ../bin/libflatearth.so.0.1

echo "############################# FINISHED ########################################"

echo "Flatearth library created!"
