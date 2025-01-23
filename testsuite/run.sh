#!/bin/bash

set echo on

buildDir=./build

echo "############################### BUILDING TESTBED ###############################"

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

if [ ! -f compile_commands.json ]; then
    echo "Cleaning old compile commands..."
    rm compile_commands.json
fi

echo "Creating compile_commands.json..."
linkPath=$(pwd)

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build
ln -s "${linkPath}/build/compile_commands.json" compile_commands.json 

echo "Running CMake..."

cmake --build build

pushd build
echo "Running make inside build..."
make
popd

testingPath=$(pwd)

mv "${testingPath}/build/flatearth_testsuite" ../bin/flatearth_testsuite

echo "############################# FINISHED ########################################"

echo "Testbed executable created!"
