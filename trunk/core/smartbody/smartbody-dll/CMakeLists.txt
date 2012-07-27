include_directories( include )
include_directories( ../smartbody-lib/src )

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
find_library (COREAUDIO_LIBRARY CoreAudio)
find_library (AUDIOUNIT_LIBRARY AudioUnit)
find_library (CORESERVICES_LIBRARY CoreServices)
find_library (LIBSNDFILE sndfile )
find_library (ALUT alut )
find_library (OPENAL openal)
find_library (FESTIVAL Festival ../../../lib/festival/festival/src/lib)
find_library (ESTBASE estbase ../../../lib/festival/speech_tools/lib)
find_library (ESTSTRING eststring ../../..//lib/festival/speech_tools/lib)
find_library (ESTOOLS estools ../../../lib/festival/speech_tools/lib)
else()
find_library (LIBSNDFILE sndfile )
find_library (ALUT alut )
find_library (OPENAL openal)
endif()


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	if (${CMAKE_C_COMPILER_VERSION} VERSION_GREATER 4.2.0)
		add_definitions(-Wno-unused-value)
	endif()
endif()


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set( MORE_LIBS
	${COREAUDIO_LIBRARY}
	${AUDIOUNIT_LIBRARY}
	${CORESERVICES_LIBRARY}
	${LIBSNDFILE}
	${ALUT}
	${OPENAL}
	${FESTIVAL}
	${ESTBASE}
	${ESTSTRING}
	${ESTOOLS}
)
else()
set( MORE_LIBS
	${LIBSNDFILE}
	${ALUT}
	${OPENAL}
)       
endif()


find_package(OpenGL REQUIRED)
add_library(
	smartbody-dll
	SHARED 
	
	smartbody-dll.cpp
	smartbody-c-dll.cpp
)

target_link_libraries( smartbody-dll

#	tcmalloc
#	profiler
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
	activemq-cpp
	steerlib
	pprAI
	ode
	python2.6
	ncurses
	${MORE_LIBS}
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

