
@rem all params must be enclosed in quotes
@rem params are optional, but an empty param must have "" to represent it

@rem usage:  call skmExporter.bat "<source maya file>" "<dest dir>/"
@rem examples:
@rem    call skmExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy2.ma" "./New Folder/"
@rem    call skmExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy2.ma" ""


set file=%1
set dir=%2
set dir_changequote=\"%dir:~1,-1%\"
"maya.exe" -log "error.log" -batch -file %file% -command "scriptJob -idleEvent \" quit -f \"; skmExport_BatchMode( %dir_changequote% );"
