# Sets the minimum required version of CMake
cmake_minimum_required(VERSION 3.18.1)

# Declares the project name
project("edgedetection")

# Add the C++ files to create the shared library "native-lib"
add_library(
        native-lib
        SHARED
        native-lib.cpp
        ImageProcessor.cpp)

# Find and include the NDK logging library (for android/log.h)
find_library(
        log-lib
        log)

# Link the libraries to our native-lib
target_link_libraries(
        native-lib
        ${log-lib})