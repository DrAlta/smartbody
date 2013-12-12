#!/bin/sh

# This script is used for compiling protobuf-0.11.1 for iphone device, other version not tested
# Modify PREFIX
# Modify variables if needed

export SBROOT=""
export ARMVERSION=7
export PREFIX="$SBROOT/ios/protobuf/iphoneos"
export DEVROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneOS6.1.sdk
export CFLAGS="-arch armv$ARMVERSION -pipe -no-cpp-precomp --sysroot=$SDKROOT -miphoneos-version-min=4.1 -I$SDKROOT/usr/include/"
export CPPFLAGS="$CFLAGS"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L$SDKROOT/usr/lib/system"
export CPP="$DEVROOT/usr/bin/llvm-cpp-4.2"
export CXX="$DEVROOT/usr/bin/llvm-g++-4.2"
export CC="$DEVROOT/usr/bin/llvm-gcc-4.2"
export LD="$DEVROOT/usr/bin/ld"
export RANLIB="$DEVROOT/usr/bin/ranlib"

if [ ! -d "$SBROOT" ]; then
echo "smartbody trunk location $SBROOT not set correctly"
else
./configure --host="arm-apple-darwin" --prefix=$PREFIX --with-protoc=/usr/local/bin/protoc
make clean
make
make install
fi

cp $PREFIX/lib/*.a $SBROOT/ios/libs/iphoneos