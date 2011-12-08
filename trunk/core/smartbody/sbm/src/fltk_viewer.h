/*
 *  sr_viewer - part of SBM: SmartBody Module
 *  Copyright (C) 2008  University of Southern California
 *
 *  SBM is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SBM is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SBM.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently at UC Merced)
 */


/** \file fltk_viewer.h
 * A fltk-opengl viewer
 */

# ifndef FLTK_VIEWER_H
# define FLTK_VIEWER_H

//#define USE_GLEW 1
#include <sbm/GPU/SbmShader.h>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Menu_Item.H>
# include <FL/Fl_Gl_Window.H>
# include <sr/sr_viewer.h>
#include <FL/Fl_Input.H>
# include <sr/sr_color.h>
# include <sr/sr_light.h>
# include <sr/sr_timer.h>
# include <sr/sr_sa_gl_render.h>
# include <sr/sr_sa_bbox.h>
# include <FL/Fl_Menu.H>


#include "ObjectManipulationHandle.h"

class SbmGeomObject;
class SrQuat;
class SrVec;
class SrEvent;
class SrCamera;
class SrSn;
class SrViewer;
class SrLight;
class FltkViewerData;
class LocomotionData;
class PALocomotionData;
class SbmCharacter;
class MeCtDataDrivenReach;
class MeCtConstraint;
class MeCtExampleBodyReach;

/*! \class SrViewer sr_viewer.h
    \brief A fltk-opengl viewer

    SrViewer implements a viewer to render objects derived from SrSn.
    The viewer has currently a planar and a tridimensional examiner mode.
    In ModePlanar, only transformation on the XY plane are accepted.
    In all modes, mouse interaction is done together with Ctrl and Shift modifiers.
    A popup menu appears with a right button click or ctrl+shift+m. */

