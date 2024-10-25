cmake_minimum_required(VERSION 3.5.0)
project(streamout_layer VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(external/OpenXR-SDK)
include_directories(include external/OpenXR-SDK/include)

set(SOURCES src/api_layer_streamout.cpp)

add_library(streamout_layer SHARED ${SOURCES})
add_dependencies(streamout_layer openxr_loader)
 