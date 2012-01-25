include_directories( 

	src
	../OpenNI/Include
)

find_package(OpenGL REQUIRED)


link_directories( 	../OpenNI/Lib
			../NITE/Lib
		)


set( KINECT_TRACKER_SRC

	src/main.cpp 
	src/SceneDrawer.cpp 
)

add_executable( kinecttracker

	${KINECT_TRACKER_SRC}
)

target_link_libraries( kinecttracker

	activemq-cpp
	OpenNI
	${OPENGL_LIBRARIES}
	glut
	vhmsg

)

set( EXEC_LIST

	kinecttracker
)

set( APP_LIBRARY_BINARY_PATHS

        ${PROJECT_BINARY_DIR}/core/smartbody/sbm/bin
)
	


set_target_properties(

        ${EXEC_LIST}

	PROPERTIES

	## link with install tree:
	INSTALL_RPATH                           "${APP_LIBRARY_SOURCE_PATHS}"
	BUILD_WITH_INSTALL_RPATH        TRUE
)

install( 
	TARGETS ${EXEC_LIST}
	RUNTIME DESTINATION lib/kinecttracker
)
