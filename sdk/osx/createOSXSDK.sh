# generate a sdk folder
mkdir -p sdk
# documentation
cp ../../SmartBodyManual.pdf ./sdk/
cp ../../SmartBodyPythonAPI.html ./sdk/
cp ../../"3rd party licenses.txt" ./sdk/

# copy include
# (xerces is not matching the windows distribution)
# (boost, ode, FL are from windows distribution)
mkdir -p sdk/include
rsync -ap --exclude=".svn" ../../core/smartbody/sbgui/external/fltk-1.3.2/FL ./sdk/include/
rsync -ap --exclude=".svn" ../../core/smartbody/ode/include/ode ./sdk/include/
rsync -ap --exclude=".svn" ../../lib/boost/boost ./sdk/include/
rsync -ap --exclude=".svn" ./include/xercesc ./sdk/include/
rsync -ap --exclude=".svn" ../../lib/vhmsg/vhmsg-c/include/*.h ./sdk/include/vhmsg/
rsync -ap --exclude=".svn" ../../lib/wsp/wsp/include/*.h ./sdk/include/wsp/
rsync -ap --exclude=".svn" ../../lib/bonebus/include/*.h ./sdk/include/bonebus/
rsync -ap --exclude=".svn" ../../lib/vhcl/include/*.h ./sdk/include/vhcl/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/pprAI/include/*.h ./sdk/include/steersuite/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/steerlib/include/* ./sdk/include/steersuite/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/* ./sdk/include/steersuite/

# copy lib
rsync -ap --exclude=".svn" ./lib ./sdk

# copy bin
rsync -ap --exclude=".svn" ./bin ./sdk

# copy dylib for SmartBody (assuming it's prebuilt)
cp ../../core/smartbody/sbgui/bin/sbgui ./sdk/bin
cp ../../core/smartbody/sbgui/bin/simplesmartbody ./sdk/bin
cp ../../core/smartbody/sbgui/bin/libSmartBody.dylib ./sdk/bin

# copy dylib for pprAI and steerlib
cp ../../core/smartbody/sbgui/bin/libpprAI.dylib ./sdk/bin
cp ../../core/smartbody/sbgui/bin/libsteerlib.dylib ./sdk/bin

# copy readme.txt
rsync -ap --exclude=".svn" ./readme.txt ./sdk/readme.txt

# copy build*.sh
rsync -ap --exclude=".svn" ./build*.sh ./sdk/

# copy CMakeLists.txt
rsync -ap --exclude=".svn" ./CMakeLists.txt ./sdk/CMakeLists.txt

# copy src
rsync -ap --exclude=".svn" ./src ./sdk
rsync -ap --exclude=".svn" ../../core/smartbody/sbgui/src/* ./sdk/src/sbgui/
rsync -ap --exclude=".svn" ../../core/smartbody/simplesmartbody/simplesmartbody.cpp ./sdk/src/simplesmartbody/
rsync -ap --exclude=".svn" ../../core/smartbody/Smartbody/src/* ./sdk/src/Smartbody/


# first need to create data & data/mesh folder
# copy data folder
mkdir -p ./sdk/data
mkdir -p ./sdk/data/mesh
rsync -ap --exclude=".svn" ../../data/behaviorsets/* ./sdk/data/behaviorsets/
rsync -ap --exclude=".svn" ../../data/ChrBrad/* ./sdk/data/ChrBrad/
rsync -ap --exclude=".svn" ../../data/ChrRachel/* ./sdk/data/ChrRachel/
rsync -ap --exclude=".svn" ../../data/examples/* ./sdk/data/examples/
rsync -ap --exclude=".svn" ../../data/fonts/* ./sdk/data/fonts/
rsync -ap --exclude=".svn" ../../data/Sinbad/* ./sdk/data/Sinbad/
rsync -ap --exclude=".svn" ../../data/scripts/* ./sdk/data/scripts/
rsync -ap --exclude=".svn" ../../data/mesh/ChrBrad/* ./sdk/data/mesh/ChrBrad/
rsync -ap --exclude=".svn" ../../data/mesh/ChrRachel/* ./sdk/data/mesh/ChrRachel/
rsync -ap --exclude=".svn" ../../data/mesh/Sinbad/* ./sdk/data/mesh/Sinbad/
rsync -ap --exclude=".svn" ../../data/mesh/Ogre/* ./sdk/data/mesh/Ogre/
