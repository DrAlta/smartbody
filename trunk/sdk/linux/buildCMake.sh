# make using standard cmake
mkdir -p buildCMake
cd buildCMake
cmake ..
make -j 6
make install
cd ..

