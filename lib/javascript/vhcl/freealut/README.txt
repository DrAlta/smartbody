2015/7/16
Zengrui Wang

Freealut library building
Version: unknown(under emscripten/1.30.0/test/freealut)
Platform: Linux

Instruction:
1) Run command: $ emconfigure ./configure --disable-threads
2) Run command: $ emmake make
3) Location: emsdk_portable/emscripten/1.30.0/tests/freealut

Note: we haven't its functionality, we just want to resolve those unresolved symbols during the linking phase.