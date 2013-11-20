11/12/13
Ari Shapiro, Ph.D.

Download
----------
The linux SDK is available for download from the SmartBody site:

http://smartbody.ict.usc.edu/download


Build instructions for SmartBody on linux:

The SmartBody SDK for linux is in source form and requires a number of supporting libraries to build
properly. A full description is located in the SmartBody Manual (SmartBodyManual.pdf). Here's a summary when using Ubuntu:

Preparation
-----------------

# packages needed for basic build 
sudo apt-get install cmake g++ libxerces-c3-dev libgl1-mesa-dev libglu1-mesa-dev xutils-dev libxi-dev freeglut3-dev libglew-dev libxft-dev libapr1-dev libaprutil1-dev libcppunit-dev liblapack-dev libblas-dev libf2c2-dev build-essential python-dev libopenal-dev libsndfile-dev libalut-dev libfltk1.3-dev libboost-all-dev libode-dev

#packages needed for Ogre rendering
sudo apt-get install libzzip-dev libxaw7-dev libxxf86vm-dev libxrandr-dev libfreeimage-dev nvidia-cg-toolkit libois-dev libogre-dev ogre-1.8-samples-data


Build
-------------------
The build uses the cmake system as follows:

# make a directory called 'mybuild' and generate the makefiles for SmartBody there
mkdir mybuild
cd mybuild
cmake ..

# do the build
make install
# alternatively, you can build using multiple threads like:
# make -j8 install


Running SmartBody
--------------------
SmartBody can be used as a library, or run directory from the front-end application called sbgui 
located in the bin/ directory:

# running sbgui
cd bin
./sbgui

There is an example of using the Ogre renderer with SmartBody called ogresmartbody which can be run:
cd bin
./ogresmartbody

Please feel free to post comments and questions on the SmartBody forum:

http://smartbody.ict.usc.edu/forum



3rd party licenses
-------------------
A list of 3rd party licenses is located in the file:
3rd party licenses.txt
The entire SmartBody distribution is available for download from SourceForge:
svn checkout svn://svn.code.sf.net/p/smartbody/code/sdk smartbodysdk
B
A
o
