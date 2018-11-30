cmake_minimum_required(VERSION 3.10)

project(LinVst)

INCLUDE(FindPackageHandleStandardArgs)

message( "Configuring Project LinVst.")

# Compile Flags.
################
SET(STATIC_LINKING FALSE )

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Wall -fPIC -O2 -DLVRT -DEMBED -DEMBEDDRAG -DWAVES -DVESTIGE ")

# Project Path.
###############
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set( CMAKE_CXX_COMPILER wineg++ )

set( LINVST_SVR_SRC
  ../lin-vst-server.cpp
  ../remotepluginserver.cpp
  ../paths.cpp
  )

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -O2 -DEMBED -DEMBEDDRAG -DWAVES -DWAVESLOOP -DVESTIGE -I/usr/include/wine-development/windows -I/usr/include/wine-development/wine/windows -I/usr/include/wine/wine/windows")

add_executable( lin-vst-server ${LINVST_SVR_SRC} )
target_link_libraries( lin-vst-server pthread rt )

