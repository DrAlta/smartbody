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
set FINAL_BIN_DIR=..\..\..\bin\ogre-viewer\bin\win32
set FINAL_MEDIA_DIR=..\..\..\bin\ogre-viewer\media

copy /Y %PLUGINS_FILE% %BIN_DIR%\plugins.cfg

copy /Y %OGRE_BIN%\OgreMain.dll %BIN_DIR%
copy /Y %OGRE_BIN%\OIS.dll %BIN_DIR%
copy /Y %OGRE_BIN%\RenderSystem_Direct3D9.dll %BIN_DIR%
copy /Y %OGRE_BIN%\RenderSystem_GL.dll %BIN_DIR%

copy /Y ..\..\..\lib\pthreads\lib\pthreadVSE2.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr\lib\libapr-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-iconv\lib\libapriconv-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-util\lib\libaprutil-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\activemq-cpp\vs2008-build\ReleaseDLL\activemq-cpp.dll %BIN_DIR%

if not exist ..\media\OgreSDK\media\packs   mkdir ..\media\OgreSDK\media\packs
if not exist ..\media\OgreSDK\media\models  mkdir ..\media\OgreSDK\media\models
copy /Y %OGRE_HOME%\media\packs\OgreCore.zip  ..\media\OgreSDK\media\packs
copy /Y %OGRE_HOME%\media\models\cube.mesh    ..\media\OgreSDK\media\models
copy /Y %OGRE_HOME%\media\models\sphere.mesh  ..\media\OgreSDK\media\models

copy /Y ..\media\resources.cfg  %BIN_DIR%

xcopy /Y /Q /E /I /D %BIN_DIR% %FINAL_BIN_DIR%
xcopy /Y /Q /E /I /D ..\media %FINAL_MEDIA_DIR%

@goto END


:DEBUG_VARS
set BIN_DIR=..\bin\win32_debug
set PLUGINS_FILE=..\src\plugins-win32_debug.cfg
set OGRE_BIN=%OGRE_HOME%\bin\debug
set FINAL_BIN_DIR=..\..\..\bin\ogre-viewer\bin\win32_debug
set FINAL_MEDIA_DIR=..\..\..\bin\ogre-viewer\media

copy /Y %PLUGINS_FILE% %BIN_DIR%\plugins.cfg

copy /Y %OGRE_BIN%\OgreMain_d.dll %BIN_DIR%
copy /Y %OGRE_BIN%\OIS_d.dll %BIN_DIR%
copy /Y %OGRE_BIN%\RenderSystem_Direct3D9_d.dll %BIN_DIR%
copy /Y %OGRE_BIN%\RenderSystem_GL_d.dll %BIN_DIR%

copy /Y ..\..\..\lib\pthreads\lib\pthreadVSE2.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr\lib\libapr-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-iconv\lib\libapriconv-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\apr\apr-util\lib\libaprutil-1.dll %BIN_DIR%
copy /Y ..\..\..\lib\activemq\activemq-cpp\vs2008-build\DebugDLL\activemq-cppd.dll %BIN_DIR%

if not exist ..\media\OgreSDK\media\packs   mkdir ..\media\OgreSDK\media\packs
if not exist ..\media\OgreSDK\media\models  mkdir ..\media\OgreSDK\media\models
copy /Y %OGRE_HOME%\media\packs\OgreCore.zip  ..\media\OgreSDK\media\packs
copy /Y %OGRE_HOME%\media\models\cube.mesh    ..\media\OgreSDK\media\models
copy /Y %OGRE_HOME%\media\models\sphere.mesh  ..\media\OgreSDK\media\models

copy /Y ..\media\resources.cfg  %BIN_DIR%

xcopy /Y /Q /E /I /D %BIN_DIR% %FINAL_BIN_DIR%
xcopy /Y /Q /E /I /D ..\media %FINAL_MEDIA_DIR%

@goto END


:ERROR
@echo ERROR in postbuild.bat: %ERROR_MSG%
@set ERRORLEV=1


:END
