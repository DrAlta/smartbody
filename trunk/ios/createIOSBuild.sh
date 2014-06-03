# generate a sdk folder
mkdir -p release

# copy core
mkdir -p release/core
mkdir -p release/core/smartbody
mkdir -p release/core/smartbody/SmartBody
mkdir -p release/core/smartbody/steersuite-1.3
mkdir -p release/core/smartbody/steersuite-1.3/external
mkdir -p release/core/smartbody/steersuite-1.3/external/glfw
mkdir -p release/core/smartbody/steersuite-1.3/external/
mkdir -p release/core/smartbody/steersuite-1.3/pprAI
mkdir -p release/core/smartbody/steersuite-1.3/steerlib
rsync -ap --exclude=".svn" --exclude="*.c" --exclude="*.cpp" --exclude="*.dll" --exclude="*.lib" ../core/smartbody/Smartbody/src ./release/core/smartbody/SmartBody/
rsync -ap --exclude=".svn" ../core/smartbody/steersuite-1.3/external/glfw/include ./release/core/smartbody/steersuite-1.3/external/glfw
rsync -ap --exclude=".svn" --exclude="*.cpp" --exclude="*.txt" ../core/smartbody/steersuite-1.3/external/tinyxml ./release/core/smartbody/steersuite-1.3/external/
rsync -ap --exclude=".svn" --exclude="*.cpp" --exclude="*.c" ../core/smartbody/steersuite-1.3/external/mersenne ./release/core/smartbody/steersuite-1.3/external/
rsync -ap --exclude=".svn" ../core/smartbody/steersuite-1.3/pprAI/include ./release/core/smartbody/steersuite-1.3/pprAI
rsync -ap --exclude=".svn" ../core/smartbody/steersuite-1.3/steerlib/include ./release/core/smartbody/steersuite-1.3/steerlib

# copy lib
mkdir -p release/lib
mkdir -p release/lib/vhcl
mkdir -p release/lib/vhmsg
mkdir -p release/lib/vhmsg/vhmsg-c
mkdir -p release/lib/bonebus
rsync -ap --exclude=".svn" ../lib/vhcl/include/*.h ./release/lib/vhcl/include/
rsync -ap --exclude=".svn" ../lib/vhmsg/vhmsg-c/include/*.h ./release/lib/vhmsg/vhmsg-c/include/
rsync -ap --exclude=".svn" ../lib/bonebus/include/*.h ./release/lib/bonebus/include/

# copy ios
# (python is not for now since mac osx has python pre-installed, xcode is referring to /usr/include/python2.6)
# (activemq is not needed, it's only called during linking phase, so only static library needs to be provided)
mkdir -p release/ios
mkdir -p release/ios/applications
mkdir -p release/ios/ode
# mkdir -p release/ios/python
# mkdir -p release/ios/python/Python-2.6.5
mkdir -p release/ios/xerces-c
mkdir -p release/ios/boost
mkdir -p release/ios/boost/boost_1_51_0
rsync -ap --exclude=".svn" ./libs ./release/ios/
rsync -ap --exclude=".svn" ./applications/minimal ./release/ios/applications/
rsync -ap --exclude=".svn" ./ode/iphoneos ./release/ios/ode/
#rsync -ap --exclude=".svn" ./python/Python-2.6.5/_install ./release/python/Python-2.6.5/
#rsync -ap --exclude=".svn" ./python/Python-2.6.5/include ./release/python/Python-2.6.5/
#rsync -ap --exclude=".svn" ./python/Python-2.6.5/pyconfig.h ./release/python/Python-2.6.5/pyconfig.h
rsync -ap --exclude=".svn" ./xerces-c/iphoneos ./release/ios/xerces-c/
rsync -ap --exclude=".svn" ./boost/boost_1_51_0/boost ./release/ios/boost/boost_1_51_0/
#activemq?
