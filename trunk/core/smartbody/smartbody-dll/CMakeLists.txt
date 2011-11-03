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
)
