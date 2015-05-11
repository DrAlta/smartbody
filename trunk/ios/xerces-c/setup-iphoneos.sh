#!/bin/sh

# This script is used for compiling xerces-c-3.1.1 for iphone device, other version not tested
# Modify PREFIX
# Modify variables if needed

export SBROOT="/Users/feng/Development/SmartBodyTrunk"
export ARMVERSION=7
export PREFIX="$SBROOT/ios/xerces-c/iphoneos"
export DEVROOT=/Developer/Platforms/iPhoneOS.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneOS4.3.sdk
export CFLAGS="-arch armv$ARMVERSION -pipe -no-cpp-precomp -isysroot$SDKROOT -miphoneos-version-min=4.1 -I$SDKROOT/usr/include/"
export CPPFLAGS="$CFLAGS"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L$SDKROOT/usr/lib/ -L$SDKROOT/usr/lib/system"
export CPP="$DEVROOT/usr/bin/cpp-4.2"
export CXX="$DEVROOT/usr/bin/g++-4.2"
export CC="$DEVROOT/usr/bin/gcc-4.2"
export LD="$DEVROOT/usr/bin/ld"


if [ ! -d "$SBROOT" ]; then
    echo "smartbody trunk location $SBROOT not set correctly"
else
    ./configure --host="arm-apple-darwin" --prefix=$PREFIX --enable-msgloader-inmemory
    make clean
    make
    make install
fi

cp $PREFIX/lib/*.a $SBROOT/ios/libs/iphoneos