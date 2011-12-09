include_directories( /usr/local/include/OGRE )
include_directories( ../smartbody/smartbody-dll/include )
include_directories( ../../lib/bonebus/include )

add_executable( ogreviewer

	src/BoneBusHandler.cpp
	src/OgreFrameListener.cpp
	src/OgreRenderer.cpp
	src/SBListener.cpp
	src/main.cpp
)

target_link_libraries( ogreviewer

	smartbody-dll
	xerces-c
	bonebus
	vhcl
	vhmsg	
	activemq-cpp
	OgreMain
	OIS
	alut
	sndfile
)

set( EXEC_LIST

       ogreviewer 
	)

set( APP_LIBRARY_SOURCE_PATHS

        ${PROJECT_SOURCE_DIR}/core/ogre-viewer/bin
        ${PROJECT_SOURCE_DIR}/core/smartbody/sbm/bin
)

set_target_properties(

        ${EXEC_LIST}

	PROPERTIES

	## link with install tree:
	INSTALL_RPATH                   "${APP_LIBRARY_SOURCE_PATHS}"
	BUILD_WITH_INSTALL_RPATH        TRUE
)


install(
	TARGETS ogreviewer
	RUNTIME DESTINATION core/ogre-viewer/bin
)
	
