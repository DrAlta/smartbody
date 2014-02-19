if (${GOOGLE_PROFILER})
	add_definitions ( -DUSE_GOOGLE_PROFILER )
endif()

include_directories(
	../../lib/vhcl/include
	../../lib/vhmsg/vhmsg-c/include
	../../lib/bonebus/include
	../../lib/wsp/wsp/include
	steersuite-1.3/steerlib/include
	steersuite-1.3/pprAI/include
	steersuite-1.3/external
	sbm-debugger/lib
	/usr/local/include
	/usr/include/python2.7
)

include_directories(
	/usr/local/python2.7
)


LINK_DIRECTORIES(
	/usr/local/lib
)


add_subdirectory( SmartBody )
add_subdirectory( steersuite-1.3 )

add_subdirectory( simplesmartbody)
add_subdirectory( sbgui )
#add_subdirectory( sbgui/external/Pinocchio )

