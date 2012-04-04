#add_definitions ( -DUSE_GOOGLE_PROFILER )

include_directories(
	steersuite-1.3/steerlib/include
	steersuite-1.3/pprAI/include
	steersuite-1.3/external
	sbm-debugger/lib
	/usr/include/python2.6
)

add_subdirectory( smartbody-lib/src )
add_subdirectory( smartbody-dll )
add_subdirectory( vhwrapper )
add_subdirectory( steersuite-1.3 )

add_subdirectory( sbm-batch )
add_subdirectory( sbm )
