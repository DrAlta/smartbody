cmake_minimum_required( VERSION 2.8 )
project( SmartBody )

set( CMAKE_INSTALL_PREFIX "../" )
set( BUILD_OGRE FALSE)
set( GOOGLE_PROFILER FALSE)
set( BUILD_FESTIVAL FALSE)

# uses the Assimp asset importer
#set( SB_NO_ASSIMP FALSE)

# uses Python as a scripting language
#set( SB_NO_PYTHON TRUE)

# uses the Virtual Human Message system
#set( SB_NO_VHMSG TRUE)

# uses the bonebus network 
#set( SB_NO_BONEBUS TRUE)

# uses the Open Dynamics Engine (ODE) physics library
#set( SB_NO_ODE_PHYSICS TRUE)


#add_definitions(	-D__linux__	-fPIC -m32 -Ignore -fpermissive -g3 )
#add_definitions(	-g3 -fPIC )
add_definitions(	-O3 -fPIC )
#add_definitions(	-v -W -Wall -Wextra -pedantic )


if (${SB_NO_VHMSG})
add_definitions(-DSB_NO_VHMSG)
set (VHMSG_INCLUDE
)	
else()
set (VHMSG_INCLUDE
	lib/vhmsg/vhmsg-c/include
	/usr/local/include/activemq-cpp-3.8.3
	)
endif()

if (${SB_NO_BONEBUS})
add_definitions(-DSB_NO_BONEBUS)
set (BONEBUS_INCLUDE
)	
else()
set (BONEBUS_INCLUDE
	lib/bonebus/include
	)
endif()

if (${SB_NO_ASSIMP})
add_definitions(-DSB_NO_ASSIMP)
endif()

if (${SB_NO_PYTHON})
add_definitions(-DSB_NO_PYTHON)
endif()

if (${SB_NO_ODE_PHYSICS})
add_definitions(-DSB_NO_ODE_PHYSICS)
endif()

if (${SB_NO_ASSIMP})
add_definitions(-DSB_NO_ASSIMP)
endif()

include_directories(

	/usr/local/include
	/usr/include/xercesc
	${VHMSG_INCLUDE}
	${BONEBUS_INCLUDE}
	lib/vhcl/include
#	lib/boost
	/usr/local/include/boost

)


LINK_DIRECTORIES(
	/usr/local/lib
)

add_subdirectory( lib )
add_subdirectory( core )


