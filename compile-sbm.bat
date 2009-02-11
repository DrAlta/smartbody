
setlocal

call "%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
@rem vcvarsall turns echo off and doesn't turn it back on, shame on them
@echo on

devenv vs2008.sln /build Debug
devenv vs2008.sln /build Release

endlocal
