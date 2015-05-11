#!/bin/zsh
set -o errexit

# credit to:
# http://randomsplat.com/id5-cross-compiling-python-for-embedded-linux.html
# http://latenitesoft.blogspot.com/2008/10/iphone-programming-tips-building-unix.html


export SBROOT="/Users/feng/Development/SmartBodyTrunk/trunk"
if [ ! -d "$SBROOT" ]; then
    echo smartbody trunk location set incorrect
    exit 1
fi

export IOS_VERSION="8.2"

# download python and patch if they aren't there
if [[ ! -a Python-2.6.5.tar.bz2 ]]; then
    curl http://www.python.org/ftp/python/2.6.5/Python-2.6.5.tar.bz2 > Python-2.6.5.tar.bz2
fi

# get rid of old build
rm -rf Python-2.6.5

# build for native machine
tar -xjf Python-2.6.5.tar.bz2
pushd ./Python-2.6.5
CC=clang ./configure
make python.exe Parser/pgen

mv python.exe hostpython
mv Parser/pgen Parser/hostpgen

make distclean

# patch python to cross-compile
patch -p1 < ../Python-2.6.5-xcompile.patch

export SDKROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS8.2.sdk
export TOOLCHAIN=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain
#export CLANG_VERBOSE="--verbose"
export CPPFLAGS="$CLANG_VERBOSE -arch armv7 -pipe -miphoneos-version-min=6.0 -isysroot $SDKROOT"
export CFLAGS="$CLANG_VERBOSE -arch armv7 -pipe -miphoneos-version-min=6.0 -isysroot $SDKROOT -g -O0 -std=c99 -fPIC"
export CXXFLAGS="$CLANG_VERBOSE -arch armv7 -pipe -miphoneos-version-min=6.0 -isysroot $SDKROOT -g -O0 -std=c++11 -stdlib=libc++ -fPIC -fcxx-exceptions"
export LDFLAGS="-arch armv7 -miphoneos-version-min=6.0 -stdlib=libc++ -L$SDKROOT/usr/lib -L$SDKROOT/usr/lib/system"
#export LIBS="-lc++ -lc++abi"
export CPP="$TOOLCHAIN/usr/bin/clang -E"
export CXX="$TOOLCHAIN/usr/bin/clang++"
export CC="$TOOLCHAIN/usr/bin/clang"
export LD="$TOOLCHAIN/usr/bin/ld"

export RANLIB="$SDKROOT/../../usr/bin/ranlib"
# build for iPhone
./configure --build=x86_64-apple-darwin --host="arm-apple-darwin" --disable-toolbox-glue

make HOSTPYTHON=./hostpython HOSTPGEN=./Parser/hostpgen \
     CROSS_COMPILE_TARGET=yes

make install HOSTPYTHON=./hostpython CROSS_COMPILE_TARGET=yes prefix="$PWD/_install"
cp ./_install/lib/*.a $SBROOT/ios/libs/iphoneos
