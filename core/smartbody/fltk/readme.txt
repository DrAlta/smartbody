
This folder contains fltk binaries needed
by the Motion Engine and SymEdge libs in
windows version.

For linux, fltk should be installed independently,
with the makefiles provided in the fltk distribution.

================================================
   The correct set of fltk libraries should be
   manually unziped in folder fltk\lib\
================================================

Current FLTK version is: FLTK 1.1.7

The used fltk compilation steps, for upgrade to future
versions, are:

A) BUILD FLTK LIBS: fltk.lib fltkd.lib fltkc.lib
1. download fltk 1.1.6 (.zip) version from www.fltk.org
2. goto fltk-1.1.6/visualc/ and open fltk.dsw
   (not tried .sln in vcnet/ folder)
3. when VisualC++ .NET asks to convert, say "Yes To All"
4. copy the .cxx files from project fltk_gl to fltk
5. for project fltk, create new configuration
   "Compact" as copy of "Release", and edit properties:
   - C/C++->output files: .\Compact\ (instead of Release)
   - C/C++->code gen->runtime lib: multi-threaded(/MT)
   - librarian->general->output file: change to fltkc.lib
6. now you can compile: fltk.lib fltkd.lib fltkc.lib

B) BUILD FLUID IN COMPACT MODE
(**OR: build in release mode with no changes)
 1. check fluid dependencies, it should be:
    fltk, fltkforms, fltkimages, libpng, zlib
 2. change Compact configurations of:
    fltk, fltkforms, fltkimages, libpng, zlib; with:
    - C/C++->code gen->runtime lib: multi-threaded(/MT)
 3. change fluid config to Compact and edit properties:
    - C/C++->code gen->runtime lib: multi-threaded(/MT)
    - Linker->Input->Ad.Dependencies: fltkc.lib
    - Linker->Input->Ignore Lib: MSVCRT.lib
 4. build fluid in compact configuration (warnings ok)

C) COPY FILES TO MOTION ENGINE FOLDER
 1. copy headers in FL\* to fltk\FL\
 2. copy fltk.lib, fltkd.lib and fltkc.lib in lib\
    to fltk\lib and zip them in vc7libs.zip
    (or in vc6libs.zip if using old visualc)
 3. copy fluid\fluid.exe to fltk\fluid\ and zip it
 4. to avoid warnings in Fl_Window.H:
    - replace (short) to (uchar) in line 91

NOTE ABOUT THE COMPACT CONFIGURATION:
The idea of the compact configuration is that all
libraries which are needed are stored ("compacted")
in the .exe, therefore leaving no dependencies on
.dlls at run time. The reason for this is to ensure
that executables can run in any windows machine, 
otherwise they remain dependent on the visualc dlls
of the same version used during compilation.

-----------------------------------------------------
 First tests with FLTK 2.0 showed that version 2.0
 is not complete enought and porting the code from
 1.1.6 to 2.0 is not very simple
-----------------------------------------------------

