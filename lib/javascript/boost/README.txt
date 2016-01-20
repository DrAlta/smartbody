2015/7/16
Zengrui Wang

Boost library building
Version: 1.58.0
Platform: Linux

Github repo: https://github.com/ZengruiWang/android-vendor-boost-1-58-0 (forked and editted from https://github.com/arielm/chronotext-boost)

Instruction:
1) Clone the repository into your local directory
2) Run 'setup.sh' which downloads the boost_1_58_0 package and some patches
3) Edit 'build-emscripten.sh'
   a) Specify EMSCRIPTEN_BIN = '/home/zewang/emsdk_portable/emscripten/1.30.0' (where your Emscripten SDK located)
   b) Specify the libraries to build: --without-python --without-coroutine --without-context
4) All compiled libs is under ./chronotext-boost/lib/emscripten/

Note: don't forget to copy the Numeric Library Bindings to boost/boost/numeric

