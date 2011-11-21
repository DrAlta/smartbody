include_directories( ../../lib/festival/src/include )
include_directories( ../../lib/festival/src/modules/VHDuration)
include_directories( ../../lib/speech_tools/include )

find_library(FESTIVALLIB Festival ../../lib/festival/src/lib )
find_library(ESTTOOLS estools ../../lib/speech_tools/lib )
find_library(ESTSTRING eststring ../../lib/speech_tools/lib )
find_library(ESTBASE estbase ../../lib/speech_tools/lib )

add_executable( festivalrelay

	src/FestivalRelay.cpp
)

target_link_libraries( festivalrelay

	${FESTIVALLIB}
	${ESTTOOLS}
	${ESTSTRING}
	${ESTBASE}
	xerces-c
	vhcl
	vhmsg	
	activemq-cpp
)

install(
	TARGETS festivalrelay
	RUNTIME DESTINATION core/FestivalRelay
)
	
