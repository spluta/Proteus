
set(CMAKE_CXX_STANDARD 17)

cmake_minimum_required (VERSION 3.5)
project (Proteus)

include_directories(${SC_PATH}/include/plugin_interface)
include_directories(${SC_PATH}/include/common)
include_directories(${SC_PATH}/external_libraries/libsndfile/)

set(CMAKE_SHARED_MODULE_PREFIX "")
if(APPLE OR WIN32)
set(CMAKE_SHARED_MODULE_SUFFIX ".scx")
endif()

find_package(nlohmann_json 3.2.0 REQUIRED)

# add the plugin we are making
add_library(Proteus MODULE Proteus.cpp)

# add the submodule
add_subdirectory(RTNeural)

# this is the location of libsamplerate.dylib, which you need to download and build separately
target_link_libraries(Proteus ${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate/build/src/libsamplerate.dylib)
target_link_libraries(Proteus RTNeural)
target_link_libraries(Proteus nlohmann_json::nlohmann_json)

set_property(TARGET Proteus PROPERTY CXX_STANDARD 17)

# cmake -DCMAKE_BUILD_TYPE=Release -DSC_PATH='/Users/spluta1/Documents/SC/SuperColliderBuild/SuperCollider' ..
# cmake --build . --config Release