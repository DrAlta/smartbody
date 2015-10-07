add_subdirectory ( external/Pinocchio)
add_subdirectory ( external/polyvox)

#add_definitions(  -std=c++11 )
add_definitions(  -std=c++0x )

include_directories( ./src )
include_directories( ../SmartBody/src )
include_directories( ./external/Pinocchio )
include_directories( ./external/polyvox/library/PolyVoxCore/include )
#include_directories( /usr/local/include/CEGUI )
#link_directories( /usr/local/lib )
link_directories( /usr/lib )                                                           

set (BUILD_OGRE FALSE)
# never build ogre if in osx
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set (BUILD_OGRE FALSE)
	#include_directories( /Users/feng/SmartBodyDev/OgreSDK_1.7/include/ )
	#include_directories( /Users/feng/SmartBodyDev/OgreSDK_1.7/include/Ogre/ )
endif()


find_package(OpenGL REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
find_library (COREAUDIO_LIBRARY CoreAudio)
find_library (AUDIOUNIT_LIBRARY AudioUnit)
find_library (CORESERVICES_LIBRARY CoreServices)
find_library (GLEW GLEW)
find_library (ACTIVEMQLIB activemq-cpp PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (XERCESLIB xerces-c PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (BOOSTPYTHONLIB boost_python PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (BOOSTFILESYSTEMLIB boost_filesystem PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (BOOSTSYSTEMLIB boost_system PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (BOOSTREGEXLIB boost_regex PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (ODELIB ode PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (FLTKLIB fltk PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (FLTKIMAGESLIB fltk_images PATHS ${PROJECT_SOURCE_DIR}/lib )
find_library (FLTKGLLIB fltk_gl PATHS ${PROJECT_SOURCE_DIR}/lib )
if (${BUILD_FESTIVAL})
find_library (FESTIVAL Festival ../../../lib/festival/festival/src/lib)
find_library (ESTBASE estbase ../../../lib/festival/speech_tools/lib)
find_library (ESTSTRING eststring ../../../lib/festival/speech_tools/lib)
find_library (ESTOOLS estools ../../../lib/festival/speech_tools/lib)
else()
find_library (ACTIVEMQLIB activemq-cpp )
find_library (XERCESLIB xerces-c )
find_library (BOOSTPYTHONLIB boost_python )
find_library (BOOSTFILESYSTEMLIB boost_filesystem )
find_library (BOOSTSYSTEMLIB boost_system )
find_library (BOOSTREGEXLIB boost_regex )
find_library (ODELIB ode )
find_library (FLTKLIB fltk )
find_library (FLTKIMAGESLIB fltk_images )
find_library (FLTKGLLIB fltk_gl )
endif()
# these are the libraries necessary for osx, but disable it for now
#find_library (IOKIT_LIBRARY IOKit)
#find_library (FOUNDATION_LIBRARY Foundation)
#find_library (COCOA_LIBRARY Cocoa)
#find_library (X11_LIBRARY X11)

endif()

if (${BUILD_OGRE})
find_library (OGREMAIN Ogre)
find_library (OISLIB OIS)
#find_library (RTSHADERLIB OgreRTShaderSystem)
endif()
if (${BUILD_OGRE})
include_directories( /usr/local/include/OGRE )
set (OGREMAINLIB OgreMain)
set (OISLIB OIS)
set (OGRE_LIBS
	${OGREMAINLIB}	
	${OISLIB}		
	#${RTSHADERLIB}		
)

if (${SB_NO_VHMSG})
set( VHMSG_LIB
)
else()
set( VHMSG_LIB
        vhmsg
	${ACTIVEMQLIB}
	activemq-cpp
)
endif()

if (${SB_NO_BONEBUS})
set( BONEBUS_LIB
)
else()
set( BONEBUS_LIB
        bonebus
)
endif()

if (${SB_NO_PYTHON})
set( PYTHON_LIB
)
else()
set( PYTHON_LIB
	${BOOSTPYTHONLIB}
        python2.7
)
endif()



set (OGRE_SRCS
	src/EmbeddedOgre.cpp
	src/ExampleFrameListener.cpp	
	src/FLTKOgreViewer.cpp	
	src/OgreFrameListener.cpp
	src/SBOgreListener.cpp	
)
else()
set (OGRE_LIBS
)
set (OGRE_SRCS
)
endif()


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
find_library (LIBSNDFILE sndfile PATHS ${PROJECT_DOURCE_DIR}/lib)
find_library (ALUT alut PATHS ${PROJECT_DOURCE_DIR}/lib)
find_library (OPENAL openal PATHS ${PROJECT_DOURCE_DIR}/lib)

link_directories(/usr/X11R6/lib)
#link_directories(/usr/local/lib)
if (${BUILD_FESTIVAL})
set( FESTIVAL_LIBS 
        ${FESTIVAL}
        ${ESTBASE}
        ${ESTSTRING}
        ${ESTOOLS}
)
else()
set( FESTIVAL_LIBS
)
find_library (LIBSNDFILE sndfile )
find_library (ALUT alut )
find_library (OPENAL openal)
endif()
set( MORE_LIBS
        ${COREAUDIO_LIBRARY}
        ${AUDIOUNIT_LIBRARY}
        ${CORESERVICES_LIBRARY}
	${IOKIT_LIBRARY}
	${FOUNDATION_LIBRARY}
	${COCOA_LIBRARY}
	${X11_LIBRARY}	
        ${LIBSNDFILE}
        ${ALUT}
        ${OPENAL}
        ${GLEW}
        ${FESTIVAL_LIBS}
)
else()
set( MORE_LIBS
        ${LIBSNDFILE}
        ${ALUT}
        ${OPENAL}
	rt
	fontconfig
	Xft
	GLEW
)

endif()


# osx c++ version >= clang 4.2.0
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
		if (NOT (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.2.0))
			add_definitions(-Wno-deprecated-writable-strings)
			add_definitions(-Wno-format-security)
		endif()
	endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
		if (NOT (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.7.3))
			add_definitions(-Wno-format)
			add_definitions(-Wno-format-security)
			add_definitions(-Wno-write-strings)
		endif()
	endif()
endif()


set( SBM_FLTK_SRC

	./src/sbm_main.cpp
	./src/ppm.cpp
	./src/pic.cpp
#	./src/glfont2.cpp
	./src/CommandWindow.cpp
	./src/ObjectManipulationHandle.cpp
	./src/PawnPosControl.cpp
	./src/PositionControl.cpp
	./src/RotationControl.cpp
	./src/RootWindow.cxx
	./src/ExportWindow.cpp
	./src/fltk_viewer.cpp
	./src/SBWindowListener.cpp
	./src/SBSelectionManager.cpp
	./src/SBInterfaceListener.cpp
	${OGRE_SRCS}
	./src/pic.cpp
	./src/ppm.cpp
	./src/bmlviewer/BehaviorBlock.cpp
	./src/bmlviewer/BehaviorEditorWidget.cpp
	./src/bmlviewer/BehaviorWindow.cpp
	./src/channelbufferviewer/GlChartView.cpp
	./src/channelbufferviewer/GlChartViewArchive.cpp
	./src/channelbufferviewer/GlChartViewCoordinate.cpp
	./src/channelbufferviewer/channelbufferWindow.cpp
	./src/nle/NonLinearEditor.cpp
	./src/nle/NonLinearEditorWidget.cpp
	./src/panimationviewer/ErrorVisualization.cpp
	./src/panimationviewer/PanimationWindow.cpp
	./src/panimationviewer/ParamAnimBlock.cpp
	./src/panimationviewer/ParamAnimEditorWidget.cpp
	./src/panimationviewer/ParamAnimRunTimeEditor.cpp
	./src/panimationviewer/ParamAnimScriptEditor.cpp
	./src/panimationviewer/ParamAnimStateEditor.cpp
	./src/panimationviewer/ParamAnimTransitionEditor.cpp
	./src/panimationviewer/ParamAnimTransitionEditor2.cpp
	./src/panimationviewer/ParamAnimTransitionCreatorWidget.cpp
	./src/panimationviewer/ParameterGroup.cpp
	./src/panimationviewer/ParameterVisualization.cpp
	./src/panimationviewer/Parameter3DVisualization.cpp
	./src/panimationviewer/ParamAnimStateCreatorWidget.cpp
	./src/panimationviewer/ParamAnimAutoMarkingEditor.cpp
	./src/panimationviewer/ParamAnimParameterEditor.cpp
	./src/panimationviewer/VisualizationView.cpp
	./src/CharacterCreatorWindow.cpp
	./src/bmlcreator/AttributeWindow.cpp
	./src/bmlcreator/TextEditor.cpp
	./src/bmlcreator/BMLAnimationObject.cpp
	./src/bmlcreator/BMLBodyObject.cpp
	./src/bmlcreator/BMLConstraintObject.cpp
	./src/bmlcreator/BMLCreatorWindow.cpp
	./src/bmlcreator/BMLEventObject.cpp
	./src/bmlcreator/BMLFaceObject.cpp
	./src/bmlcreator/BMLGazeObject.cpp
	./src/bmlcreator/BMLGestureObject.cpp
	./src/bmlcreator/BMLHeadObject.cpp
	./src/bmlcreator/BMLLocomotionObject.cpp
	./src/bmlcreator/BMLObject.cpp
	./src/bmlcreator/BMLReachObject.cpp
	./src/bmlcreator/BMLSpeechObject.cpp
	./src/bmlcreator/BMLSaccadeObject.cpp
	./src/bmlcreator/BMLHandObject.cpp
	./src/bmlcreator/BMLStateObject.cpp
	./src/bmlcreator/BMLNoiseObject.cpp

	./src/resourceViewer/Fl_Tree_Horizontal.cpp
	./src/resourceViewer/TreeItemInfoWidget.cpp
	./src/resourceViewer/ResourceWindow.cpp
	./src/resourceViewer/TreeInfoObject.cpp
	./src/resourceViewer/AttributeEditor.cpp

	./src/visemeviewer/VisemeCurve.cpp
	./src/visemeviewer/VisemeCurveEditor.cpp
	./src/visemeviewer/VisemeViewerWindow.cpp
	./src/visemeviewer/VisemeRunTimeWindow.cpp
	./src/visemeviewer/VisemeRTCurveViewer.cpp
	./src/visemeviewer/ImageSequencePlayer.cpp
	
	./src/retargetviewer/RetargetViewer.cpp
	./src/retargetcreator/RetargetCreatorWindow.cpp
	./src/autorig/SBAutoRigManager.cpp
	./src/autorig/VoxelizerWindow.cpp
	./src/autorig/AutoRigViewer.cpp

	./src/monitorviewer/MonitorConnectWindow.cpp 
	./src/motioneditor/MotionEditorWindow.cpp
	./src/jointmapviewer/JointMapViewer.cpp
	./src/jointmapviewer/RetargetStepWindow.cpp

	./src/faceviewer/FaceViewer.cpp
	./src/faceshiftviewer/FaceShiftViewer.cpp
	./src/FLTKListener.cpp
	./src/SBGUIManager.cpp
	
	./src/flu/Flu_Button.cpp
	./src/flu/Flu_Collapsable_Group.cpp
	./src/flu/FluSimpleString.cpp
	
	./src/pythonbind/SBPythonAutorig.cpp
)

add_executable( sbgui

	${SBM_FLTK_SRC}
)

if(NOT APPLE )
set ( XFT  "")	
else()
set ( XFT "" ) 
endif()

if (${GOOGLE_PROFILER})
set ( PROFILER_LIBS
	tcmalloc
	profiler
)
else()
set ( PROFILER_LIBS "")
endif()

target_link_libraries( sbgui

	${PROFILER_LIBS}
	SmartBody
	${BONEBUS_LIB}
	${VHMSG_LIB}
	vhcl
	${BOOSTFILESYSTEMLIB}
	${BOOSTSYSTEMLIB}
	${BOOSTREGEXLIB}
	${PYTHON_LIB}
	${XFT}
	${FLTKLIB}
	${FLTKIMAGESLIB}
	${FLTKGLLIB}
	${OPENGL_LIBRARIES}
    	ncurses	
	pthread
	pinocchio
        polyvoxcore
	#pinocchio
	${MORE_LIBS}
	${OGRE_LIBS}
	#CEGUIBase
	#CEGUIOpenGLRenderer
	${PROTOCLIB}
	${PROTOBUFLIB}
)

set( EXEC_LIST

	sbgui
)

set( APP_LIBRARY_BINARY_PATHS

#	${PROJECT_BINARY_DIR}/core/smartbody/steersuite-1.3/pprAI ;
#	${PROJECT_BINARY_DIR}/core/smartbody/steersuite-1.3/steerlib

	${PROJECT_BINARY_DIR}/core/smartbody/sbgui/bin
)

set( APP_LIBRARY_SOURCE_PATHS

#	${PROJECT_SOURCE_DIR}/core/smartbody/steersuite-1.3/pprAI/bin ;
#	${PROJECT_SOURCE_DIR}/core/smartbody/steersuite-1.3/steerlib/bin

	${PROJECT_SOURCE_DIR}/core/smartbody/sbgui/bin
)

set_target_properties( 

	${EXEC_LIST} 

	PROPERTIES 

## link with binary tree:
#	INSTALL_RPATH				"${APP_LIBRARY_BINARY_PATHS}"

## link with install tree:
	#INSTALL_RPATH				"${APP_LIBRARY_SOURCE_PATHS}"
	INSTALL_RPATH				lib

	BUILD_WITH_INSTALL_RPATH	TRUE
)

install(

  	TARGETS ${EXEC_LIST}
	RUNTIME DESTINATION core/smartbody/sbgui/bin
)

