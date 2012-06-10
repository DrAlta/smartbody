include_directories( ./src )
include_directories( ../smartbody-lib/src )

find_package(OpenGL REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
find_library (COREAUDIO_LIBRARY CoreAudio)
find_library (AUDIOUNIT_LIBRARY AudioUnit)
find_library (CORESERVICES_LIBRARY CoreServices)
find_library (FESTIVAL Festival ../../../lib/festival/festival/src/lib)
find_library (ESTBASE estbase ../../../lib/festival/speech_tools/lib)
find_library (ESTSTRING eststring ../../../lib/festival/speech_tools/lib)
find_library (ESTOOLS estools ../../../lib/festival/speech_tools/lib)
endif()

find_library (LIBSNDFILE sndfile )
find_library (ALUT alut )
find_library (OPENAL openal)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set( MORE_LIBS
        ${COREAUDIO_LIBRARY}
        ${AUDIOUNIT_LIBRARY}
        ${CORESERVICES_LIBRARY}
        ${LIBSNDFILE}
        ${ALUT}
        ${OPENAL}
        ${FESTIVAL}
        ${ESTBASE}
        ${ESTSTRING}
        ${ESTOOLS}
)
else()
set( MORE_LIBS
        ${LIBSNDFILE}
        ${ALUT}
        ${OPENAL}
)

endif()


set( SBM_FLTK_SRC

	src/sbm_main.cpp
	src/ppm.cpp
	src/pic.cpp
#	src/glfont2.cpp
	src/CommandWindow.cpp
	src/ObjectManipulationHandle.cpp
	src/PawnPosControl.cpp
	src/PositionControl.cpp
	src/RotationControl.cpp
	src/RootWindow.cxx
	src/fltk_viewer.cpp
	src/pic.cpp
	src/ppm.cpp
	src/bmlviewer/BehaviorBlock.cpp
	src/bmlviewer/BehaviorEditorWidget.cpp
	src/bmlviewer/BehaviorWindow.cpp
	src/channelbufferviewer/GlChartView.cpp
	src/channelbufferviewer/GlChartViewArchive.cpp
	src/channelbufferviewer/GlChartViewCoordinate.cpp
	src/channelbufferviewer/channelbufferWindow.cpp
	src/nle/NonLinearEditor.cpp
	src/nle/NonLinearEditorWidget.cpp
	src/panimationviewer/PanimationWindow.cpp
	src/panimationviewer/ParamAnimBlock.cpp
	src/panimationviewer/ParamAnimEditorWidget.cpp
	src/panimationviewer/ParamAnimRunTimeEditor.cpp
	src/panimationviewer/ParamAnimScriptEditor.cpp
	src/panimationviewer/ParamAnimStateEditor.cpp
	src/panimationviewer/ParamAnimTransitionEditor.cpp
	src/panimationviewer/ParamAnimTransitionEditor2.cpp
	src/panimationviewer/ParamAnimTransitionCreatorWidget.cpp
	src/panimationviewer/ParameterGroup.cpp
	src/panimationviewer/ParameterVisualization.cpp
	src/panimationviewer/Parameter3DVisualization.cpp
	src/panimationviewer/ParamAnimStateCreatorWidget.cpp
	src/panimationviewer/ParamAnimAutoMarkingEditor.cpp
	src/panimationviewer/ParamAnimParameterEditor.cpp
	src/CharacterCreatorWindow.cpp
	src/bmlcreator/AttributeWindow.cpp
	src/bmlcreator/BMLAnimationObject.cpp
	src/bmlcreator/BMLBodyObject.cpp
	src/bmlcreator/BMLConstraintObject.cpp
	src/bmlcreator/BMLCreatorWindow.cpp
	src/bmlcreator/BMLEventObject.cpp
	src/bmlcreator/BMLFaceObject.cpp
	src/bmlcreator/BMLGazeObject.cpp
	src/bmlcreator/BMLGestureObject.cpp
	src/bmlcreator/BMLHeadObject.cpp
	src/bmlcreator/BMLLocomotionObject.cpp
	src/bmlcreator/BMLObject.cpp
	src/bmlcreator/BMLReachObject.cpp
	src/bmlcreator/BMLSpeechObject.cpp
	src/bmlcreator/BMLSaccadeObject.cpp
	src/bmlcreator/BMLStateObject.cpp

	src/resourceViewer/Fl_Tree_Horizontal.cpp
	src/resourceViewer/TreeItemInfoWidget.cpp
	src/resourceViewer/ResourceWindow.cpp
	src/resourceViewer/TreeInfoObject.cpp

	src/visemeviewer/VisemeCurve.cpp
	src/visemeviewer/VisemeCurveEditor.cpp
	src/visemeviewer/VisemeViewerWindow.cpp

	src/faceviewer/FaceViewer.cpp
	src/FLTKListener.cpp

)

add_executable( sbm-fltk

	${SBM_FLTK_SRC}
)

if(NOT APPLE )
set ( XFT Xft )	
else()
set ( XFT "" ) 
endif()

target_link_libraries( sbm-fltk

	#tcmalloc
	#profiler
	smartbody-lib
	bonebus
	vhmsg
	vhcl
	wsp
	xerces-c
	boost_filesystem
	boost_system
	boost_regex
	boost_python
	blas
	lapack
	GLEW
	${XFT}
	fltk_images 
	fltk 
	fltk_gl 
	${OPENGL_LIBRARIES}
	activemq-cpp
	steerlib
	pprAI
	ode
	python2.6
    ncurses

	${MORE_LIBS}
)

set( EXEC_LIST

	sbm-fltk
)

set( APP_LIBRARY_BINARY_PATHS

#	${PROJECT_BINARY_DIR}/core/smartbody/steersuite-1.3/pprAI ;
#	${PROJECT_BINARY_DIR}/core/smartbody/steersuite-1.3/steerlib

	${PROJECT_BINARY_DIR}/core/smartbody/sbm/bin
)

set( APP_LIBRARY_SOURCE_PATHS

#	${PROJECT_SOURCE_DIR}/core/smartbody/steersuite-1.3/pprAI/bin ;
#	${PROJECT_SOURCE_DIR}/core/smartbody/steersuite-1.3/steerlib/bin

	${PROJECT_SOURCE_DIR}/core/smartbody/sbm/bin
)

set_target_properties( 

	${EXEC_LIST} 

	PROPERTIES 

## link with binary tree:
#	INSTALL_RPATH				"${APP_LIBRARY_BINARY_PATHS}"

## link with install tree:
	INSTALL_RPATH				"${APP_LIBRARY_SOURCE_PATHS}"

	BUILD_WITH_INSTALL_RPATH	TRUE
)

install(

  	TARGETS ${EXEC_LIST}
	RUNTIME DESTINATION core/smartbody/sbm/bin
)

