cmake_minimum_required( VERSION 2.8 )
project( SmartBody )
SET(CMAKE_SYSTEM_NAME Darwin)

set( CMAKE_INSTALL_PREFIX "../" )
#release
add_definitions(	-g3 -fPIC )
#debug
#add_definitions(	-fPIC )

include_directories(
	/System/Library/Frameworks/Python.framework/Headers/
	./include/vhcl
	./include/vhmsg
	./include/bonebus
	./include/wsp
	./include/steersuite
	./include/
)

LINK_DIRECTORIES(
	/usr/lib
	/System/Library/Frameworks/Python.framework
)

add_subdirectory( src )


