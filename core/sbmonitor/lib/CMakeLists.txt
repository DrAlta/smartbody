include_directories( "." )


####

set( SBM_DEBUGGER_LIB_SRC

	SbmDebuggerCommon.cpp
	SbmDebuggerClient.cpp

)

####


add_library(
	SbmDebugger
	STATIC

	${SBM_DEBUGGER_LIB_SRC}
)
