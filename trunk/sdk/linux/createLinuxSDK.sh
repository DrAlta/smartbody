# generate a sdk folder
mkdir -p sdk
mkdir -p sdk/bin

# documentation
cp ../../SmartBodyManual.pdf ./sdk/
cp ../../SmartBodyPythonAPI.html ./sdk/
cp ../../"3rd party licenses.txt" ./sdk/

# copy include (do not include third party ones)
mkdir -p sdk/include
rsync -ap --exclude=".svn" ../../lib/vhmsg/vhmsg-c/include/*.h ./sdk/include/vhmsg/
rsync -ap --exclude=".svn" ../../lib/wsp/wsp/include/*.h ./sdk/include/wsp/
rsync -ap --exclude=".svn" ../../lib/bonebus/include/*.h ./sdk/include/bonebus/
rsync -ap --exclude=".svn" ../../lib/vhcl/include/*.h ./sdk/include/vhcl/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/pprAI/include/*.h ./sdk/include/steersuite/
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/steerlib/include/* ./sdk/include/steersuite/
mkdir -p sdk/include/steersuite/external
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/tinyxml/*.h ./sdk/include/steersuite/external/tinyxml/
mkdir -p sdk/include/steersuite/mersenne
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/mersenne/*.h ./sdk/include/steersuite/external/mersenne/
mkdir -p sdk/include/steersuite/external/glfw
mkdir -p sdk/include/steersuite/external/glfw/include
mkdir -p sdk/include/steersuite/external/glfw/include/GL
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/glfw/include/GL/glfw.h ./sdk/include/steersuite/external/glfw/include/GL

# copy lib
#rsync -ap --exclude=".svn" ./lib ./sdk

# copy bin
#rsync -ap --exclude=".svn" ./bin ./sdk

# copy .so for SmartBody (assuming it's prebuilt)
#cp ../../core/smartbody/sbgui/bin/sbgui ./sdk/bin
#cp ../../core/smartbody/sbgui/bin/simplesmartbody ./sdk/bin
#cp ../../core/smartbody/sbgui/bin/libSmartBody.so ./sdk/bin

# copy dylib for pprAI and steerlib
#cp ../../core/smartbody/sbgui/bin/libpprAI.so ./sdk/bin
#cp ../../core/smartbody/sbgui/bin/libsteerlib.so ./sdk/bin

# copy readme.txt
rsync -ap --exclude=".svn" ./README.txt ./sdk/README.txt

# copy build*.sh
#rsync -ap --exclude=".svn" ./build*.sh ./sdk/

# copy src
rsync -ap --exclude=".svn" ./src ./sdk
mkdir -p ./sdk/src/sbgui
rsync -ap --exclude=".svn" ../../core/smartbody/sbgui/src/* ./sdk/src/sbgui/
mkdir -p ./sdk/src/simplesmartbody
rsync -ap --exclude=".svn" ../../core/smartbody/simplesmartbody/simplesmartbody.cpp ./sdk/src/simplesmartbody/
mkdir -p ./sdk/src/SmartBody
rsync -ap --exclude=".svn" ../../core/smartbody/SmartBody/src/* ./sdk/src/SmartBody/
mkdir -p ./sdk/src/sbgui
rsync -ap --exclude=".svn" --exclude="fltk-1.3.2" --exclude="cegui" --exclude="cegui-0.8.2" --exclude="polyvox" ../../core/smartbody/sbgui/src/* ./sdk/src/sbgui/
mkdir -p ./sdk/src/sbgui/external
mkdir -p ./sdk/src/sbgui/external/polyvox
mkdir -p ./sdk/src/sbgui/external/polyvox/library
mkdir -p ./sdk/src/sbgui/external/polyvox/library/PolyVoxCore
rsync -arp --exclude=".svn" ../../core/smartbody/sbgui/external/polyvox/library/PolyVoxCore/* ./sdk/src/sbgui/external/polyvox/library/PolyVoxCore
mkdir -p ./sdk/src/sbgui/external/Pinocchio
rsync -ap --exclude=".svn" ../../core/smartbody/sbgui/external/Pinocchio/*.cpp ./sdk/src/sbgui/external/Pinocchio/
rsync -ap --exclude=".svn" ../../core/smartbody/sbgui/external/Pinocchio/*.h ./sdk/src/sbgui/external/Pinocchio/

mkdir -p ./sdk/src/vhcl
rsync -ap --exclude=".svn" ../../lib/vhcl/src/* ./sdk/src/vhcl/
mkdir -p ./sdk/src/vhmsg
rsync -ap --exclude=".svn" ../../lib/vhmsg/vhmsg-c/src/* ./sdk/src/vhmsg/
mkdir -p ./sdk/src/wsp
rsync -ap --exclude=".svn" ../../lib/wsp/wsp/src/* ./sdk/src/wsp/
mkdir -p ./sdk/src/bonebus
rsync -ap --exclude=".svn" ../../lib/bonebus/src/* ./sdk/src/bonebus/
mkdir -p ./sdk/src/steerlib
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/steerlib/src/* ./sdk/src/steerlib/
mkdir -p ./sdk/src/pprAI
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/pprAI/src/* ./sdk/src/pprAI/
mkdir -p ./sdk/src/external/tinyxml
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/tinyxml/*.cpp ./sdk/src/external/tinyxml/
mkdir -p ./sdk/src/external/glfw
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/glfw/lib/*.c ./sdk/src/external/glfw/
mkdir -p ./sdk/src/external/glfw/x11
rsync -ap --exclude=".svn" ../../core/smartbody/steersuite-1.3/external/glfw/lib/x11/*.c ./sdk/src/external/glfw/x11

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

# integration examples
mkdir sdk/src/ogresmartbody
cp ../win32/src/ogresmartbody/*.cpp sdk/src/ogresmartbody
cp ../win32/src/ogresmartbody/*.h sdk/src/ogresmartbody
cp ../win32/src/ogresmartbody/plugins.cfg-linux sdk/bin/plugins.cfg
cp ../win32/src/ogresmartbody/resources.cfg-linux sdk/bin/resources.cfg
cp ../win32/src/ogresmartbody/*.py sdk/data
mkdir sdk/src/irrlichtsmartbody
cp ../win32/src/irrlichtsmartbody/*.cpp  sdk/src/irrlichtsmartbody
cp ../win32/src/irrlichtsmartbody/*.h sdk/src/irrlichtsmartbody
cp ../win32/src/irrlichtsmartbody/*.py sdk/src/irrlichtsmartbody/data
mkdir sdk/data/irrlichtmedia
cp ../win32/src/irrlichtsmartbody/media/* sdk/data/irrlichtmedia

# build files
cp build/CMakeLists.txt ./sdk/
cp build/CMakeLists.txt-SmartBody ./sdk/src/SmartBody/CMakeLists.txt
cp build/CMakeLists.txt-src ./sdk/src/CMakeLists.txt
cp build/CMakeLists.txt-sbgui ./sdk/src/sbgui/CMakeLists.txt
cp build/CMakeLists.txt-polyvox ./sdk/src/sbgui/external/polyvox/library/PolyVoxCore/CMakeLists.txt
cp build/CMakeLists.txt-simplesmartbody ./sdk/src/simplesmartbody/CMakeLists.txt
cp build/CMakeLists.txt-vhcl ./sdk/src/vhcl/CMakeLists.txt
cp build/CMakeLists.txt-vhmsg ./sdk/src/vhmsg/CMakeLists.txt
cp build/CMakeLists.txt-bonebus ./sdk/src/bonebus/CMakeLists.txt
cp build/CMakeLists.txt-wsp ./sdk/src/wsp/CMakeLists.txt
cp build/CMakeLists.txt-steerlib ./sdk/src/steerlib/CMakeLists.txt
cp build/CMakeLists.txt-pprAI ./sdk/src/pprAI/CMakeLists.txt
cp build/CMakeLists.txt-polyvox ./sdk/src/sbgui/external/polyvox/CMakeLists.txt
cp build/CMakeLists.txt-Pinocchio ./sdk/src/sbgui/external/Pinocchio/CMakeLists.txt
cp build/CMakeLists.txt-ogresmartbody ./sdk/src/ogresmartbody/CMakeLists.txt
cp build/CMakeLists.txt-irrlichtsmartbody ./sdk/src/irrlichtsmartbody/CMakeLists.txt

