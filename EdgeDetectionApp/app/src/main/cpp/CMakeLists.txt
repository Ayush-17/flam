# Sets the minimum required version of CMake
cmake_minimum_required(VERSION 3.18.1)

# Declares the project name
project("edgedetection")

# --- OpenCV Integration ---
set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/sdk/native/jni)
message(STATUS "OpenCV SDK path: ${OpenCV_DIR}")

find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
# --- End OpenCV Integration ---

# Add the C++ files to create the shared library "native-lib"
add_library(
        native-lib
        SHARED
        native-lib.cpp
        ImageProcessor.cpp
        SimpleTextureRenderer.cpp)

# Find and include the NDK logging library (for android/log.h)
find_library(
        log-lib
        log)
find_library(
        gles-lib
        GLESv2)
find_library(
        egl-lib
        EGL)

# Link the libraries to our native-lib
target_link_libraries(
        native-lib
        ${gles-lib}
        ${egl-lib}
        ${OpenCV_LIBS}
        ${log-lib})