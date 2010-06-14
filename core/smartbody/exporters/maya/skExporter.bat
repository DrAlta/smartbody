
@rem all params must be enclosed in quotes
@rem params are optional, but an empty param must have "" to represent it

@rem usage:  call skExporter.bat "<source maya file>" "<root joint>" "<dest dir>" "<skeleton name>" "<rootJoint>" "<showAll/onlyLimited>" "<useLimits/ignoreLimits>"
@rem examples:
@rem    call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "" "" "" ""
@rem    call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "" "" ""
@rem    call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "l_shoulder" "./New Folder/" "doctor-from-lshoulder" "" ""
@rem    call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-showall-useLimits" "showAll" "useLimits"
@rem    call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-showall-ignorelimits" "showAll" "ignoreLimits"
@rem    call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-onlyLimited-useLimits" "onlyLimited" "useLimits"
@rem    call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-onlyLimited-ignorelimits" "onlyLimited" "ignoreLimits"


set file=%1
set root=%2
set root_changequote=\"%root:~1,-1%\"
set dir=%3
set dir_changequote=\"%dir:~1,-1%\"
set name=%4
set name_changequote=\"%name:~1,-1%\"
set showAll=%5
set showAll_changequote=\"%showAll:~1,-1%\"
set useLimits=%6
set useLimits_changequote=\"%useLimits:~1,-1%\"
"maya.exe" -log "error.log" -batch -file %file% -command "scriptJob -idleEvent \" quit -f \"; skExport_BatchMode( %root_changequote%, %dir_changequote%, %name_changequote%, %showAll_changequote%, %useLimits_changequote% );"
