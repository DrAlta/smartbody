# generate a SmartBodySmartBodySDK folder
mkdir -p SmartBodySDK
# documentation
cp ../../SmartBodyManual.pdf ./SmartBodySDK/
cp ../../SmartBodyPythonAPI.html ./SmartBodySDK/
cp ../../"3rd party licenses.txt" ./SmartBodySDK/

# copy include
# (xerces is not matching the windows distribution)
# (boost, ode, FL are from windows distribution)
mkdir -p SmartBodySDK/include
rsync -ap --exclude=".svn" ../../core/smartbody/sbgui/external/fltk-1.3.2/FL ./SmartBodySDK/include/
rsync -ap --exclude=".svn" ../../core/smartbody/ode/include/ode ./SmartBodySDK/include/
rsync -ap --exclude=".svn" ../../lib/boost/boost ./SmartBodySDK/include/
rsync -ap --exclude=".svn" ./include/xercesc ./SmartBodySDK/include/
rsync -ap --exclude=".svn" ../../lib/vhmsg/vhmsg-c/include/*.h ./SmartBodySDK/include/vhmsg/
rsync -ap --exclude=".svn" ../../lib/wsp/wsp/include/*.h ./SmartBodySDK/include/wsp/
rsync -ap --exclude=".svn" ../../lib/bonebus/include/*.h ./SmartBodySDK/include/bonebus/
rsync -ap --exclude=".svn" ../../lib/vhcl/include/*.h ./SmartBodySDK/include/vhcl/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/pprAI/include/*.h ./SmartBodySDK/include/steersuite/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/steerlib/include/* ./SmartBodySDK/include/steersuite/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/* ./SmartBodySDK/include/steersuite/

# copy lib
rsync -ap --exclude=".svn" ./lib ./SmartBodySDK

# copy bin
rsync -ap --exclude=".svn" ./bin ./SmartBodySDK

# copy dylib for SmartBody (assuming it's prebuilt)
cp ../../core/smartbody/sbgui/bin/sbgui ./SmartBodySDK/bin
cp ../../core/smartbody/sbgui/bin/simplesmartbody ./SmartBodySDK/bin
cp ../../core/smartbody/sbgui/bin/libSmartBody.dylib ./SmartBodySDK/bin

# copy dylib for pprAI and steerlib
cp ../../core/smartbody/sbgui/bin/libpprAI.dylib ./SmartBodySDK/bin
cp ../../core/smartbody/sbgui/bin/libsteerlib.dylib ./SmartBodySDK/bin

# copy readme.txt
rsync -ap --exclude=".svn" ./readme.txt ./SmartBodySDK/readme.txt

# copy build*.sh
rsync -ap --exclude=".svn" ./build*.sh ./SmartBodySDK/

# copy CMakeLists.txt
rsync -ap --exclude=".svn" ./CMakeLists.txt ./SmartBodySDK/CMakeLists.txt

# copy src
rsync -ap --exclude=".svn" ./src ./SmartBodySDK
rsync -ap --exclude=".svn" ../../core/smartbody/sbgui/src/* ./SmartBodySDK/src/sbgui/
rsync -ap --exclude=".svn" ../../core/smartbody/simplesmartbody/simplesmartbody.cpp ./SmartBodySDK/src/simplesmartbody/
rsync -ap --exclude=".svn" ../../core/smartbody/Smartbody/src/* ./SmartBodySDK/src/Smartbody/


# first need to create data & data/mesh folder
# copy data folder
mkdir -p ./SmartBodySDK/data
mkdir -p ./SmartBodySDK/data/mesh
rsync -ap --exclude=".svn" ../../data/behaviorsets/* ./SmartBodySDK/data/behaviorsets/
rsync -ap --exclude=".svn" ../../data/ChrBrad/* ./SmartBodySDK/data/ChrBrad/
rsync -ap --exclude=".svn" ../../data/ChrRachel/* ./SmartBodySDK/data/ChrRachel/
rsync -ap --exclude=".svn" ../../data/examples/* ./SmartBodySDK/data/examples/
rsync -ap --exclude=".svn" ../../data/fonts/* ./SmartBodySDK/data/fonts/
rsync -ap --exclude=".svn" ../../data/Sinbad/* ./SmartBodySDK/data/Sinbad/
rsync -ap --exclude=".svn" ../../data/scripts/* ./SmartBodySDK/data/scripts/
rsync -ap --exclude=".svn" ../../data/mesh/ChrBrad/* ./SmartBodySDK/data/mesh/ChrBrad/
rsync -ap --exclude=".svn" ../../data/mesh/ChrRachel/* ./SmartBodySDK/data/mesh/ChrRachel/
rsync -ap --exclude=".svn" ../../data/mesh/Sinbad/* ./SmartBodySDK/data/mesh/Sinbad/
rsync -ap --exclude=".svn" ../../data/mesh/Ogre/* ./SmartBodySDK/data/mesh/Ogre/

