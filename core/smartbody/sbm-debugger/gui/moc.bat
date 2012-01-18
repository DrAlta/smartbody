set MOC=..\vh\qt\bin\moc.exe  -DUNICODE -DWIN32 -DQT_LARGEFILE_SUPPORT -DQT_DLL -DQT_GUI_LIB -DQT_CORE_LIB -DQT_HAVE_MMX -DQT_HAVE_3DNOW -DQT_HAVE_SSE -DQT_HAVE_MMXEXT -DQT_HAVE_SSE2 -DQT_THREAD_SUPPORT -I"..\vh\qt\include\QtCore" -I"..\vh\qt\include\QtGui" -I"..\vh\qt\include" -I"." -I"..\vh\qt\include\ActiveQt" -I"..\vh\qt\mkspecs\win32-msvc2008" -D_MSC_VER=1500 -DWIN32

%MOC% ConnectDialog.h   -o moc_ConnectDialog.cpp
%MOC% glwidget.h        -o moc_glwidget.cpp
%MOC% SbmDebuggerForm.h -o moc_SbmDebuggerForm.cpp
%MOC% SettingsDialog.h -o moc_SettingsDialog.cpp
