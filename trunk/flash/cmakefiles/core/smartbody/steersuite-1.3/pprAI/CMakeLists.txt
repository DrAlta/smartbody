include_directories( "." )


add_library(
        pprAI
	STATIC
	src/PPRAIModule.cpp
	src/PPRAgent.cpp
)

target_link_libraries( pprAI

	steerlib
)