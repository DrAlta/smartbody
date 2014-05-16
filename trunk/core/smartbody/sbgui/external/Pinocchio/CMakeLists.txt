cmake_minimum_required (VERSION 2.8)

project(Pinocchio)


include_directories( . )

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   add_definitions(-fPIC)
endif()

# osx c++ version >= clang 4.2.0
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
		if (NOT (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.2.0))
			add_definitions(-Wno-format)
		endif()
	endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
		if (NOT (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.7.3))
			add_definitions(-Wno-deprecated)
			add_definitions(-Wno-format)
		endif()
	endif()
endif()

set ( SRC
	./attachment.cpp
	./discretization.cpp	
	./embedding.cpp	
	./graphutils.cpp	
	./indexer.cpp	
	./intersector.cpp	
	./lsqSolver.cpp	
	./matrix.cpp	
	./mesh.cpp	
	./Pinocchio.cpp	
	./pinocchioApi.cpp	
	./refinement.cpp	
	./skeleton.cpp	
)

add_library( pinocchio
   STATIC 
   ${SRC}
)

