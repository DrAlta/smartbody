## http://snikt.net/index.php/2010/04/01/howto-use-cmake-with-cc-projects

project( test-package )
cmake_minimum_required( VERSION 2.6 )

#set( CMAKE_INSTALL_PREFIX "." )
set( CMAKE_INSTALL_PREFIX "../install" )

## packaging config
#include( CMake_package.txt )

####

add_definitions( -v -Wall -Wextra -pedantic -std=c99 -D_BSD_SOURCE )
#add_definitions( -DTEST_DEFINITION_ARG )

####

add_subdirectory( inc )
add_subdirectory( lib )

