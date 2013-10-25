cmake_minimum_required( VERSION 2.8 )
project( SmartBody )

set( CMAKE_INSTALL_PREFIX "../" )
set( BUILD_OGRE FALSE)
set( GOOGLE_PROFILER FALSE)
#add_definitions(	-D__linux__	-fPIC -m32 -Ignore -fpermissive -g3 )
#add_definitions(	-g3 -fPIC )
add_definitions(	-O3 -fPIC )
#add_definitions(	-v -W -Wall -Wextra -pedantic )

include_directories(

	/usr/local/include
	/usr/include
	/usr/X11/include
	/usr/include/xercesc

	lib/vhcl/include
	lib/vhmsg/vhmsg-c/include
	lib/bonebus/include
	lib/wsp/wsp/include
#	lib/boost
	/usr/local/include/boost

)

LINK_DIRECTORIES(
	/usr/local/lib
)

add_subdirectory( lib )
add_subdirectory( core )


