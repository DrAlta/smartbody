								INSTALL INSTRUCTION

--------------------------------Compiling using console---------------------------------------------
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
	
4) Cross compiling clapack (Optional since you can add Acceleration framework from xcode project)
	http://www.netlib.org/clapack/
	Download clapack-3.2.1-CMAKE.tgz, unzip and copy toolchain-iphone*.cmake and setup-iphone*.sh from trunk/ios/activemq/activemq-cpp to that folder
	To make a Unity compatible build, you need to modify the cmake file. If not used for Unity Iphone, you can skip the following step:
	-Go to clapack/CMakeLists.txt, comment out include(CTest) and add_subdirectory(TESTING)
	-Go to clapack/BLAS/CMakeLists.txt, comment out add_subdirectory(TESTING)
	-Go to clapack/F2CLIBS/libf2c/CMakeLists.txt, take out main.c from first SET so it became
			set(MISC 
			f77vers.c i77vers.c s_rnge.c abort_.c exit_.c getarg_.c iargc_.c
  			getenv_.c signal_.c s_stop.c s_paus.c system_.c cabs.c ctype.c
 			derf_.c derfc_.c erf_.c erfc_.c sig_die.c uninit.c)
 	-Go to clapack/SRC/CMakeLists.txt, take out ../INSTALL/lsame.c from first SET so it became
			set(ALLAUX  maxloc.c ilaenv.c ieeeck.c lsamen.c  iparmq.c	
   			 ilaprec.c ilatrans.c ilauplo.c iladiag.c chla_transtype.c 
    		../INSTALL/ilaver.c) # xerbla.c xerbla_array.c
    After doing this, change the SBROOT inside setup-iphoneos.sh and setup-iphonesimulator.sh, run the scripts according to what you are building to.

5) Cross compiling python
	Go to trunk/ios/python, modify the SBROOT inside setup-iphoneos.sh, then run the script.
	
6) Cross compiling pocket sphinx(Optional)
	http://www.rajeevan.co.uk/pocketsphinx_in_iphone/
	The steps are on the website. Since the results are not that good on Unity and I don't have time fully test out, the code is not intergrated into smartbody yet.
	When integrating pocketsphinx with Unity, it would have duplicated symbol problem(this might be the reason of bad recognizing result, it's under sphinx/src/util). I already built a library for Unity that can be used directly.
	Also for Unity you may need get prime31 iphone plugin AudioRecorder
	
--------------------------------Compiling using Xcode4------------------------------
1) Build bonebus
	Open smartbody-iphone.xcworkspace, select the scheme to be bonebus, build

2) Build boost
	http://www.boost.org/users/history/version_1_44_0.html
	Download boost_1_44_0.tar.gz, unzip to trunk/ios/boost. Make sure the folder name is boost_1_44_0.
	Open smartbody-iphone.xcworkspace, select boost_system, boost_filesystem, boost_regex, build them seperately.
	http://mathema.tician.de/news.tiker.net/download/software/boost-numeric-bindings/boost-numeric-bindings-20081116.tar.gz
	Download boost_numeric_bindings, unzip it to trunk/ios/boost, make sure the name is boost_numeric_bindings
	
3) Build steersuite
	Open smartbody-iphone.xcworkspace, select the steerlib, pprAI, build them seperately.

4) Build vhmsg
	Open smartbody-iphone.xcworkspace, select scheme vhmsg and build.
	
5) Build vhcl
	Since the vhcl_log.cpp hasn't been changed from VH group, you have to copy trunk/ios/vhcl/vhcl_log.cpp to trunk/lib/vhcl/src/vhcl_log.cpp for now.
	Open smartbody-iphone.xcworkspace, select scheme vhcl and build.

6) Build smartbody-lib
	Open smartbody-iphone.xcworkspace, select scheme smartbody-lib and build. 

7) Build smartbody-dll (For now, this is optional)
	Open smartbody-iphone.xcworkspace, select scheme smartbody-dll and build. 
	
8) Build vhwrapper-dll (For unity only, optional)
	Open smartbody-iphone.xcworkspace, select scheme vhwrapper-dll and build. 


--------------------------------Compiling and Running Applications using Xcode4------------------------------
There are two applications under trunk/ios/applications, you have to go over the previous steps. Make sure you have your device connected.
1) Build smartbody-openglES
	Go to trunk/ios/applications/minimal, open smartbody-iphone.xcodeproj, build and run.

Note: Under smartbody-openglES project Frameworks, you should see all the libraries existing. If not, go over previous steps to check if anything is wrong

2) Build smartbody-ogre
	http://www.ogre3d.org/download/sdk
	Download ogreSDK
	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Building%20From%20Source%20-%20iPhone&redirectpage=Building%20From%20Source%20(for%20iPhone)
	Go over this tutorial and build ogre iphone libraries. Make sure all the libraries exist inside build/lib/Debug
	Go to trunk/ios/applications/ogreIphone, open smartbody-ogre.xcodeproj, go to smartbody-ogre project, set OGRE_SDK_ROOT to your ogreSDK directory, set OGRE_SRC_ROOT to your ogre source directory. Select scheme smartbody-ogre, build and run.
	If the program hangs on boost thread function, try rebuild the ogre iphone dependencies boost libs (pthread, date_time), alternative way is to build the whole ogre iphone libraries with boost symbol turned off which may affect the results.
	
Note: ogre 1.8 seems to have trouble when building for iphone/ipad, use ogre 1.7.3. 
	  It is extremely slow running on armv6 ipod(after testing), and there's something wrong with the texture and shader. So maybe should just run on armv7 iphone/ipad.


3) Build smartbody-unity
	...
Note: If unity project hangs on the boost spinlock, make sure that the options for Compiling Thumb is set to the same for all the projects

--------------------------------------------------------------------------------------------------------------
