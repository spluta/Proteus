set(CMAKE_CXX_STANDARD 17)

if (NOT MSVC)
  # specify the C++ standard
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" CACHE STRING "c++ version" FORCE)
  if(CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

cmake_minimum_required (VERSION 3.5)
project (Proteus)

# add_subdirectories manually
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate ${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate/build)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/RTNeural RTNeural)

include_directories(${SC_PATH}/include/plugin_interface)
include_directories(${SC_PATH}/include/common)

set(CMAKE_SHARED_MODULE_PREFIX "")
if(APPLE OR WIN32)
set(CMAKE_SHARED_MODULE_SUFFIX ".scx")
endif()

# add the plugin we are making
add_library(${PROJECT_NAME} MODULE Proteus.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

set(LIBSR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate/build/src")
set(LIBSRI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate/include")

message(STATUS "LIBSR_PATH: ${LIBSR_PATH}")

include_directories(${LIBSR_PATH})
include_directories(${LIBSRI_PATH})

target_link_libraries(${PROJECT_NAME} RTNeural)

option(LIBSAMPLERATE_EXAMPLES "libsamplerate: build examples" OFF) 
option(BUILD_TESTING "libsamplerate: run testing" OFF)

if(WIN32)
        target_link_libraries(${PROJECT_NAME} ${LIBSR_PATH}/Release/samplerate.lib)
else()
        target_link_libraries(${PROJECT_NAME} ${LIBSR_PATH}/libsamplerate.a)
endif()

# set installation directory
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}")

message(STATUS "Install directory set to: ${CMAKE_INSTALL_PREFIX}")

install(DIRECTORY "${PROJECT_NAME}/Classes" "${PROJECT_NAME}/HelpSource" DESTINATION "${CMAKE_INSTALL_PREFIX}")

install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/scx_files")

message(STATUS "CMake current binary directory: ${CMAKE_CURRENT_BINARY_DIR}")

if(WIN32)
  install(FILES ${CMAKE_BINARY_DIR}/Release/${PROJECT_NAME}.scx DESTINATION ${CMAKE_INSTALL_PREFIX}/scx_files)
else()
  install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.scx DESTINATION ${CMAKE_INSTALL_PREFIX}/scx_files)
endif()









# set(CMAKE_CXX_STANDARD 17)

# cmake_minimum_required (VERSION 3.5)
# project (Proteus)

# include_directories(${SC_PATH}/include/plugin_interface)
# include_directories(${SC_PATH}/include/common)
# include_directories(${SC_PATH}/external_libraries/libsndfile/)

# set(CMAKE_SHARED_MODULE_PREFIX "")
# if(APPLE OR WIN32)
# set(CMAKE_SHARED_MODULE_SUFFIX ".scx")
# endif()

# find_package(nlohmann_json 3.2.0 REQUIRED)

# # add the plugin we are making
# add_library(Proteus MODULE Proteus.cpp)

# # add the submodule
# add_subdirectory(RTNeural)

# # this is the location of libsamplerate.dylib, which you need to download and build separately
# target_link_libraries(Proteus ${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate/build/src/libsamplerate.a)
# target_link_libraries(Proteus RTNeural)
# target_link_libraries(Proteus nlohmann_json::nlohmann_json)

# set_property(TARGET Proteus PROPERTY CXX_STANDARD 17)

# # cmake -DCMAKE_BUILD_TYPE=Release -DSC_PATH='/Users/spluta1/Documents/SC/SuperColliderBuild/SuperCollider' ..
# # cmake --build . --config Release