class FltkViewer : public SrViewer, public Fl_Gl_Window, public SmartBody::SBObserver
 {
   public : // enumerators

    enum RenderMode { ModeAsIs,
                      ModeDefault,
                      ModeSmooth,
                      ModeFlat,
                      ModeLines,
                      ModePoints
                    };

	enum ShadowMode { ModeNoShadows,
					  ModeShadows
                };

	enum SteerMode {  ModeNoSteer,
					  ModeSteerCharactersGoalsOnly,
					  ModeSteerAll,
                };

	enum CharacterMode { ModeShowGeometry,
                         ModeShowCollisionGeometry,
						 ModeShowDeformableGeometry,
						 ModeShowDeformableGeometryGPU,
                         ModeShowBones,
                         ModeShowAxis
                    };

	enum PawnMode {	  ModeNoPawns,
                      ModePawnShowAsSpheres,
                    };

	enum ReachRenderMode { ModeShowExamples,
			               ModeNoExamples,
	};

	enum terrainMode { ModeNoTerrain,
					   ModeTerrainWireframe,
					   ModeTerrain,
                };

	enum EyeBeamMode { ModeNoEyeBeams,
					   ModeEyeBeams
                };
	enum EyeLidMode { ModeNoEyeLids,
					   ModeEyeLids
                };
	enum DynamicsMode { ModeNoDynamics,
					    ModeShowCOM,
						ModeShowCOMSupportPolygon,
						ModeShowMasses
                };
	enum LocomotionMode { 
						ModeEnableLocomotion,
						ModeShowAll,
						ModeShowVelocity,
						ModeShowOrientation,
						ModeShowSelection,
						ModeShowKinematicFootprints,
					    ModeShowLocomotionFootprints,
						ModeInteractiveLocomotion
                };

	enum GridMode { 
						ModeShowGrid,
						ModeNoGrid
                };

    enum MenuCmd { CmdHelp,
                   CmdViewAll,
                   CmdBackground,
                   CmdAsIs,
                   CmdDefault,
                   CmdSmooth,
                   CmdFlat,
                   CmdLines,
                   CmdPoints,                   
				   CmdNoShadows,
				   CmdShadows,	
				   CmdCharacterShowGeometry,
				   CmdCharacterShowCollisionGeometry,
				   CmdCharacterShowDeformableGeometry,
				   CmdCharacterShowDeformableGeometryGPU,
				   CmdCharacterShowBones,
				   CmdCharacterShowAxis,				   					   
				   CmdAxis,	
				   CmdBoundingBox,
				   CmdStatistics,
				   CmdGrid,
				   CmdNoGrid,				   
				   CmdNoPawns,
				   CmdPawnShowAsSpheres,
				   CmdCreatePawn,
				   CmdGazeOnTargetType1,
				   CmdGazeOnTargetType2,
				   CmdGazeOnTargetType3,
				   CmdGazeOnTargetType4,
				   CmdRemoveAllGazeTarget,
				   CmdNoTerrain,
				   CmdTerrainWireframe,
				   CmdTerrain,
				   CmdNoEyeBeams,
				   CmdEyeBeams,
				   CmdNoEyeLids,
				   CmdEyeLids,
				   CmdNoDynamics,
				   CmdShowCOM,
				   CmdShowCOMSupportPolygon,
				   CmdShowMasses,
				   CmdEnableLocomotion,
				   CmdShowLocomotionAll,
				   CmdShowVelocity,
				   CmdShowOrientation,
				   CmdShowSelection,
				   CmdShowKinematicFootprints,
				   CmdShowLocomotionFootprints,
				   CmdInteractiveLocomotion,
				   CmdShowTrajectory,				  
				   CmdReachShowExamples,
				   CmdReachNoExamples,
				   CmdConstraintToggleIK,
				   CmdConstraintToggleBalance,
				   CmdConstraintToggleReferencePose,
				   CmdNoSteer,
				   CmdSteerCharactersGoalsOnly,
				   CmdSteerAll,
                 };

   private : // internal data

    FltkViewerData* _data;
	LocomotionData* _locoData;
	float _arrowTime;
	PALocomotionData* _paLocoData;
	ObjectManipulationHandle _objManipulator; // a hack for testing. 

 private:
	 Fl_Input* off_height_window;
   public : //----> public methods 

    /*! Constructor needs the size and location of the window. */
    FltkViewer ( int x, int y, int w, int h, const char *label=0 );

    /*! Destructs all internal data, and calls unref() for the root node. */
    virtual ~FltkViewer ();

    /*! Retreave the scene root pointer, without calling unref() for it. Note that
        if the user does not give any root node to SrViewer, an empty (but valid)
        SrSnGroup is returned. */
    SrSn *root ();

    /*! Changes the scene root pointer. When the new node r is given, r->ref() is 
        called, and the old root node has its unref() method called. If r is null,
        an empty SrSnGroup is created and used as root */
    void root ( SrSn *r );

    /*! Draws string in the graphics window. If s==0 it will erase current string. */
    void draw_message ( const char* s );

    /*! Shows the viewer pop up menu. */
    void show_menu ();

    /*! Activates an option available from the right button mouse menu of the viewer. */
    void menu_cmd ( MenuCmd c, const char* label );

    /*! Returns true if the cmd is currently activated. */
    bool menu_cmd_activated ( MenuCmd c );

    void update_bbox ();

    void update_axis ();

    /*! Sets the camera to see the whole bounding box of the scene. The camera
        center is put in the center of the bounding box, and the eye is put in
        the line passing throught the center and parallel to the z axis, in a
        sufficient distance from the center to visualize the entire bounding,
        leaving the camera with a 60 degreed fovy. The up vector is set to (0,1,0). */
    void view_all ();

    /*! Will make SrViewer to render the scene in the next fltk loop. */
    void render ();

    /*! Returns true if the window is iconized, false otherwise. */
    bool iconized ();

    void increment_model_rotation ( const SrQuat &dq );

	void translate_keyboard_state();
    float fps ();
    sruint curframe ();

    SrColor background ();
    void background ( SrColor c );

    SrCamera* get_camera();
    void set_camera (const SrCamera &cam);

	FltkViewerData* getData() { return _data; };
	LocomotionData* getLocomotionData() { return _locoData; };
	ObjectManipulationHandle& getObjectManipulationHandle() { return _objManipulator; };

	void updateLights();

   public : // virtual methods

    /*! When the window manager asks the window to close.
        The default implementation calls exit(0). */
    virtual void close_requested ();

    /*! draw() sets the viewer options and render the scene root.
        This is a derived FLTK method and should not be called directly.
        To draw the window contents, use render() instead. */
    virtual void draw ();

    /*! Called to initialize opengl and to set the viewport to (w,h). This
        method is called also each time the window is reshaped. */
    virtual void init_opengl ( int w, int h );

	void initShadowMap();
	void makeShadowMap();
    /*! hande(int) controls the activation of the button menu, 
		and translates fltk events into the SrEvent class
        to then call handle_event(). Note that all handle methods should return
        1 when the event was understood and executed, and return 0 otherwise.
        This is a derived FLTK method. */ 
    virtual int handle ( int event );

    /*! handle_event() will just call the other handle methods, according 
        to the viewer configuration and generated event. This is the first method
        that will be called from the fltk derived handle() method. */
    virtual int handle_event ( const SrEvent &e );

    /*! Takes mouse events to rotate, scale and translate the scene. 
        If the event is not used, it is passed to the scene by calling
        handle_scene_event(). */
    virtual int handle_examiner_manipulation ( const SrEvent &e );

    /*! Applies an event action to the scene */
    virtual int handle_scene_event ( const SrEvent &e );

    /*! Events related to interactive object movement, rotation, etc */
	virtual int handle_object_manipulation( const SrEvent& e);

   /*! All keyboard events are passed to this method. The SrViewer
       implementation checks if crtl+shift+m is pressed to display
       the mouse menu, crtl+shift+x to exit the application, 
       crtl+shift+e to call the eps export action;
       otherwise it passes the event to the scene graph. */
    virtual int handle_keyboard ( const SrEvent &e );

	void processDragAndDrop(std::string dndMsg, float x, float y);
	void initGridList();	
	void drawAllGeometries(bool shadowPass = false); // draw all objects with geometry ( so no debug rendering included )
	void drawGrid();
	void drawEyeBeams();
	void drawEyeLids();
	void drawDynamics();
	void drawLocomotion();
	void drawReach();
	void drawInteractiveLocomotion();
	void drawPawns();
	void drawCharacterPhysicsObjs();
	void drawSteeringInfo();
	static void drawColObject(SbmGeomObject* colObj, SrMat& gmat);
	void drawTetra(SrVec vtxPos[4], SrVec& color);
	void drawArrow(SrVec& from, SrVec& to, float width, SrVec& color);
	void drawCircle(float cx, float cy, float cz, float r, int num_segments, SrVec& color);
	void drawPoint(float cx, float cy, float cz, float size, SrVec& color);
	void drawActiveArrow(SrVec& from, SrVec& to, int num, float width, SrVec& color, bool spin);
	void init_foot_print();
	//void drawLocomotionFootprints();
	void drawKinematicFootprints(int index);
	void newPrints(bool newprint, int index, SrVec& pos, SrVec& orientation, SrVec& normal, SrVec& color, int side, int type);
	static void ChangeOffGroundHeight(Fl_Widget* widget, void* data);
	void create_pawn();

	int gridList;
	float gridColor[4];
	float gridHighlightColor[3];
	float gridSize;
	float gridStep;

	SrVec interactivePoint;
	
	virtual void notify(SmartBody::SBSubject* subject);

	virtual void label_viewer(const char* str);
	virtual void show_viewer();
	virtual void hide_viewer();
	virtual void makeGLContext();

protected:
	
	void set_gaze_target(int itype, const char* targetname);	
	void set_reach_target(int itype, const char* targetname);	
	void update_submenus();
	void create_popup_menus();	
	MeCtExampleBodyReach* getCurrentCharacterBodyReachController();
	MeCtConstraint*    getCurrentCharacterConstraintController();
	SbmCharacter*        getCurrentCharacter();
	std::vector<SrLight> _lights;
 };


 class FltkViewerData
 { public :
   SrSn*  root;              // contains the user scene
   FltkViewer::RenderMode rendermode; // render mode
   FltkViewer::CharacterMode charactermode; // character mode
   FltkViewer::PawnMode pawnmode; // pawn mode
   FltkViewer::ShadowMode shadowmode;     // shadow mode
   FltkViewer::terrainMode terrainMode;     // terrain mode
   FltkViewer::EyeBeamMode eyeBeamMode;     // eye beam mode
   FltkViewer::EyeLidMode eyeLidMode;     // eye lid mode
   FltkViewer::DynamicsMode dynamicsMode;     // dynamics information mode
   FltkViewer::LocomotionMode locomotionMode;   // locomotion mode
   FltkViewer::ReachRenderMode reachRenderMode;
   FltkViewer::SteerMode steerMode;
	FltkViewer::GridMode gridMode;


   bool iconized;      // to stop processing while the window is iconized
   bool statistics;    // shows statistics or not
   bool displayaxis;   // if shows the axis or not
   bool boundingbox;   // if true will show the bbox of the whole scene
   bool scene_received_event; // to detect and send a release event to the scene graph
                              // when the alt key is released but the mouse is pushed
   bool showgeometry;
   bool showcollisiongeometry;
   bool showdeformablegeometry;
   bool showbones;
   bool showaxis;
   bool showmasses;

   bool locomotionenabled;
   bool showlocomotionall;
   bool showvelocity;
   bool showorientation;
   bool showselection;
   bool showlocofootprints;
   bool showkinematicfootprints;
   bool showtrajectory;
   bool interactiveLocomotion;

   SrString message;   // user msg to display in the window
   SrLight light;

   SrTimer    fcounter;   // To count frames and measure frame rate
   SrEvent    event;      // The translated event from fltk to sr format
   SrColor    bcolor;     // Background color currently used
   SrBox      bbox;       // Bounding box of the root, calculated with viewall
   SrCamera   camera;     // The current camera parameters

   SrSnLines* scenebox;  // contains the bounding box to display, and use in view_all
   SrSnLines* sceneaxis; // the current axis being displayed

   Fl_Menu_Button* menubut; // the ctrl+shift+m or button3 menu
   Fl_Window* helpwin;

   SrSaGlRender render_action;
   SrSaBBox bbox_action;

   GLuint  shadowMapID, depthMapID, depthFB, rboID;
   GLfloat shadowCPM[16];

 };

