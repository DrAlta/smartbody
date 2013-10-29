- How to generate sdk folder
Open terminal and direct to current folder, run the following command
./createLinuxSDK.sh 
This will generate a sdk folder.

- What's inside sdk folder
"bin" folder contains ready-to-use execuables. Note that bin folder assumes that SmartBody is already built on linux
./simplesmartbody will launch a simple SmartBody test program.
./sbgui will launch standard SmartBody program.

If you want to build on your own instead of using executables out of box. You need to install cmake (Note that cmake 2.8.4 and 2.8.5 will give an error, maybe more versions will also give errors, this is not fully tested), so use latest cmake version 2.8.10+.

Also for linux user, you need to download following 3rd party libaries:
boost_1_51
xerces-c-3.1.1
ode-0.11.1
fltk-1.32

Once you have cmake ready. Run
./buildCMake.sh if you just want to run a standard cmake build process.

