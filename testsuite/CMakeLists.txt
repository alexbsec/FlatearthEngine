############################################################################
#   SELECTING CMAKE & C++ MINIMUM VERSION
cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 23)

############################################################################
#   PROJECT SETUP
project("Flatearth Testing" LANGUAGES CXX)

# We'll define a variable for our executable name
set(BINARY_NAME "flatearth_testsuite")

############################################################################
#   INCLUDE DIRECTORIES AND LIBRARIES

# Path to the Flatearth Engine headers
include_directories("${CMAKE_SOURCE_DIR}/../engine/src")

# Path to the Flatearth shared library
link_directories("${CMAKE_SOURCE_DIR}/../bin")

############################################################################
#   SOURCE FILES

# Add your test source files here
set(SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/Game.cc"
    "${CMAKE_SOURCE_DIR}/src/Entrypoint.cc"
)

############################################################################
#   EXECUTABLE AND LINKING

# Define the testing executable
add_executable(${BINARY_NAME} ${SOURCE_FILES})

# On Unix (excluding MSVC), link X11/XCB
if(UNIX AND NOT MSVC)
    target_link_libraries(${BINARY_NAME}
        PRIVATE
            X11
            xcb
            X11-xcb
            xcb-keysyms
    )
elseif(WIN32)
    if(MSVC)
        # Add target name to target_compile_options
        target_compile_options(${BINARY_NAME} PRIVATE /W4 /permissive- /std:c++latest)
    endif()
endif()

# Link against the Flatearth shared library.
# On Windows, this will link flatearth.lib + flatearth.dll at runtime;
# on Linux, libflatearth.so.
target_link_libraries(${BINARY_NAME} PRIVATE flatearth)

# Place the final executable in the local 'build' directory:
set_target_properties(${BINARY_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)

# On Unix-like systems, set the RPATH so it can find flatearth in ../bin
# (Windows does not typically use RPATH, but it won't hurt to leave it set).
set_target_properties(${BINARY_NAME} PROPERTIES
    INSTALL_RPATH "${CMAKE_SOURCE_DIR}/../bin"
)
