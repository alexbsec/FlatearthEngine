#!/bin/bash

set -e

buildDir="./build"
linkPath=$(pwd)
binDir="../bin"

# ANSI Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[1;36m'
YELLOW='\033[1;33m'
RESET='\033[0m'

echo -e "${CYAN}############################### BUILDING TESTS ###############################${RESET}"

# Check build directory
echo -e "${YELLOW}>> Checking if build directory exists...${RESET}"
if [ ! -d "$buildDir" ]; then
    echo -e "${GREEN}Creating build directory...${RESET}"
    mkdir "$buildDir"
    echo -e "${CYAN}Running 'cmake ..'${RESET}"
    cmake -S . -B "$buildDir"
else
    echo -e "${GREEN}Build directory exists!${RESET}"
fi

echo -e "${CYAN}############################# CMAKE STEP ######################################${RESET}"

if [ -f compile_commands.json ]; then
    echo -e "${YELLOW}Cleaning old compile commands...${RESET}"
    rm compile_commands.json
fi

echo -e "${GREEN}Creating compile_commands.json...${RESET}"
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B "$buildDir"
ln -sf "${linkPath}/build/compile_commands.json" compile_commands.json 

echo -e "${CYAN}Running CMake build...${RESET}"
cmake --build "$buildDir"

# Copy test binary
echo -e "${CYAN}Moving test executable to bin...${RESET}"
mv -f "${buildDir}/flatearth_tests" "${binDir}/flatearth_tests"

echo -e "${CYAN}############################# FINISHED ########################################${RESET}"
echo -e "${GREEN}Testbed executable created successfully!${RESET}"

