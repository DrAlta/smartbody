
include_directories( 

	../smartbody-lib/src
	../smartbody-dll/include
)

link_directories( /home/shapiro/smartbody/trunk/lib/festival/src/lib
                  /home/shapiro/smartbody/trunk/lib/speech_tools/lib
		                  )


set( SBM_BATCH_SRC

	sbm-batch.cpp 
)

add_executable( sbm-batch 

	${SBM_BATCH_SRC}
)

target_link_libraries( sbm-batch 

	smartbody-dll
	activemq-cpp
	tcmalloc
	profiler
#	${CMAKE_REQUIRED_LIBRARIES} 
	#Festival
        estools
	estbase
	eststring
	ncurses
	sndfile
	openal
	alut
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
	RUNTIME DESTINATION core/smartbody/sbm-batch
)
