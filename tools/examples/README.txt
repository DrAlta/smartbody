sudo apt-get install cmake
sudo apt-get install libc6-dev-i386
==
0. cd build-dir/
1. cmake -DCMAKE_INSTALL_PREFIX="." ../tut
2. make 
3. make install
==

cmake/tutorialA: simple example 
 - shows copying *.h or select set of header files
 - shows included set of .c source listed in separate file
 - compiling static or shared libs
 - use: mkdir build; cd build/; cmake ../tutorialA; make install
 - installs in ../install by default
 - Otherwise cmake -DCMAKE_INSTALL_PREFIX="../install" ../tutorialA

cmake/tutorialB: exercising features, clearing up formatting
 - builds LibGWIZ.a, installs lib and headers
 - builds and installs demo app: hermite_plot
 - use: cd tutorialB/build; cmake ..; make install; ../install/app/hermite_plot;
