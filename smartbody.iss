; -- SmartBody.iss --

[Setup]
AppName=SmartBody
AppVerName=SmartBody
DefaultDirName={userdocs}\SmartBody
DefaultGroupName=SmartBody
UninstallDisplayIcon={app}\SmartBody_48x48.ico
Compression=lzma
SolidCompression=yes
OutputDir=.
AppPublisher=USC Institute for Creative Technologies
AppPublisherURL=http://smartbody.ict.usc.edu
AppVersion=r5498
OutputBaseFilename=SmartBody
WizardImageFile=".\sdk\SmartBody_splash.bmp"
RestartIfNeededByRun=yes


[Files]
; documentation
Source: ".\SmartBodyManual.pdf"; DestDir: "{app}"
Source: ".\SmartBodyPythonAPI.html"; DestDir: "{app}"
Source: ".\3rd party licenses.txt"; DestDir: "{app}"

; executables
Source: ".\core\smartbody\sbgui\bin\*.exe"; DestDir: "{app}\bin"
Source: ".\core\smartbody\sbgui\bin\*.dll"; DestDir: "{app}\bin"
Source: ".\core\smartbody\sbgui\bin\*.pyd"; DestDir: "{app}\bin"
Source: ".\sdk\.smartbodysettings"; DestDir: "{app}\bin"
;Source: ".\core\smartbody\sbgui\visualc9\SmartBody_48x48.ico"; DestDir: "{app}\bin"

; supporting libraries - Python 2.7
Source: ".\core\smartbody\Python27\*"; DestDir: "{app}\python27"; Excludes: "tcl, tk, Cerebella, pyke, pyxb, scipy, .svn"; Flags: recursesubdirs

; supporting libraries - Ogre 1.8.1
Source: ".\lib\OgreSDK\bin\*"; DestDir: "{app}\OgreSDK\bin";  Excludes: ".svn";  Flags: recursesubdirs
Source: ".\lib\OgreSDK\Docs\*"; DestDir: "{app}\OgreSDK\Docs"; Excludes: ".svn";  Flags: recursesubdirs
Source: ".\lib\OgreSDK\include\*"; DestDir: "{app}\OgreSDK\include"; Excludes: ".svn"; Flags: recursesubdirs
Source: ".\lib\OgreSDK\lib\*"; DestDir: "{app}\OgreSDK\lib"; Excludes: ".svn"; Flags: recursesubdirs
Source: ".\lib\OgreSDK\media\*"; DestDir: "{app}\OgreSDK\media"; Excludes: ".svn"; Flags: recursesubdirs
Source: ".\lib\OgreSDK\samples\*"; DestDir: "{app}\OgreSDK\samples"; Excludes: ".svn"; Flags: recursesubdirs

; supporting libraries - CEGUI 0.8.2



; supplementary executables
;Source: ".\core\TtsRelay\bin\x86\Release\*.exe"; DestDir: "{app}\bin"
;Source: ".\core\TtsRelay\bin\x86\Release\*.dll"; DestDir: "{app}\bin"

; library includes
Source: ".\core\smartbody\steersuite-1.3\steerlib\include\*"; DestDir: "{app}\include\steersuite";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\core\smartbody\steersuite-1.3\pprAI\include\*"; DestDir: "{app}\include\steersuite";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\core\smartbody\steersuite-1.3\external\tinyxml\*"; DestDir: "{app}\include\steersuite\tinyxml";  Excludes: ".svn"; 
Source: ".\core\smartbody\steersuite-1.3\external\mersenne\*"; DestDir: "{app}\include\steersuite\mersenne"; Excludes: ".svn"
Source: ".\core\smartbody\SmartBody\src\external\protobuf\include\*"; DestDir: "{app}\include\"; Excludes: ".svn"; Flags: recursesubdirs
Source: ".\lib\vhcl\include\*"; DestDir: "{app}\include\vhcl"
Source: ".\lib\vhmsg\vhmsg-c\include\*"; DestDir: "{app}\include\vhmsg"
Source: ".\lib\bonebus\include\*"; DestDir: "{app}\include\bonebus"
Source: ".\lib\wsp\wsp\include\*"; DestDir: "{app}\include\wsp"
Source: ".\core\smartbody\sbgui\external\fltk-1.3.2\FL\*"; DestDir: "{app}\include\FL"
Source: ".\core\smartbody\sbgui\external\cegui-0.8.2\include\*"; DestDir: "{app}\include\CEGUI";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\core\smartbody\sbgui\external\Pinocchio\*.h"; DestDir: "{app}\include\Pinocchio";
Source: ".\core\smartbody\sbgui\external\polyvox\library\PolyVoxCore\include\*"; DestDir: "{app}\include\polyvox";   Excludes: ".svn"; Flags: recursesubdirs

; libary header files
Source: ".\lib\xerces-c\include\xercesc\*"; DestDir: "{app}\include\xercesc";Flags: recursesubdirs
Source: ".\lib\boost\boost\*"; DestDir: "{app}\include\boost";Flags: recursesubdirs
Source: ".\core\SmartBody\ode\include\*"; DestDir: "{app}\include";Flags: recursesubdirs

