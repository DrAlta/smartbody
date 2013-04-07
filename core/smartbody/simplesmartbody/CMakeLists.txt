
include_directories( 

	../smartbody-lib/src
	../smartbody-dll/include
)


set( SIMPLESMARTBODY_SRC

	simplesmartbody.cpp 
)

add_executable( simplesmartbody 

	${SIMPLESMARTBODY_SRC}
)

target_link_libraries( simplesmartbody 

	SmartBody
	pthread
	GLEW
#	tcmalloc
#	profiler
#	${CMAKE_REQUIRED_LIBRARIES} 
)

set( EXEC_LIST

	simplesmartbody
)

set( APP_LIBRARY_BINARY_PATHS

        ${PROJECT_BINARY_DIR}/core/smartbody/sbm/bin
)
	
set( APP_LIBRARY_SOURCE_PATHS

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
	TARGETS ${EXEC_LIST}
	RUNTIME DESTINATION core/smartbody/sbm/bin
)
