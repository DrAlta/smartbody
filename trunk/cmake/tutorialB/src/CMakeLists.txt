# define sources

include( CMake_lib_src.txt )

include_directories( ../inc )

add_library( GWIZ 
	STATIC 
#	SHARED 
	${GWIZ_LIB_SRC} 
)

target_link_libraries( GWIZ 
	${CMAKE_REQUIRED_LIBRARIES} 
)

install( 
	TARGETS GWIZ
	ARCHIVE DESTINATION lib 
#	LIBRARY DESTINATION lib 
)
