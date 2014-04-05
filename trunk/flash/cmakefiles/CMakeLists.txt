cmake_minimum_required( VERSION 2.8 )
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
project( SmartBody )

set( CMAKE_INSTALL_PREFIX "../" )
set( BUILD_OGRE FALSE)
set( FLASCC_SDK "C:/users/shapiro/crossbridge/sdk")

add_definitions(-O4)

#add_definitions(	-D__linux__	-fPIC -m32 -Ignore -fpermissive -g3 )
#add_definitions(	-g3 -fPIC )
add_definitions(	-fPIC )
#add_definitions(	-v -W -Wall -Wextra -pedantic )

include_directories(
	${FLASCC_SDK}/usr/local/include
	${FLASCC_SDK}/usr/local/include/boost
	
	lib/vhcl/include
	lib/vhmsg/vhmsg-c/include
	lib/bonebus/include
	lib/wsp/wsp/include		
)

LINK_DIRECTORIES(
	${FLASCC_SDK}/usr/local/lib
)

#add_subdirectory( lib )
add_subdirectory( core )


