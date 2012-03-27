
setlocal

call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
@rem vcvarsall turns echo off and doesn't turn it back on, shame on them
@echo on

devenv vs2010.sln /build Debug
devenv vs2010.sln /build Release

endlocal
