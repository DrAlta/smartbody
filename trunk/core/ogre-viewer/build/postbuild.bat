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
goto COPY

:DEBUG_VARS
set BIN_DIR=..\bin\win32_debug
set PLUGINS_FILE=..\src\plugins-win32_debug.cfg
set OGRE_BIN=%OGRE_HOME%\bin\debug
set OGRE_FILES=OgreMain_d.dll;OIS_d.dll;RenderSystem_Direct3D9_d.dll;RenderSystem_GL_d.dll
goto COPY

:COPY
@REM Copy files
copy %PLUGINS_FILE% %BIN_DIR%\plugins.cfg
for %%F in (%OGRE_FILES%) do echo %%F && COPY %OGRE_BIN%\%%F %BIN_DIR%
@REM expand "%DXSDK_DIR%\Redist\JUN2008_d3dx9_38_x86.cab" -F:d3dx9_38.dll %BIN_DIR%

call copy_media.bat SHARE %BIN_DIR%
@goto END

:ERROR
@echo ERROR in copy_media.bat: %ERROR_MSG%
@set ERRORLEV=1

:END
