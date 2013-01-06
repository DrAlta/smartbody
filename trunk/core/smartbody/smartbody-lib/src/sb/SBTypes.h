#ifndef _SBTYPES_H_
#define _SBTYPES_H_

#ifdef WIN32

	#ifdef SB_EXPORTS
		#define SBAPI __declspec(dllexport)
	#else
		#define SBAPI __declspec(dllimport)
	#endif

#else

	#if __GNUC__ >= 4
		#ifdef SB_EXPORTS
			#define SBAPI __attribute__ ((visibility ("default")))
		#else
			#define SBAPI 
		#endif
	#else
		#define SBAPI 
	#endif
#endif

#endif

