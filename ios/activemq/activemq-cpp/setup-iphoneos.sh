#!/bin/sh

# This script is used for compiling activemq-3.4.0 for iphone device, other version not testes
# Modify variables if needed
# Activemq need patch for iphone, read INSTALL.txt under smartbody root
# If running on Unity, need additional patch for decaf/internal/util/zip

export SBROOT=""
export ARMVERSION=7
export PREFIX="$SBROOT/ios/activemq/activemq-cpp/iphoneos"
export DEVROOT=/Developer/Platforms/iPhoneOS.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneOS4.3.sdk
export CFLAGS="-arch armv$ARMVERSION -pipe -no-cpp-precomp -isysroot$SDKROOT -miphoneos-version-min=4.1 -I$SDKROOT/usr/include/"
export CPPFLAGS="$CFLAGS"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-L$SDKROOT/usr/lib/ -L$SDKROOT/usr/lib/system -L$SBROOT/ios/activemq/apr-util/iphoneos/lib"
export CPP="$DEVROOT/usr/bin/cpp-4.2"
export CXX="$DEVROOT/usr/bin/g++-4.2"
export CC="$DEVROOT/usr/bin/gcc-4.2"
export LD="$DEVROOT/usr/bin/ld"

if [ ! -d "$SBROOT" ]; then
    echo "smartbody trunk location $SBROOT not set correctly"
else
    ./configure \
    --prefix=$PREFIX \
    --host="arm-apple-darwin" \
    --disable-ssl \
    --with-apr=$SBROOT/ios/activemq/apr/iphoneos \
    --with-apr-util=$SBROOT/ios/activemq/apr-util/iphoneos \
    --disable-dependency-tracking \

    make clean
    make
    make install
fi

cp $PREFIX/lib/*.a $SBROOT/ios/libs/iphoneos