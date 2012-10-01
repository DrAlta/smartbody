#!/bin/sh


cd lib/festival/speech_tools
sed -i '' -e 's/-Wall/-Wall -arch i386/g' config/compilers/gcc_defaults.mak
chmod 700 configure
./configure
make clean
make
make install
cd ../../..

cd lib/festival/festival
chmod 700 configure
./configure
make clean
make
make install
cd ../../..


mkdir bin
cd bin
cmake -DCMAKE_OSX_ARCHITECTURES="i386" ..
make
make install
cd ..
