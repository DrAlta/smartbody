
include_directories( ../inc )

add_executable( hermite_plot 

	hermite_plot.cpp 
	sr_linear_curve.cpp 
	sr_spline_curve.cpp 
)
target_link_libraries( hermite_plot 

	GWIZ glut GLU GL 
)

add_executable( decay_plot 

	decay_plot.cpp 
)
target_link_libraries( decay_plot 

	GWIZ glut GLU GL 
)

set( NOD_VIEWER_SRC

	nod_viewer.cpp 
)

add_executable( nod_viewer 

	${NOD_VIEWER_SRC}
)

target_link_libraries( nod_viewer 

	glut 
	GLU 
	GL 
	GWIZ
)

set( APP_EXE_LIST

	hermite_plot
	decay_plot
	nod_viewer
)

#set_target_properties( 

#	${APP_EXE_LIST} 

#	PROPERTIES 

## link with in-place source/binary tree:
#	INSTALL_RPATH				${PROJECT_SOURCE_DIR}/src/

## link with binary tree:
#	INSTALL_RPATH				${PROJECT_BINARY_DIR}/src/

## link with install tree:
#	INSTALL_RPATH				${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/lib/

#	BUILD_WITH_INSTALL_RPATH	TRUE
#)

install( 
	TARGETS ${APP_EXE_LIST}
	RUNTIME DESTINATION app 
)
