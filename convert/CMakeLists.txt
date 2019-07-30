# cmake_minimum_required(VERSION 3.9)

set( CMAKE_INSTALL_PREFIX "/usr" )

find_package( PkgConfig REQUIRED )

pkg_check_modules ( GTK gtk+-3.0 )

if( NOT GTK_FOUND )
  pkg_check_modules ( GTK gtk+-2.0 )  
endif()
  
if( GTK_FOUND ) 
  message ("Compiling convert utilities with gui." )

  set( CMAKE_CXX_STANDARD 14 )
  
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie" )
  
  add_executable( linvstconvert linvstconvertgtk.cpp )
  target_include_directories( linvstconvert PUBLIC ${GTK_INCLUDE_DIRS} )
  target_link_libraries( linvstconvert ${GTK_LIBRARIES} )
  target_compile_options(linvstconvert PUBLIC ${GTK_CFLAGS_OTHER})

  add_executable( linvstconverttree linvstconvertgtktree.cpp )
  target_include_directories( linvstconverttree PUBLIC ${GTK_INCLUDE_DIRS} )
  target_link_libraries( linvstconverttree ${GTK_LIBRARIES} )
  target_compile_options(linvstconverttree PUBLIC ${GTK_CFLAGS_OTHER})

  install( TARGETS linvstconvert linvstconverttree DESTINATION bin )
  
  install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/linvstconvert ${CMAKE_CURRENT_BINARY_DIR}/linvstconverttree DESTINATION bin
      PERMISSIONS OWNER_EXECUTE OWNER_READ 
      GROUP_EXECUTE GROUP_READ
      WORLD_EXECUTE WORLD_READ )  
  
endif()

message ("Compiling command line convert utilities." )

set( CMAKE_CXX_STANDARD 14 )
add_executable( linvstconvert-cli linvstconvert.cpp )

  install( TARGETS linvstconvert-cli DESTINATION bin )

  install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/linvstconvert-cli DESTINATION bin
      PERMISSIONS OWNER_EXECUTE OWNER_READ 
      GROUP_EXECUTE GROUP_READ
      WORLD_EXECUTE WORLD_READ ) 


