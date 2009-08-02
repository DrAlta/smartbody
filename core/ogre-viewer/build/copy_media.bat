@echo Running copy_media.bat
@REM Windows version of the copy media process
@REM See :INIT_VARS for configuration

@setlocal

@if (%1)==() goto HELP
@REM if (%1)==() (set SHARE=1) && goto INIT_VARS
@if (%1)==(SHARE) (set SHARE=1) && goto INIT_VARS
@if (%1)==(COPY) (set SHARE=0) && goto INIT_VARS
@REM else
@set ERROR_MSG="Unknown parameter %"%1%". Expected SHARED or COPIED." && goto ERROR

:HELP
@echo SYNTAX: copy_media.bat COPY^|SHARE [bin directory]
@set ERRORLEV=1
@goto END

:INIT_VARS
@if (%2)==() goto DESTINATIONS_LOOP
@if not exist %2 set ERROR_MSG=Destination %2 does not exist && goto ERROR

set OGRE_HOME=..\..\..\lib\OgreSDK
@if not exist %OGRE_HOME% set ERROR_MSG="OGRE_HOME %OGRE_HOME% does not exist" && goto ERROR

set MEDIA_SRC_DIR=..\media
set BIN_DIR=%2
@if (%SHARE%)==(1) goto COPY_SHARED_CFG

@REM Copy only variables
set MEDIA_DIR=%BIN_DIR%\media
set MEDIA_SUBDIRECTORIES=packs;models
set OGRE_MEDIA=%OGRE_HOME%\media
set OGRE_RESOURCES=packs\OgreCore.zip;models\cube.mesh;models\sphere.mesh

:COPY_MEDIA
@REM Copy project media resources
if not exist %MEDIA_DIR% (echo Making directory %MEDIA_DIR% && mkdir %MEDIA_DIR%)
copy /Y %MEDIA_SRC_DIR% %MEDIA_DIR%

@REM Select resources-copied.cfg
if exist %BIN_DIR%\resources.cfg (del %BIN_DIR%\resources.cfg)
move %MEDIA_DIR%\resources-copied.cfg %BIN_DIR%\resources.cfg
del %MEDIA_DIR%\resources-shared.cfg


@REM Copy Ogre resources
@for %%D in (%MEDIA_SUBDIRECTORIES%) do if not exist %MEDIA_DIR%\%%D (echo Making directory %MEDIA_DIR%\%%D && mkdir %MEDIA_DIR%\%%D)
@for %%F in (%OGRE_RESOURCES%) do if not exist %MEDIA_DIR%\%%F (echo Copying %OGRE_MEDIA%\%%F && copy %OGRE_MEDIA%\%%F %MEDIA_DIR%\%%F)
@goto END

:COPY_SHARED_CFG
@REM Uses project media directory and OrgeCore from their pre-build locations
if exist %BIN_DIR%\resources.cfg (del %BIN_DIR%\resources.cfg)
copy %MEDIA_SRC_DIR%\resources-shared.cfg %BIN_DIR%\resources.cfg
@goto END

:DESTINATIONS_LOOP
@REM Attempt to copy media to the expect windows build
@set DEST=..\bin\win32;..\bin\win32_debug;..\bin\win64;..\win64_debug
for %%d in (%DEST%) do if exist %%d (echo Found destination %%d && call copy_media.bat %1 %%d)
@goto END

:ERROR
@echo ERROR in copy_media.bat: %ERROR_MSG%
@set ERRORLEV=1

:END