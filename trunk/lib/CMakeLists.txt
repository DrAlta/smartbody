
add_subdirectory( vhcl )
add_subdirectory( bonebus )
add_subdirectory( vhmsg/vhmsg-c )
#add_subdirectory( kinecttracker )

if(${USE_ASSIMP})
add_subdirectory( assimp-3.1.1)
endif()

