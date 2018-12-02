
find_package( PkgConfig REQUIRED )

pkg_check_modules ( GTK gtk+-3.0 )

if( NOT GTK_FOUND )
  pkg_check_modules ( GTK gtk+-2.0 )  
endif()
  
if( GTK_FOUND ) 
  message ("Compiling convert utilities with gui." )

  set( CMAKE_CXX_STANDARD 14 )

  add_executable( linvstconvert-gui linvstconvertgtk.cpp )
  target_include_directories( linvstconvert-gui PUBLIC ${GTK_INCLUDE_DIRS} )
  target_link_libraries( linvstconvert-gui ${GTK_LIBRARIES} )
  target_compile_options(linvstconvert-gui PUBLIC ${GTK_CFLAGS_OTHER})

  add_executable( linvstconverttree-gui linvstconvertgtktree.cpp )
  target_include_directories( linvstconverttree-gui PUBLIC ${GTK_INCLUDE_DIRS} )
  target_link_libraries( linvstconverttree-gui ${GTK_LIBRARIES} )
  target_compile_options(linvstconverttree-gui PUBLIC ${GTK_CFLAGS_OTHER})

  install( TARGETS linvstconvert-gui linvstconverttree-gui DESTINATION bin )
  
endif()

message ("Compiling command line convert utilities." )

set( CMAKE_CXX_STANDARD 14 )
add_executable( linvstconvert linvstconvert.cpp )

install( TARGETS linvstconvert DESTINATION bin )

