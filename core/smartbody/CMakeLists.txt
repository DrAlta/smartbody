add_definitions ( -DUSE_GOOGLE_PROFILER )

include_directories(
	steersuite-1.3/steerlib/include
	steersuite-1.3/pprAI/include
	steersuite-1.3/external
	sbm-debugger/lib
	/usr/include/python2.7
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
include_directories(
	/usr/local/python2.7
)
else()
include_directories(
	/usr/include/python2.7
)
endif()


#add_subdirectory( smartbody-lib/src )
#add_subdirectory( smartbody-dll )
add_subdirectory( SmartBody )
add_subdirectory( steersuite-1.3 )

add_subdirectory( simplesmartbody)
add_subdirectory( sbm )
