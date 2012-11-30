#ifndef _SBTYPES_H_
#define _SBTYPES_H_

#ifdef WIN32
#ifdef SB_EXPORTS
#define SBAPI __declspec(dllexport)
#else
#define SBAPI __declspec(dllimport)
#endif
#else
#define SBAPI 
#endif


#endif

