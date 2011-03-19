echo Running Postbuild.bat

@setlocal

@REM Configure for a VHuman project directory
set OGRE_HOME=..\..\..\lib\OgreSDK

@REM Initialize variables according to build type
if (%1)==(DEBUG) goto DEBUG_VARS
goto RELEASE_VARS

:RELEASE_VARS
set BIN_DIR=..\bin\win32
set PLUGINS_FILE=..\src\plugins-win32.cfg
set OGRE_BIN=%OGRE_HOME%\bin\release
set OGRE_FILES=OgreMain.dll;OIS.dll;RenderSystem_Direct3D9.dll;RenderSystem_GL.dll
copy /Y ..\..\..\lib\pthreads\lib\pthreadVSE2.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr\lib\libapr-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-iconv\lib\libapriconv-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-util\lib\libaprutil-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\activemq-cpp\vs2008-build\ReleaseDLL\activemq-cpp.dll %BIN_DIR%
set FINAL_BIN_DIR=..\..\..\bin\ogre-viewer\bin\win32
set FINAL_MEDIA_FOLDER=..\..\..\bin\ogre-viewer\media
goto COPY

:DEBUG_VARS
set BIN_DIR=..\bin\win32_debug
set PLUGINS_FILE=..\src\plugins-win32_debug.cfg
set OGRE_BIN=%OGRE_HOME%\bin\debug
set OGRE_FILES=OgreMain_d.dll;OIS_d.dll;RenderSystem_Direct3D9_d.dll;RenderSystem_GL_d.dll
copy /Y ..\..\..\lib\pthreads\lib\pthreadVSE2.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr\lib\libapr-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-iconv\lib\libapriconv-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-util\lib\libaprutil-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\activemq-cpp\vs2008-build\DebugDLL\activemq-cppd.dll %BIN_DIR%
set FINAL_BIN_DIR=..\..\..\bin\ogre-viewer\bin\win32_debug
set FINAL_MEDIA_FOLDER=..\..\..\bin\ogre-viewer\media
goto COPY

:COPY
@REM Copy files
copy %PLUGINS_FILE% %BIN_DIR%\plugins.cfg
for %%F in (%OGRE_FILES%) do echo %%F && COPY %OGRE_BIN%\%%F %BIN_DIR%
@REM expand "%DXSDK_DIR%\Redist\JUN2008_d3dx9_38_x86.cab" -F:d3dx9_38.dll %BIN_DIR%

call copy_media.bat SHARE %BIN_DIR%

if not exist %FINAL_BIN_DIR% mkdir %FINAL_BIN_DIR%
copy %BIN_DIR%\*.* %FINAL_BIN_DIR%

if not exist %FINAL_MEDIA_FOLDER%  mkdir %FINAL_MEDIA_FOLDER%
copy ..\media\*.* %FINAL_MEDIA_FOLDER%


@goto END

:ERROR
@echo ERROR in copy_media.bat: %ERROR_MSG%
@set ERRORLEV=1

:END
