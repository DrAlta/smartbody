# use cmake to generate xcode and build
mkdir -p buildXcode
cd buildXcode
cmake -GXcode ..
xcodebuild -alltargets -configuration Release
cp ./src/Release/sbgui ../bin
cp ./src/Release/simplesmartbody ../bin/simplesmartbody
cp ./src/Release/libSmartBody.dylib ../bin/libSmartBody.dylib
cd ..