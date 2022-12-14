# Find PolyVox includes and library
#
# This module defines
#  PolyVox_INCLUDE_DIRS
#  PolyVox_LIBRARIES, the libraries to link against to use OGRE.
#  PolyVox_LIBRARY_DIRS, the location of the libraries
#  PolyVox_FOUND, If false, do not try to use OGRE
#
# Copyright © 2008, Matt Williams
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageMessage)

get_filename_component(THIS_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
if(WIN32)
	set(PREFIX ${THIS_DIR}/..)
else()
	set(PREFIX ${THIS_DIR}/../../..)
endif()


set(PolyVoxCore_LIBRARY_DIRS "${PREFIX}/PolyVoxCore/lib")
set(PolyVoxUtil_LIBRARY_DIRS "${PREFIX}/PolyVoxUtil/lib")
set(PolyVox_LIBRARY_DIRS "${PolyVoxCore_LIBRARY_DIRS}" "${PolyVoxUtil_LIBRARY_DIRS}")

set(PolyVoxCore_INCLUDE_DIRS "${PREFIX}/PolyVoxCore/include")
set(PolyVoxUtil_INCLUDE_DIRS "${PREFIX}/PolyVoxUtil/include")
set(PolyVox_INCLUDE_DIRS "${PolyVoxCore_INCLUDE_DIRS}" "${PolyVoxUtil_INCLUDE_DIRS}" "${PREFIX}/include")

set(PolyVoxCore_LIBRARIES "PolyVoxCore")
set(PolyVoxUtil_LIBRARIES "PolyVoxUtil")
set(PolyVox_LIBRARIES "${PolyVoxCore_LIBRARIES};${PolyVoxUtil_LIBRARIES}")

message(STATUS "Found PolyVox")
message(STATUS "  libraries : '${PolyVoxCore_LIBRARIES}' from ${PolyVoxCore_LIBRARY_DIRS}")
message(STATUS "            : '${PolyVoxUtil_LIBRARIES}' from ${PolyVoxUtil_LIBRARY_DIRS}")
message(STATUS "  includes  : ${PolyVox_INCLUDE_DIRS}")
