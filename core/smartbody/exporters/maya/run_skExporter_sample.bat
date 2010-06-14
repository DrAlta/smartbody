
@rem usage:  call skExporter.bat "<source maya file>" "<root joint>" "<dest dir>" "<skeleton name>" "<rootJoint>" "<showAll/onlyLimited>" "<useLimits/ignoreLimits>"

call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "" "" "" ""
call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "" "" ""
call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "l_shoulder" "./New Folder/" "doctor-from-lshoulder" "" ""
call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-showall-useLimits" "showAll" "useLimits"
call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-showall-ignorelimits" "showAll" "ignoreLimits"
call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-onlyLimited-useLimits" "onlyLimited" "useLimits"
call skExporter.bat "C:/art/SBM_MayaProject/scenes/sbm_Proxy_Final.ma" "" "./New Folder/" "doctor-onlyLimited-ignorelimits" "onlyLimited" "ignoreLimits"
