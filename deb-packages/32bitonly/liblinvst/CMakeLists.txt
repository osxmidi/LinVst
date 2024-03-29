
# Compile Flags.
################
SET(STATIC_LINKING FALSE )

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fPIC -O2 -DLVRT -DVST32 -DEMBED -DEMBEDDRAG -DWAVES -DVESTIGE -DNEWTIME -DINOUTMEM -DCHUNKBUF -DEMBEDRESIZE -DPCACHE")

# Project Path.
###############
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../../)

set( CMAKE_CXX_COMPILER /usr/bin/c++ )

set( LINVST_SO_SRC SHARED
  ../../../linvst.cpp
  ../../../deb-packages/remotevstclientpkg.cpp
  ../../../remotepluginclient.cpp
  ../../../paths.cpp
  )

add_library(linvst ${LINVST_SO_SRC} )
set_target_properties(linvst PROPERTIES PREFIX "")
target_link_libraries( linvst pthread dl X11 )

install( TARGETS linvst DESTINATION share/LinVst/32bit )
