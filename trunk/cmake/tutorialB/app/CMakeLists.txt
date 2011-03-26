
include_directories( ../inc )

set( HERMITE_SRC

	hermite_plot.cpp 
	sr_linear_curve.cpp 
	sr_spline_curve.cpp 
)

add_executable( hermite_plot 

	${HERMITE_SRC}
)

target_link_libraries( hermite_plot 

	GWIZ
	glut 
	GLU 
	GL 
	${CMAKE_REQUIRED_LIBRARIES} 
)

set( APP_EXE_LIST

	hermite_plot
)

install( 
	TARGETS ${APP_EXE_LIST}
	RUNTIME DESTINATION app 
)
