
include_directories( 

	../smartbody-lib/src
	../smartbody-dll/include
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
)

set( EXEC_LIST

	sbm-batch 
)

install( 
	TARGETS ${EXEC_LIST}
	RUNTIME DESTINATION core/smartbody/sbm-batch
)
