# cmake_minimum_required(VERSION 3.9)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

# Compile Flags.
################
SET(STATIC_LINKING FALSE )

set( CMAKE_CXX_STANDARD 14 )

# Project Path.
###############
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set( CMAKE_CXX_COMPILER /usr/bin/c++ )

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${GTK3_INCLUDE_DIRS})
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

set( CONVERTVST_SRC ../../convert-64bit-32bit/linvstconvertgtk.cpp)

add_executable( linvstconvert ${CONVERTVST_SRC} )
set_target_properties( linvstconvert PROPERTIES COMPILE_FLAGS " ${CMAKE_CXX_FLAGS} " )
target_link_libraries( linvstconvert ${GTK3_LIBRARIES} )
set_target_properties( linvstconvert PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie " )

install( TARGETS linvstconvert DESTINATION bin )
