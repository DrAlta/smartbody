#add_definitions(-O4)
add_definitions(-flto-api=exports.txt)
set(LINKER_FLAG_SWF "-emit-swf")
set(LINKER_FLAG_PTHREAD "-pthread")
#aaa
#set(LINKER_FLAG_DATA "${FLASCC_SDK}/usr/local/lib/libGL.abc VFS.abc ${FLASCC_SDK}/usr/lib/AlcVFSzip.abc -symbol-abc=Console.abc -swf-size=500x500")
set(LINKER_FLAG_DATA "${FLASCC_SDK}/usr/local/lib/libGL.abc ${FLASCC_SDK}/usr/lib/AlcVFSzip.abc -swf-preloader=PreLoader.swf -symbol-abc=Console.abc -swf-size=800x600")
set(LINKER_MEMORY -jvmopt="-Xmx6000M")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_MEMORY} ${LINKER_FLAG_SWF} ${LINKER_FLAG_PTHREAD} ${LINKER_FLAG_DATA} ")

include_directories( 
	.
	../SmartBody/src
	#../smartbody-dll/include
)


set( SIMPLESMARTBODY_SRC

	simplesmartbody.cpp
)

add_executable( simplesmartbody 

	${SIMPLESMARTBODY_SRC}
)

target_link_libraries( simplesmartbody 

	SmartBody
	GL
	glut
	Flash++
	AS3++
	#pthread
)

set( EXEC_LIST

	simplesmartbody
)

set( APP_LIBRARY_BINARY_PATHS

        ${PROJECT_BINARY_DIR}/core/smartbody/sbgui/bin
)
	
set( APP_LIBRARY_SOURCE_PATHS

        ${PROJECT_SOURCE_DIR}/core/smartbody/sbgui/bin
)
	

set_target_properties(

        ${EXEC_LIST}

	PROPERTIES
	SUFFIX ".swf"
	## link with install tree:
	INSTALL_RPATH                   "${APP_LIBRARY_SOURCE_PATHS}"
	BUILD_WITH_INSTALL_RPATH        TRUE
)

install( 
	TARGETS ${EXEC_LIST}
	RUNTIME DESTINATION core/smartbody/sbgui/bin
)
