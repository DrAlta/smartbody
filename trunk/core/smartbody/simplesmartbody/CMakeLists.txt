
include_directories( 

	../smartbody-lib/src
	../smartbody-dll/include
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
find_library (COREAUDIO_LIBRARY CoreAudio)
find_library (AUDIOUNIT_LIBRARY AudioUnit)
find_library (CORESERVICES_LIBRARY CoreServices)
find_library (LIBSNDFILE sndfile )
find_library (ALUT alut )
find_library (OPENAL openal)
find_library (FESTIVAL Festival ../../lib/festival/festival/src/lib)
find_library (ESTBASE estbase ../../lib/festival/speech_tools/lib)
find_library (ESTSTRING eststring ../../lib/festival/speech_tools/lib)
find_library (ESTOOLS estools ../../lib/festival/speech_tools/lib)
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



set( SIMPLESMARTBODY_SRC

	simplesmartbody.cpp 
)

add_executable( simplesmartbody 

	${SIMPLESMARTBODY_SRC}
)

target_link_libraries( simplesmartbody 

	smartbody-dll
	activemq-cpp
#	tcmalloc
#	profiler
#	${CMAKE_REQUIRED_LIBRARIES} 
	ncurses
	pthread
	${MORE_LIBS}
)

set( EXEC_LIST

	sbm-batch 
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
	RUNTIME DESTINATION core/smartbody/simplesmartbody
)
