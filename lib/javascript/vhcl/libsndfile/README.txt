2015/7/16
Zerngui Wang

libsndfile library building
Version: 1.0.25
Platform: Linux

Instruction:
1) Download package and unzip
2) Run command: $emconfigure ./configure
3) Copy and Paste config.h and sndfile.h to ./src
4) Eddit sndfile.c in the function SNDFILE* sf_open (const char *path, int mode, SF_INFO *sfinfo)
   Comment out the assert(sizeof(sf_count_t) == 8);
5) Run command: $emmake make
6) Complied libs location:
	./src/.libs/libcommon.a
	./src/.libs/libsndfile.a
	
