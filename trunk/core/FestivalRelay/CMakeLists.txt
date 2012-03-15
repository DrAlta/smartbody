include_directories( ../../lib/festival/festival/src/include )
include_directories( ../../lib/festival/festival/src/modules/VHDuration)
include_directories( ../../lib/festival/speech_tools/include )

find_library(FESTIVALLIB Festival ../../lib/festival/festival/src/lib )
find_library(ESTTOOLS estools ../../lib/festival/speech_tools/lib )
find_library(ESTSTRING eststring ../../lib/festival/speech_tools/lib )
find_library(ESTBASE estbase ../../lib/festival/speech_tools/lib )

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
	
