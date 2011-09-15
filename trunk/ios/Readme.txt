								INSTALL INSTRUCTION

--------------------------------Compiling using shell script------------------------------
1) Cross compiling activemq-cpp-library (dependent on apr&apr-util)
- Cross compiling apr
	http://archive.apache.org/dist/apr/
	Download apr-1.3.*, copy setup-iphoneos.sh and setup-iphonesimulator.sh from trunk/ios/activemq/apr to that folder
	Change the SBROOT inside setup-iphoneos.sh and setup-iphonesimulator.sh to your trunk directory
	Run both of the scripts
	Go to trunk/ios/activemq/apr/iphone*/include ,edit line 79 of apr_general.h to be: #if defined(CRAY) || (defined(__arm) && !(defined(LINUX) || defined(__APPLE__)))

- Cross compiling apr-util
	http://archive.apache.org/dist/apr/
	Download apr-util-1.3.*, copy setup-iphoneos.sh and setup-iphonesimulator.sh from trunk/ios/activemq/apr-util to that folder
	Change the SBROOT inside setup-iphoneos.sh and setup-iphonesimulator.sh to your trunk directory
	Run both of the scripts

- Cross compiling activemq-cpp-library
	http://apache.osuosl.org/activemq/activemq-cpp/source/
	Download activemq-cpp-library-3.4.0, copy setup-iphoneos.sh and setup-iphonesimulator.sh from trunk/ios/activemq/apr-util to that folder
	change src/main/decaf/lang/system.cpp line 471 inside activemq folder from "#if defined (__APPLE__)" to "#if 0"
	Change the SBROOT inside setup-iphoneos.sh and setup-iphonesimulator.sh to your trunk directory
	Run both of the scripts

Note: for smartbody iphone running on unity, we need to rename variables inside activemq-cpp-library decaf/internal/util/zip/*.c to avoid conflict symbols. If you don't want to do that, you can directly use the one under trunk/ios/activemq/activemq-cpp/libs/activemq-unity

2) Cross compiling xerces-c
	http://xerces.apache.org/xerces-c/download.cgi
	Download xerces-c-3.1.1.tar.gz, unzip it, copy setup-iphoneos.sh and setup-iphonesimulator.sh from trunk/ios/activemq/activemq-cpp to that folder
	Change the SBROOT inside setup-iphoneos.sh and setup-iphonesimulator.sh to your trunk directory
	Run both of the scripts
	
3) Cross compiling ode
	http://sourceforge.net/projects/opende/files/
	Download ode-0.11.1.zip, copy setup-iphoneos.sh and setup-iphonesimulator.sh from trunk/ios/activemq/activemq-cpp to that folder
	Change the SBROOT inside setup-iphoneos.sh and setup-iphonesimulator.sh to your trunk directory
	Run both of the scripts
	
--------------------------------Compiling using Xcode4------------------------------
4) Build bonebus
	Open smartbody-iphone.xcworkspace, select the scheme to be bonebus, build

5) Build boost
	http://www.boost.org/users/history/version_1_44_0.html
	Download boost_1_44_0.tar.gz, unzip to trunk/ios/boost. Make sure the folder name is boost_1_44_0.
	Open smartbody-iphone.xcworkspace, select boost_system, boost_filesystem, boost_regex, build them seperately.
	http://mathema.tician.de/news.tiker.net/download/software/boost-numeric-bindings/boost-numeric-bindings-20081116.tar.gz
	Download boost_numeric_bindings, unzip it to trunk/ios/boost, make sure the name is boost_numeric_bindings
	
6) Build steersuite
	Open smartbody-iphone.xcworkspace, select the steerlib, pprAI, build them seperately.

7) Build vhmsg
	Open smartbody-iphone.xcworkspace, select scheme vhmsg and build.
	
8) Build vhcl
	Since the vhcl_log.cpp hasn't been changed from VH group, you have to copy trunk/ios/vhcl/vhcl_log.cpp to trunk/lib/vhcl/src/vhcl_log.cpp for now.
	Open smartbody-iphone.xcworkspace, select scheme vhcl and build.

9) Build wsp
	Open smartbody-iphone.xcworkspace, select scheme wsp and build.
	
10) Build smartbody-lib
	Open smartbody-iphone.xcworkspace, select scheme smartbody-lib and build. 

11) Build smartbody-dll (For now, this is optional)
	Open smartbody-iphone.xcworkspace, select scheme smartbody-dll and build. 
	
12) Build vhwrapper-dll (For unity only, optional)
	Open smartbody-iphone.xcworkspace, select scheme vhwrapper-dll and build. 


--------------------------------Compiling and Running Applications using Xcode4------------------------------
There are two applications under trunk/ios/applications, you have to go over the previous steps. Make sure you have your device connected.
13) Build smartbody-openglES
	Go to trunk/ios/applications/minimal, open smartbody-iphone.xcodeproj, build and run.

Note: Under smartbody-openglES project Frameworks, you should see all the libraries existing. If not, go over previous steps to check if anything is wrong

14) Build smartbody-ogre
	http://www.ogre3d.org/download/sdk
	Download ogreSDK
	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Building%20From%20Source%20-%20iPhone&redirectpage=Building%20From%20Source%20(for%20iPhone)
	Go over this tutorial and build ogre iphone libraries. Make sure all the libraries exist inside build/lib/Debug
	Go to trunk/ios/applications/ogreIphone, open smartbody-ogre.xcodeproj, go to smartbody-ogre project, set OGRE_SDK_ROOT to your ogreSDK directory, set OGRE_SRC_ROOT to your ogre source directory. Select scheme smartbody-ogre, build and run.
	If the program hangs on boost thread function, try rebuild the ogre iphone dependencies boost libs (pthread, date_time), alternative way is to build the whole ogre iphone libraries with boost symbol turned off which may affect the results.
	
Note: ogre 1.8 seems to have trouble when building for iphone/ipad, use ogre 1.7.3. 
	  It is extremely slow running on armv6 ipod(after testing), and there's something wrong with the texture and shader. So maybe should just run on armv7 iphone/ipad.


15) Build smartbody-unity
	...

--------------------------------------------------------------------------------------------------------------
