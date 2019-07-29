cmake_minimum_required(VERSION 3.9)

# General set up, must stay here.
set( CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix." )

# Base project setup.
project(LinVst-i386)

INCLUDE(FindPackageHandleStandardArgs)

message( "Configuring Project LinVst." )

SET(MAJOR_VERSION "2")
SET(MINOR_VERSION "23")
SET(PATCH_VERSION "0")

add_subdirectory( liblinvst )
add_subdirectory( exelinvst )

install(FILES ../convert/pylinvstconvert-386 DESTINATION ${SCRIPT1_INSTALL_PATH} bin
      PERMISSIONS OWNER_EXECUTE OWNER_READ
      GROUP_EXECUTE GROUP_READ
      WORLD_EXECUTE WORLD_READ )		  

#-----------------------------------------------------------------
# CPack configuration.
#-----------------------------------------------------------------

IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
INCLUDE(InstallRequiredSystemLibraries)
 
SET(CPACK_SET_DESTDIR "on")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
SET(CPACK_GENERATOR "DEB")
 
SET(CPACK_PACKAGE_DESCRIPTION "LinVst windows->linux vst bridge for 32 bits vsts.")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Wrapper for windows VST plugins to make them available on Linux.")
SET(CPACK_PACKAGE_VENDOR "osxmidi")
SET(CPACK_PACKAGE_CONTACT "abc@def.com")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386" )
 
#dependencies for this service menu
#SET(CPACK_DEBIAN_PACKAGE_DEPENDS "wine-stable")
 
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "audio")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
 
SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)
INCLUDE(CPack)
 
ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
