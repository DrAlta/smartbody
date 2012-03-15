include_directories( include )
include_directories( ../smartbody-lib/src )

find_package(OpenGL REQUIRED)
add_library(
	smartbody-dll
	SHARED 
	
	smartbody-dll.cpp
	smartbody-c-dll.cpp
)

target_link_libraries( smartbody-dll

	tcmalloc
	profiler
	smartbody-lib
	bonebus
	vhmsg
	vhcl
	wsp
	xerces-c
	boost_filesystem
	boost_system
	boost_regex
	boost_python
	blas
	lapack
	${OPENGL_LIBRARIES}
	GLEW
	log4cxx
	activemq-cpp
	steerlib
	pprAI
	ode
	python2.6
	SbmDebugger
)

set( EXEC_LIST

       smartbody-dll
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
	DESTINATION core/smartbody/sbm/bin
)

