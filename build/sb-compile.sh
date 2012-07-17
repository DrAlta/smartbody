#!/bin/sh


cd build/lib/festival/speech_tools
sed -i '' -e 's/-Wall/-Wall -arch i386/g' config/compilers/gcc_defaults.mak
chmod 700 configure
./configure
make clean
make
make install
cd ../../../..

cd build/lib/festival/festival
chmod 700 configure
./configure
make clean
make
make install
cd ../../../..


mkdir build/bin
cd build/bin
cmake -DCMAKE_OSX_ARCHITECTURES="i386" ..
make
make install
