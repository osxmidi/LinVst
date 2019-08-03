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

set( CONVERTTREEVST_SRC ../../../convert/linvstconvertgtktree.cpp)

add_executable( linvstconverttree ${CONVERTTREEVST_SRC} )
set_target_properties( linvstconverttree PROPERTIES COMPILE_FLAGS " ${CMAKE_CXX_FLAGS} " )
target_link_libraries( linvstconverttree ${GTK3_LIBRARIES} )
set_target_properties( linvstconverttree PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie " )

install( TARGETS linvstconverttree DESTINATION bin )
