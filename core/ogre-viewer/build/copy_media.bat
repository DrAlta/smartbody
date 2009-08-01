@echo Running copy_media.bat
@REM Windows version of the copy media process

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
@if (%OGRE_HOME%)==() set ERROR_MSG="OGRE_HOME not defined" && goto ERROR
@if not exist %OGRE_HOME% set ERROR_MSG="OGRE_HOME %OGRE_HOME% does not exist" && goto ERROR

set MEDIA_SRC_DIR=..\media
set BIN_DIR=%2
@if (%SHARE%)==(1) goto COPY_SHARED_CFG
set MEDIA_DIR=%BIN_DIR%\media

:COPY_MEDIA
@REM Copy build resources
@REM TODO: Use robocopy /MIR
copy %MEDIA_SRC_DIR% %BIN_DIR%
if (exist %BIN_DIR%\resources.cfg) del %BIN_DIR%\resources.cfg
move %MEDIA_DIR%\resources-copied.cfg %BIN_DIR%\resources.cfg
del %MEDIA_DIR%\resources-shared.cfg


@REM Copy Ogre resources
if not exist %MEDIA_DIR%\packs (mkdir %MEDIA_DIR%\packs)
if not exist %MEDIA_DIR%\packs\OgreCore.zip (copy %OGRE_HOME%\media\packs\OgreCore.zip %MEDIA_DIR%\packs)
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