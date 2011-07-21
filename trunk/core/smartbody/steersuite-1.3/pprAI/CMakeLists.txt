include_directories( "." )


add_library(
        pprAI
        SHARED
	
	src/PPRAIModule.cpp
	src/PPRAgent.cpp
)

target_link_libraries( pprAI

	steerlib
)

install(

  	TARGETS pprAI
	LIBRARY DESTINATION core/smartbody/sbm/bin
)

