- How to generate sdk folder
Open terminal and direct to current folder, run the following command
./createOSXSDK.sh 
This will generate a sdk folder.

- What's inside sdk folder that's interesting to end user
"bin" folder contains ready-to-use execuables. Note that bin folder assumes that SmartBody is already built on macosx
startSimpleSmartBody.command will launch a simple SmartBody test program.
startSBGUI.command will launch standard SmartBody program.

If you want to build on your own instead of using executables out of box. You need to install cmake (Note that cmake 2.8.4 and 2.8.5 will give an error, maybe more versions will also give errors, this is not fully tested), so use latest cmake version 2.8.10+.

Once you have cmake ready. Run
./buildCMake.sh if you just want to run a standard cmake build process.


- Developer should know following issues
->If 3rd party libraries are updated, you will need to update libraries inside lib folder by installing the new ones, for xerces you need to install adding --disable-rpath option. After installing, you will also need to use install_name_tools to strip out the path name. Currently this is done to activemq, ode, fltk etc. To check which libraries need to be modified by install_name_tools, you can use otool. Following are articles that would help:
http://www.cmake.org/Wiki/CMake_RPATH_handling
Currently sdk have following version of 3rd party library: boost_1_51, xercds-c-3.1.1, ode-0.11.1, fltk-1.32
->Also when vhcl, vhmsg, bone bus, wsp change, the new built static libraries has to be copied to lib folder, when steer lib changes, the new built .dylib has to be copied to bin folder.

