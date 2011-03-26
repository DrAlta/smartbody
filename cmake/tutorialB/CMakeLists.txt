project( GWiz )
cmake_minimum_required( VERSION 2.6 )

set( CMAKE_INSTALL_PREFIX "../install" )

#add_definitions( -v -Wall -Wextra -Werror -pedantic -std=c99 -D_BSD_SOURCE )
add_definitions( -Wall -Wextra -pedantic )

add_subdirectory( inc )
add_subdirectory( src )
add_subdirectory( app )
