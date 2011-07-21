
## for copying all headers:
#install(
#	DIRECTORY		""
#	DESTINATION		inc
#	FILES_MATCHING	PATTERN "*.h"
#	PATTERN 		".svn" EXCLUDE
#)

## for specific identification:
set( 
	EXAMPLE_LIB_HDR
	lib_a.h
	lib_b.h
)

install( 
	FILES			${EXAMPLE_LIB_HDR}
#	DIRECTORY		""
	DESTINATION		inc 
)
