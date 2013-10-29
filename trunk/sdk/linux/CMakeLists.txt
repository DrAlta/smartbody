cmake_minimum_required( VERSION 2.8 )
project( SmartBody )

set( CMAKE_INSTALL_PREFIX "../" )
#debug
#add_definitions(	-g3 -fPIC )
#release
#add_definitions(	-fPIC )
#optimization
add_definitions( -O3	-fPIC )

include_directories(
	./include/vhcl
	./include/vhmsg
	./include/bonebus
	./include/wsp
	./include/steersuite
	./include
	/usr/include
	/usr/local/include
	/usr/include/python2.7
)

LINK_DIRECTORIES(
	/usr/lib
	/usr/local/lib
)

add_subdirectory( src )


