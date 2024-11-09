cmake_minimum_required(VERSION 3.5.0)
project(streamout_layer VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/OpenXR-SDK)
include_directories(include external/OpenXR-SDK/include)
set(SOURCES src/api_layer_streamout.cpp
)

set(LOCAL_HEADERS include/simulatedLayer.h include/compatibility.h)
 
add_library(streamout_layer SHARED ${SOURCES} ${LOCAL_HEADERS})
add_dependencies(streamout_layer openxr_loader)
     
#include("I:/vcpkg/scripts/buildsystems/vcpkg.cmake")
find_package(Boost CONFIG REQUIRED COMPONENTS beast)
find_package(OpenGL)
target_link_libraries(streamout_layer PRIVATE Boost::beast ${OPENGL_LIBRARIES})
 

configure_file(XrApiLayer_streamout.json.in ${LIBRARY_OUTPUT_DIRECTORY}/Debug/XrApiLayer_streamout.json)