class LocomotionData
{
	public:
		LocomotionData() 
		{
			rps = 0.7f;
			x_flag = 0;
			z_flag = 0;
			rps_flag = 0;
			spd;
			x_spd = 7;
			z_spd = 70;
			t_direction[200];
			character[100];
			char_index = 0;
			kmode = 0;
			height_disp = 0.0f;
			height_disp_delta = 1.0f;
			height_disp_inc = false;
			height_disp_dec = false;
			upkey = false;
			downkey = false;
			leftkey = false;
			rightkey = false;
			a_key = false;
			d_key = false;

			off_height_comp = 0.0f;
		}
		
		float rps;
		int x_flag;
		int z_flag;
		int rps_flag;
		float spd;
		float x_spd;
		float z_spd;
		char t_direction[200];
		char character[100];
		int char_index;
		int kmode;
		float height_disp;
		float height_disp_delta;
		bool height_disp_inc;
		bool height_disp_dec;
		bool upkey;
		bool downkey;
		bool leftkey;
		bool rightkey;
		bool a_key;
		bool d_key;
		float off_height_comp;
};

class PALocomotionData
{
	public:
		PALocomotionData();
		~PALocomotionData();
	float w;	// angular velocity
	float v;	// velocity
	SbmCharacter* character;
	bool jumping;
	bool starting;
	bool stopping;
	bool prevJumping;
	bool prevStarting;
	bool prevStopping;
	float linearVelocityIncrement;
	float angularVelocityIncrement;
};
//================================ End of File =================================================

# endif // FLTK_VIEWER_H
