cmake_minimum_required( VERSION 2.8 )
project( SmartBody )
SET(CMAKE_SYSTEM_NAME Darwin)

set( CMAKE_INSTALL_PREFIX "../" )
set( CMAKE_OSX_DEPLOYMENT_TARGET "10.6" )
#debug
#add_definitions(	-g3 -fPIC )
#release
#add_definitions(	-fPIC )
#optimization
add_definitions( -O3	-fPIC )

include_directories(
	./include/
)

LINK_DIRECTORIES(
	/usr/lib
)

add_subdirectory( src )