; applications
Source: ".\core\smartbody\SmartBody\src\*"; DestDir: "{app}\src\SmartBody";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\core\smartbody\sbgui\src\*"; DestDir: "{app}\src\sbgui";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\core\smartbody\simplesmartbody\simplesmartbody.cpp"; DestDir: "{app}\src\simplesmartbody"; Flags:

; build
Source: ".\core\smartbody\SmartBody\SmartBody.vcxproj"; DestDir: "{app}\build"; Flags:
Source: ".\core\smartbody\SmartBody\SmartBody.vcxproj.filters"; DestDir: "{app}\build"; Flags:
Source: ".\core\smartbody\sbgui\visualc9\sbgui.vcxproj"; DestDir: "{app}\build"; Flags:
Source: ".\core\smartbody\sbgui\visualc9\sbgui.vcxproj.filters"; DestDir: "{app}\build"; Flags:
Source: ".\core\smartbody\simplesmartbody\simplesmartbody.vcxproj"; DestDir: "{app}\build"; Flags:


; libraries
Source: ".\core\smartbody\SmartBody\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\SmartBody\lib\*.pdb"; DestDir: "{app}\lib"

Source: ".\core\smartbody\SmartBody\src\external\glew\glew32.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\SmartBody\src\external\protobuf\lib\Release\*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\SmartBody\src\external\protobuf\lib\Debug\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\vhcl\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\vhmsg\vhmsg-c\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\bonebus\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\wsp\wsp\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\clapack\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\activemq\activemq-cpp\vs2010-build\ReleaseDLL\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\activemq\activemq-cpp\vs2010-build\DebugDLL\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\xerces-c\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\boost\lib\*vc100-mt-1_51.lib"; DestDir: "{app}\lib"
Source: ".\lib\boost\lib\*vc100-mt-gd-1_51.lib"; DestDir: "{app}\lib"
Source: ".\lib\vhcl\openal\libs\Win32\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\vhcl\libsndfile\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\pthreads\lib\pthreadVSE2.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\ode\lib\ode*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\steersuite-1.3\build\win32\Release\*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\steersuite-1.3\build\win32\Debug\*.lib"; DestDir: "{app}\lib"
Source: ".\lib\pthreads\lib\pthreadVSE2.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\sbgui\external\fltk-1.3.2\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\sbgui\external\cegui-0.8.2\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\sbgui\external\Pinocchio\lib\*.lib"; DestDir: "{app}\lib"
Source: ".\core\smartbody\sbgui\external\polyvox\build\lib\*.lib"; DestDir: "{app}\lib"

; data
Source: ".\data\behaviorsets\*"; DestDir: "{app}\data\behaviorsets";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\data\examples\*"; DestDir: "{app}\data\examples"; Excludes: "Physics, Terrain, .svn";Flags: recursesubdirs
Source: ".\data\fonts\*"; DestDir: "{app}\data\fonts";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\data\ChrBrad\*"; DestDir: "{app}\data\ChrBrad";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\data\ChrRachel\*"; DestDir: "{app}\data\ChrRachel";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\data\Sinbad\*"; DestDir: "{app}\data\Sinbad";  Excludes: ".svn"; Flags: recursesubdirs
Source: ".\data\mesh\ChrBrad\*"; DestDir: "{app}\data\mesh\ChrBrad"
Source: ".\data\mesh\ChrRachel\*"; DestDir: "{app}\data\mesh\ChrRachel"
Source: ".\data\mesh\ChrMaarten\*"; DestDir: "{app}\data\mesh\ChrMaarten"
Source: ".\data\mesh\Sinbad\*"; DestDir: "{app}\data\mesh\Sinbad"
Source: ".\data\scripts\*"; DestDir: "{app}\data\scripts"
Source: ".\data\cegui\datafiles-0.8.2\*"; DestDir: "{app}\data\cegui\datafiles-0.8.2";  Excludes: ".svn"; Flags: recursesubdirs

Source: "c:\users\shapiro\smartbody\externalprograms\vcredist_x86.exe"; DestDir: "{app}"

; sdk build
Source: ".\sdk\README.txt"; DestDir: "{app}\"
Source: ".\sdk\resources.cfg"; DestDir: "{app}\"
Source: ".\sdk\win32\build\*"; DestDir: "{app}\build"
Source: ".\sdk\win32\bin\*"; DestDir: "{app}\bin"
Source: ".\sdk\win32\src\*"; DestDir: "{app}\src";  Excludes: ".svn"; Flags: recursesubdirs



[Registry]

[Icons]
Name: "{group}\SmartBody"; Filename: "{app}\bin\smartbody\sbm\sbgui.exe"; WorkingDir: "{app}"
Name: "{group}\SmartBody Manual"; Filename: "{app}\SmartBodyManual.pdf"; WorkingDir: "{app}"
Name: "{group}\SmartBody Python API"; Filename: "{app}\SmartBodyPythonAPI.html"; WorkingDir: "{app}"

[Run]
Filename: {app}\vcredist_x86.exe; Description: "Visual Studio 2010 Redistributable"; Parameters: "/q:a /c:""install /l """; WorkingDir: {tmp}; Flags: postinstall runascurrentuser ; StatusMsg: "Installing ""Microsoft Visual C++ 2010 Redistributable Package"" if needed. This can take several minutes..."
Filename: {app}\bin\sbgui.exe ; Flags: postinstall ; Description: "Run SmartBody"




