# define sources

include( CMake_lib_src.txt )

include_directories(. ../inc )

add_library( exampleLib STATIC ${EXAMPLE_LIB_SRC} )
#add_library( exampleLib SHARED ${EXAMPLE_LIB_SRC} )
target_link_libraries( exampleLib ${CMAKE_REQUIRED_LIBRARIES} )

## What is for?
#set( CMAKE_REQUIRED_LIBRARIES "exampleLib; ${CMAKE_REQUIRED_LIBRARIES}" )

install( 
	TARGETS exampleLib
	LIBRARY DESTINATION lib 
	ARCHIVE DESTINATION lib 
)
