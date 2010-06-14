
@rem all params must be enclosed in quotes
@rem params are optional, but an empty param must have "" to represent it

@rem usage:  call skmExporterSingle.bat "<source maya file>" "<dest dir and/or file>" "<anim name>" "<root joint>"
@rem examples:
@rem    call skmExporterSingle.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy2.ma" "" "rightUp" "l_shoulder"
@rem    call skmExporterSingle.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy2.ma" "./New Folder/" "rightUp" "l_shoulder"
@rem    call skmExporterSingle.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy2.ma" "./New Folder/testing.skm" "rightUp" ""
@rem    call skmExporterSingle.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy2.ma" "./New Folder/testing2.skm" "rightUp" "l_shoulder"


set file=%1
set dir=%2
set dir_changequote=\"%dir:~1,-1%\"
set animName=%3
set animName_changequote=\"%animName:~1,-1%\"
set joint=%4
set joint_changequote=\"%joint:~1,-1%\"
"maya.exe" -log "error.log" -batch -file %file% -command "scriptJob -idleEvent \" quit -f \"; skmExport_BatchMode_Single( %dir_changequote%, %animName_changequote%, %joint_changequote% );"
