/*
 /*
 *  sr_viewer.cpp - part of SBM: SmartBody Module
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

#include "FL/Fl_Slider.H"  // before vhcl.h because of LOG enum which conflicts with vhcl::Log
#include "vhcl.h"
//#include <FL/enumerations.H>
# include "fltk_viewer.h"
# include <FL/Fl.H>
# include <GL/gl.h>
# include <GL/glu.h>
//# include <fltk/visual.h>
//# include <fltk/compat/FL/Fl_Menu_Item.H>
# include <FL/fl_draw.H>
# include <FL/Fl_Color_Chooser.H>
# include <FL/Fl_File_Chooser.H>
# include <FL/Fl_Browser.H>
//# include <fltk/ToggleItem.h>
# include <sr/sr_box.h>
# include <sr/sr_sphere.h>
# include <sr/sr_cylinder.h>
# include <sr/sr_quat.h>
# include <sr/sr_line.h>
# include <sr/sr_plane.h>
# include <sr/sr_event.h>
# include <sr/sr_string.h>

# include <sr/sr_gl.h>
# include <sr/sr_camera.h>
# include <sr/sr_trackball.h>
# include <sr/sr_lines.h>
# include <sr/sr_color.h>
# include <sr/sr_points.h>

# include <sr/sr_sn.h>
# include <sr/sr_sn_group.h>

# include <sr/sr_sa.h>
# include <sr/sr_sa_event.h>
# include <sr/sr_gl_render_funcs.h>
# include <controllers/me_ct_eyelid.h>
# include <controllers/me_ct_data_driven_reach.hpp>
# include <controllers/MeCtBodyReachState.h>
# include <controllers/me_ct_example_body_reach.hpp>
# include <controllers/me_ct_constraint.hpp>
# include <sbm/Physics/SbmColObject.h>
# include <controllers/me_ct_param_animation_data.h>
# include <sbm/GPU/SbmDeformableMeshGPU.h>
# include <sb/SBScene.h>
# include <sb/SBSkeleton.h>
# include <sb/SBSteerManager.h>
# include <sb/SBSteerAgent.h>
# include <sb/SBAnimationStateManager.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>

#if !defined (__ANDROID__) && !defined(SBM_IPHONE) // disable shader support
#include "sbm/GPU/SbmShader.h"
#include "sbm/GPU/SbmTexture.h"
#endif

#include <sbm/mcontrol_util.h>

//#include <sbm/SbmShader.h>

//#include "Heightfield.h"

////# define SR_USE_TRACE1  // basic fltk events
////# define SR_USE_TRACE2  // more fltk events
////# define SR_USE_TRACE3  // sr translated events
////# define SR_USE_TRACE4  // view_all
////# define SR_USE_TRACE5  // timeout
//# include <sr/sr_trace.h>

const int SHADOW_MAP_RES = 2048;

std::string Std_VS =
"varying vec4 vPos;\n\
varying vec3 normal;\n\
void main()\n\
{\n\
	vPos = gl_TextureMatrix[7]* gl_ModelViewMatrix * gl_Vertex;\n\
	gl_Position = ftransform();\n\
	normal = normalize(gl_NormalMatrix * gl_Normal);\n\
	gl_FrontColor = gl_FrontMaterial.diffuse*gl_LightSource[0].diffuse *vec4(max(dot(normal, normalize(gl_LightSource[0].position.xyz)), 0.0));\n\
	//gl_FrontColor = color;\n\
	gl_TexCoord[0] = gl_MultiTexCoord0;\n\
}";
std::string Std_FS = 
"uniform sampler2D tex;\n\
uniform sampler2D diffuseTex;\n\
uniform int useShadowMap;\n\
varying vec4 vPos;\n\
float shadowCoef()\n\
{\n\
	int index = 0;\n\
	vec4 shadow_coord = vPos/vPos.w;\n\
	shadow_coord.z += 0.000005;\n\
	float shadow_d = texture2D(tex, shadow_coord.st).x;\n\
	float diff = 1.0;\n\
	diff = shadow_d - shadow_coord.z;\n\
	return clamp( diff*850.0 + 1.0, 0.0, 1.0);\n\
}\n\
void main()\n\
{\n\
	const float shadow_ambient = 1.0;\n\
	float shadow_coef = 1.0;\n\
	if (useShadowMap == 1) \n\
		shadow_coef = shadowCoef();\n\
	vec4 texColor = texture2D(diffuseTex,gl_TexCoord[0].st);\n\
	if (length(texColor.rgb) < 0.01) texColor = vec4(1.0,1.0,1.0,1.0); \n\
	vec4 color = vec4(gl_Color.rgb*texColor.rgb, texColor.a);\n\
        gl_FragColor = vec4(color.rgb*shadow_coef*shadow_ambient,color.a);//color.a);//gl_Color*shadow_ambient * shadow_coef;\n\
}";


std::string Shadow_FS = 
"void main (void)\n\
{\n\
gl_FragColor = gl_Color;\n\
}";

//=============================== srSaSetShapesChanged ===========================================

class srSaSetShapesChanged : public SrSa
 { public :
    virtual bool shape_apply ( SrSnShapeBase* s ) { s->changed(true); return true; }
 };


//================================= popup menu ===================================================

static void menucb ( Fl_Widget* o, void* v ) 
 {
	 Fl_Widget* widget = o->parent();
	 
	 FltkViewer* viewer = NULL;
	 while (!viewer && widget && widget->parent() != NULL)
	 {
		widget = widget->parent();
		viewer = dynamic_cast<FltkViewer*>(widget);
	 }
	 if (viewer)
		 viewer->menu_cmd((FltkViewer::MenuCmd)(uintptr_t)v,o->label());
 }

# define MCB     ((Fl_Callback*)menucb)
# define CMD(c)  ((void*)FltkViewer::c)
const int NUM_GAZE_TYPES = 4;
const int NUM_REACH_TYPES = 2;

static char gaze_on_target_menu_name[] = {"&gaze"};
static char gaze_type_name[NUM_GAZE_TYPES][40] = {"&create EYE gaze","&create EYE NECK gaze","&create EYE CHEST gaze","&create EYE BACK gaze" }; 
static SrArray<Fl_Menu_Item> gaze_submenus[NUM_GAZE_TYPES];

Fl_Menu_Item GazeMenuTable[] = 
{
	{ gaze_type_name[0],   0, MCB, 0 },			
    { gaze_type_name[1],   0, MCB, 0 },
    { gaze_type_name[2],   0, MCB, 0 },
	{ gaze_type_name[3],   0, MCB, 0 },
	{ "&remove all gazes",   0, MCB, CMD(CmdRemoveAllGazeTarget) },
	{ 0 }
};

static char body_reach_menu_name[] = {"&reach"};
Fl_Menu_Item BodyReachMenuTable[] = 
{		
	{ "&show pose examples", 0, MCB, CMD(CmdReachShowExamples), 0 },
	{ "&no pose examples", 0, MCB, CMD(CmdReachNoExamples), 0 },	
	{ 0 }
};

Fl_Menu_Item MenuTable[] =
 { 
   { "&view all",   0, MCB, CMD(CmdViewAll) },
   { "&background", 0, MCB, CMD(CmdBackground) }, // FL_FL_MENU_DIVIDER

   { "&draw style", 0, 0, 0, FL_SUBMENU },
         { "&as is",   0, MCB, CMD(CmdAsIs),    FL_MENU_RADIO },
         { "d&efault", 0, MCB, CMD(CmdDefault), FL_MENU_RADIO },
         { "&smooth",  0, MCB, CMD(CmdSmooth),  FL_MENU_RADIO },
         { "&flat",    0, MCB, CMD(CmdFlat),    FL_MENU_RADIO },
         { "&lines",   0, MCB, CMD(CmdLines),   FL_MENU_RADIO },
         { "&points",  0, MCB, CMD(CmdPoints),  FL_MENU_RADIO },
         { 0 },
	{ "&shadows", 0, 0, 0, FL_SUBMENU },
         { "&no shadows",   0, MCB, CMD(CmdNoShadows),    FL_MENU_RADIO },
         { "&shadows",  0, MCB, CMD(CmdShadows),  FL_MENU_RADIO },
         { 0 },
    { "&characters", 0, 0, 0, FL_SUBMENU },
         { "&geometry", 0, MCB, CMD(CmdCharacterShowGeometry), FL_MENU_RADIO },
         { "&collision geometry", 0, MCB, CMD(CmdCharacterShowCollisionGeometry),   FL_MENU_RADIO },
         { "&deformable geometry", 0, MCB, CMD(CmdCharacterShowDeformableGeometry),   FL_MENU_RADIO },
		 { "&gpu deformable geometry", 0, MCB, CMD(CmdCharacterShowDeformableGeometryGPU),   FL_MENU_RADIO },
         { "&bones",   0, MCB, CMD(CmdCharacterShowBones),   FL_MENU_RADIO },
         { "&axis",   0, MCB, CMD(CmdCharacterShowAxis),   FL_MENU_RADIO },
         { 0 },
    { "p&references", 0, 0, 0, FL_SUBMENU },
		 { "&axis",         0, MCB, CMD(CmdAxis),        FL_MENU_TOGGLE },
		 { "b&ounding box", 0, MCB, CMD(CmdBoundingBox), FL_MENU_TOGGLE },
		 { "&statistics",   0, MCB, CMD(CmdStatistics),  FL_MENU_TOGGLE },
		 { 0 },
	{ "&pawns", 0, 0, 0, FL_SUBMENU },
		 { "&create pawn", 0, MCB, CMD(CmdCreatePawn), FL_MENU_DIVIDER},		 
         { "&no pawns shown", 0, MCB, CMD(CmdNoPawns), FL_MENU_RADIO },
         { "&show pawns as spheres", 0, MCB, CMD(CmdPawnShowAsSpheres),   FL_MENU_RADIO },        
         { 0 },
    { "&constraint", 0, 0, 0, FL_SUBMENU },
	     { "&use IK constraint", 0, MCB, CMD(CmdConstraintToggleIK), FL_MENU_TOGGLE},	
		 //{ "&use balance", 0, MCB, CMD(CmdConstraintToggleBalance), FL_MENU_TOGGLE},		 
		 { "&use reference joints", 0, MCB, CMD(CmdConstraintToggleReferencePose), FL_MENU_TOGGLE },		     
		 { 0 },    
    { gaze_on_target_menu_name, 0, 0, GazeMenuTable, FL_SUBMENU_POINTER }, 	
	{ body_reach_menu_name, 0, 0, BodyReachMenuTable, FL_SUBMENU_POINTER },        
    { "&terrain", 0, 0, 0, FL_SUBMENU },
         { "&no terrain",   0, MCB, CMD(CmdNoTerrain),    FL_MENU_RADIO },
         { "&terrain wireframe",  0, MCB, CMD(CmdTerrainWireframe),  FL_MENU_RADIO },
         { "&terrain",  0, MCB, CMD(CmdTerrain),  FL_MENU_RADIO },
         { 0 },
	{ "&eye beams", 0, 0, 0, FL_SUBMENU },
         { "&no eye beams",   0, MCB, CMD(CmdNoEyeBeams),    FL_MENU_RADIO },
         { "&eye beams",  0, MCB, CMD(CmdEyeBeams),  FL_MENU_RADIO },
         { 0 },
	{ "&eye lids", 0, 0, 0, FL_SUBMENU },
         { "&no eye lids",   0, MCB, CMD(CmdNoEyeLids),    FL_MENU_RADIO },
         { "&eye lids",  0, MCB, CMD(CmdEyeLids),  FL_MENU_RADIO },
         { 0 },
	{ "&dynamics", 0, 0, 0, FL_SUBMENU },
         { "&no dynamics",   0, MCB, CMD(CmdNoDynamics),    FL_MENU_RADIO },
         { "&show COM",  0, MCB, CMD(CmdShowCOM),  FL_MENU_RADIO },
         { "&show COM and support polygon",  0, MCB, CMD(CmdShowCOMSupportPolygon),  FL_MENU_RADIO },
		 { "&show masses",   0, MCB, CMD(CmdShowMasses),  FL_MENU_TOGGLE },
		 { "&show bounding volume",   0, MCB, CMD(CmdShowBoundingVolume),  FL_MENU_TOGGLE },
         { 0 },
	{ "&locomotion", 0, 0, 0, FL_SUBMENU },
         { "&enable locomotion",   0, MCB, CMD(CmdEnableLocomotion),    FL_MENU_TOGGLE },
         //{ "&show all",  0, MCB, CMD(CmdShowLocomotionAll),  FL_MENU_TOGGLE },
         { "&show velocity",  0, MCB, CMD(CmdShowVelocity),  FL_MENU_TOGGLE },
		 { "&show orientation",   0, MCB, CMD(CmdShowOrientation),  FL_MENU_TOGGLE },
		 { "&show selection",   0, MCB, CMD(CmdShowSelection),  FL_MENU_TOGGLE },
		 { "&show kinematic footprints",   0, MCB, CMD(CmdShowKinematicFootprints),  FL_MENU_TOGGLE },
		 { "&show locomotion footprints",   0, MCB, CMD(CmdShowLocomotionFootprints),  FL_MENU_TOGGLE },
		 { "&show trajectory", 0, MCB, CMD(CmdShowTrajectory), FL_MENU_TOGGLE },
		 { "&interactive",   0, MCB, CMD(CmdInteractiveLocomotion),  FL_MENU_TOGGLE },
         { 0 },
   { 0 }
 };


static void get_pawn_submenus(void* user_data,SrArray<Fl_Menu_Item>& menu_list)
{
	std::vector<SbmPawn*> pawn_list;
	ObjectManipulationHandle::get_pawn_list(pawn_list);
	for (unsigned int i=0;i<pawn_list.size();i++)
	{
		SbmPawn* pawn = pawn_list[i];
		//printf("pawn name = %s\n",pawn->name);
		Fl_Menu_Item temp_pawn = { pawn->getName().c_str(), 0, MCB, user_data } ;
		menu_list.push(temp_pawn);		
	}

	Fl_Menu_Item temp = {0};
	menu_list.push(temp);
}


void FltkViewer::update_submenus()
{
	for (int i=0;i<NUM_GAZE_TYPES;i++)
	{
		Fl_Menu_Item& gaze_menu = GazeMenuTable[i];	
		gaze_menu.flags |= FL_SUBMENU_POINTER;
		SrArray<Fl_Menu_Item>& menu_list = gaze_submenus[i];
		menu_list = SrArray<Fl_Menu_Item>();
		int iCmd = FltkViewer::CmdGazeOnTargetType1+i;
		Fl_Menu_Item select_pawn = { "selected pawn",   0, MCB,((void*)iCmd)  };
		menu_list.push(select_pawn);			
		get_pawn_submenus(select_pawn.user_data(),menu_list);
		const Fl_Menu_Item* pmenu = (const Fl_Menu_Item *)menu_list;
		gaze_menu.user_data((void*)pmenu);				
	}
}

# undef CMD
# undef MCB

// need to set/get data to be able to share the same popup menu with many instances of viewers

static void set_menu_data ( FltkViewer::RenderMode r, FltkViewer::CharacterMode c,
                            bool axis, bool bbox, bool stat)
 {
   # define SET(i,b)  if(b) MenuTable[i].set(); else MenuTable[i].clear();
#ifdef WIN32
   # define SETO(i)   MenuTable[i].setonly();
#else
   # define SETO(i)   
#endif
   # define CMD(i)    ((uintptr_t)(MenuTable[i].user_data_))

   uintptr_t i=0;
   while ( CMD(i)!=FltkViewer::CmdAsIs ) i++;          SETO (  i+(uintptr_t)r );      
   while ( CMD(i)!=FltkViewer::CmdCharacterShowGeometry ) i++; SETO (  i+(uintptr_t)c );
   while ( CMD(i)!=FltkViewer::CmdAxis ) i++;          SET  ( i, axis );
   while ( CMD(i)!=FltkViewer::CmdBoundingBox ) i++;   SET  ( i, bbox );
   while ( CMD(i)!=FltkViewer::CmdStatistics ) i++;    SET  ( i, stat );

# undef CMD
   # undef SETO
   # undef SET
 }


//================================= Internal Structures =================================


//===================================== FltkViewer =================================

// Called when the small "cross" button to close the window is pressed
static void _callback_func ( Fl_Widget* win, void* pt )
 {
   //LOG("DBG callback_func!\n");
   FltkViewer* v = (FltkViewer*)pt;
   v->close_requested ();
 }

FltkViewer::FltkViewer ( int x, int y, int w, int h, const char *label )
         : SrViewer(x, y, w, h) , Fl_Gl_Window ( x, y, w, h, label )
 {
	 Fl::gl_visual( FL_RGB | FL_DOUBLE | FL_DEPTH );//| FL_ALPHA );

   callback ( _callback_func, this );

   resizable(this);

   _data = new FltkViewerData();
   _locoData = new LocomotionData();
   _paLocoData = new PALocomotionData();

   _data->root = new SrSnGroup; // we maintain root pointer always valid
   _data->rendermode = ModeAsIs;
   _data->charactermode = ModeShowGeometry;
   _data->pawnmode = ModePawnShowAsSpheres;
   _data->shadowmode = ModeNoShadows;
   _data->terrainMode = ModeTerrain;
   _data->eyeBeamMode = ModeNoEyeBeams;
   _data->eyeLidMode = ModeNoEyeLids;
   _data->dynamicsMode = ModeNoDynamics;
   _data->locomotionMode = ModeEnableLocomotion;
   _data->reachRenderMode = ModeNoExamples;
   _data->steerMode = ModeNoSteer;
   _data->gridMode = ModeShowGrid;

   _data->iconized    = false;
   _data->statistics  = false;
   _data->displayaxis = false;
   _data->boundingbox = false;
   _data->scene_received_event = false;
   _data->showgeometry = false;
   _data->showcollisiongeometry = false;
   _data->showbones = true;
   _data->showaxis = false;
   _data->showmasses = false;
   _data->showBoundingVolume = false;
   _data->showlocomotionall = false;
   _data->showvelocity = false;
   _data->showorientation = false;
   _data->showselection = false;
   _data->showlocofootprints = false;
   _data->showkinematicfootprints = false;
   _data->interactiveLocomotion = false;
   _data->showtrajectory = false;

   _data->light.init();

   _data->bcolor = SrColor(.63f, .63f, .63f);

   _data->scenebox = new SrSnLines;
   _data->sceneaxis = new SrSnLines;

   user_data ( (void*)(this) ); // to be retrieved by the menu callback

   create_popup_menus();   

   gridColor[0] = 0.5;
   gridColor[1] = 0.5;
   gridColor[2] = 0.5;
   gridColor[3] = 0.5;
   gridHighlightColor[0] = .0;
   gridHighlightColor[1] = .0;
   gridHighlightColor[2] = .0;
   gridSize = 200.0;
   gridStep = 20.0;
//   gridSize = 400.0;
//   gridStep = 50.0;
   gridList = -1;
   _arrowTime = 0.0f;

   init_foot_print();
   _lastSelectedCharacter = "";

   // init timer update for keyboard
   Fl::add_timeout(0.01,timerUpdate,_paLocoData);
}

void FltkViewer::create_popup_menus()
{
	update_submenus();   
	begin();
    _data->menubut = new Fl_Menu_Button(0,0,50,50);
	_data->menubut->type(Fl_Menu_Button::POPUP23);
    _data->menubut->menu(MenuTable);   
    _data->menubut->textsize(12);	
    end();
}

FltkViewer::~FltkViewer ()
 {
   _data->root->unref ();
   delete _data->scenebox;
   delete _data->sceneaxis;
   delete _data;
}

SrSn *FltkViewer::root ()
 { 
   return _data->root; 
 }

void FltkViewer::root ( SrSn *r )
 { 
   if ( r==_data->root ) return;
   if ( !r ) r = new SrSnGroup;
   _data->root->unref();
   _data->root = r; 
   _data->root->ref();
 }

void FltkViewer::draw_message ( const char* s )
 {
   if ( _data->message!=s ) redraw();
   _data->message.set(s);
 }

void FltkViewer::show_menu ()
 { 
	create_popup_menus();
	set_menu_data (_data->rendermode, _data->charactermode, _data->displayaxis,
                   _data->boundingbox, _data->statistics);
	const Fl_Menu_Item *m = _data->menubut->popup();	
	
	if ( m ) 
	{		
		//m->do_callback(_data->menubut,m->user_data());     
		// instead of doing the callback function, call the menu command directly.
		menu_cmd((FltkViewer::MenuCmd)(uintptr_t)m->user_data(),m->label());
	}
	//MenuTable->popup();	
 }

void FltkViewer::applyToCharacters()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		// set the visibility parameters of the scene
		//character->scene_p->set_visibility(_data->showbones,_data->showgeometry, _data->showcollisiongeometry, _data->showaxis);

		// feng : never show the collision mesh, instead we will show the bounding volumes as capsules
		if (character->scene_p && character->dMeshInstance_p)
		{
			character->scene_p->set_visibility(_data->showbones,_data->showgeometry, false, _data->showaxis);
			character->dMeshInstance_p->setVisibility(_data->showdeformablegeometry);
		}
	}
}

void FltkViewer::menu_cmd ( MenuCmd s, const char* label  )
 {
	 bool applyToCharacter = false; 	 
	 MeCtExampleBodyReach* bodyReachCt = getCurrentCharacterBodyReachController();
	 MeCtConstraint* constraintCt = getCurrentCharacterConstraintController();

   switch ( s )
    { 
      case CmdViewAll : view_all (); break;

      case CmdAsIs   : _data->rendermode = ModeAsIs;
                       _data->render_action.restore_render_mode ( _data->root );
                       break;
      case CmdDefault : _data->rendermode = ModeDefault;
                       _data->render_action.override_render_mode ( _data->root, srRenderModeDefault );
                       break;
      case CmdSmooth : _data->rendermode = ModeSmooth;
                       _data->render_action.override_render_mode ( _data->root, srRenderModeSmooth );
                       break;
      case CmdFlat   : _data->rendermode = ModeFlat;
                       _data->render_action.override_render_mode ( _data->root, srRenderModeFlat );
                       break;
      case CmdLines  : _data->rendermode = ModeLines;
                       _data->render_action.override_render_mode ( _data->root, srRenderModeLines );
                       break;
      case CmdPoints : _data->rendermode = ModePoints;
                       _data->render_action.override_render_mode ( _data->root, srRenderModePoints );
                       break;

      case CmdAxis : SR_SWAPB(_data->displayaxis); 
                     if ( _data->displayaxis ) update_axis();
                     break;
	  case CmdShadows  : _data->shadowmode = ModeShadows;             
                       break;
      case CmdNoShadows : _data->shadowmode = ModeNoShadows;
                       break;
	  case CmdGrid: 
					   _data->gridMode = ModeShowGrid;
					   break;
	  case CmdNoGrid: 
					   _data->gridMode = ModeNoGrid;
					   break;
	  case CmdNoTerrain  : _data->terrainMode = ModeNoTerrain;             
                       break;
      case CmdTerrainWireframe : _data->terrainMode = ModeTerrainWireframe;
                       break;
      case CmdTerrain : _data->terrainMode = ModeTerrain;
                       break;
	  case CmdNoEyeBeams  : _data->eyeBeamMode = ModeNoEyeBeams;             
                       break;
      case CmdEyeBeams: _data->eyeBeamMode = ModeEyeBeams;
                       break;
	  case CmdNoEyeLids  : _data->eyeLidMode = ModeNoEyeLids;             
                       break;
	  case CmdEyeLids: _data->eyeLidMode = ModeEyeLids;
                       break;
	  case CmdNoDynamics  : _data->dynamicsMode = ModeNoDynamics;             
                       break;
      case CmdShowCOM:		_data->dynamicsMode = ModeShowCOM;
                       break;
	  case CmdShowCOMSupportPolygon: _data->dynamicsMode = ModeShowCOMSupportPolygon;
                       break;
	  case CmdShowMasses: _data->showmasses =  !_data->showmasses;
                       break;
	  case CmdShowBoundingVolume: _data->showBoundingVolume =  !_data->showBoundingVolume;
					   break;
	  case CmdEnableLocomotion  : _data->locomotionenabled = !_data->locomotionenabled;             
                       break;
	  case CmdShowLocomotionAll  : _data->showlocomotionall = !_data->showlocomotionall;
						if(_data->showlocomotionall)
						{
							_data->showvelocity = true;
							_data->showorientation = true;
							_data->showselection = true;

						}
						else
						{
							_data->showvelocity = false;
							_data->showorientation = false;
							_data->showselection = false;
						}
                       break;
      case CmdShowVelocity  : _data->showvelocity = !_data->showvelocity;
						if(!_data->showvelocity) _data->showlocomotionall = false;
                       break;
	  case CmdShowOrientation  : _data->showorientation = !_data->showorientation;
						if(!_data->showorientation) _data->showlocomotionall = false;
                       break;
	  case CmdShowSelection  : _data->showselection = !_data->showselection;
						if(!_data->showselection) _data->showlocomotionall = false;
                       break;
	  case CmdShowKinematicFootprints  : _data->showkinematicfootprints = !_data->showkinematicfootprints;
						if(!_data->showkinematicfootprints) _data->showlocomotionall = false;
                       break;
	  case CmdShowLocomotionFootprints  : _data->showlocofootprints = !_data->showlocofootprints;
						if(!_data->showlocofootprints) _data->showlocomotionall = false;
                       break;
	  case CmdShowTrajectory : _data->showtrajectory = !_data->showtrajectory;
						if (!_data->showtrajectory) _data->showtrajectory = false;
					   break;
	  case CmdInteractiveLocomotion  : _data->interactiveLocomotion = !_data->interactiveLocomotion;
                       break;
      case CmdBoundingBox : SR_SWAPB(_data->boundingbox); 
                            if ( _data->boundingbox ) update_bbox();
                            break;

      case CmdStatistics : SR_SWAPB(_data->statistics); break;
	  case CmdCharacterShowGeometry:
		  _data->charactermode = ModeShowGeometry;		  
		  _data->showgeometry = true;
		  _data->showcollisiongeometry = false;
		  _data->showdeformablegeometry = false;
		  _data->showbones = false;
		  _data->showaxis = false;
		  applyToCharacter = true;
		  break;
	  case CmdCharacterShowCollisionGeometry: 
		  _data->charactermode = ModeShowCollisionGeometry;		
		  _data->showgeometry = false;
		  _data->showcollisiongeometry = true;
		  _data->showdeformablegeometry = false;
		  _data->showbones = false;
		  _data->showaxis = false;
		  applyToCharacter = true;
		  break;
	  case CmdCharacterShowDeformableGeometry: 
		  _data->charactermode = ModeShowDeformableGeometry;		
		  SbmDeformableMeshGPU::useGPUDeformableMesh = false;
		  _data->showgeometry = false;
		  _data->showcollisiongeometry = false;
		  _data->showdeformablegeometry = true;
		  _data->showbones = false;
		  _data->showaxis = false;
		  applyToCharacter = true;
		  break;
	  case CmdCharacterShowDeformableGeometryGPU: 
		  _data->charactermode = ModeShowDeformableGeometryGPU;		
		  SbmDeformableMeshGPU::useGPUDeformableMesh = true;
		  _data->showgeometry = false;
		  _data->showcollisiongeometry = false;
		  _data->showdeformablegeometry = true;
		  _data->showbones = false;
		  _data->showaxis = false;
		  applyToCharacter = true;
		  break;
	  case CmdCharacterShowBones: 
		  _data->charactermode = ModeShowBones;		
		  _data->showgeometry = false;
		  _data->showcollisiongeometry = false;
		  _data->showdeformablegeometry = false;
		  _data->showbones = true;
		  _data->showaxis = false;
		  applyToCharacter = true;
		  break;
	  case CmdCharacterShowAxis: 
		  _data->charactermode = ModeShowAxis;		
		  _data->showgeometry = false;
		  _data->showcollisiongeometry = false;
		  _data->showdeformablegeometry = false;
		  _data->showbones = false;
		  _data->showaxis = true;
		  applyToCharacter = true;
		  break;
	  case CmdCreatePawn : 
						create_pawn();						
						break;
	  case CmdNoPawns : _data->pawnmode = ModeNoPawns;
                       break;
	  case CmdPawnShowAsSpheres  : _data->pawnmode = ModePawnShowAsSpheres;             
                       break;
	  case CmdGazeOnTargetType1:	
	  case CmdGazeOnTargetType2:
	  case CmdGazeOnTargetType3:
	  case CmdGazeOnTargetType4:
					   set_gaze_target(s-CmdGazeOnTargetType1,label);
					   break;
	  case CmdRemoveAllGazeTarget:
		               set_gaze_target(-1,NULL);
					   break;	  
	  case CmdReachShowExamples:
		  _data->reachRenderMode = ModeShowExamples;
		  break;
	  case CmdReachNoExamples:
		  _data->reachRenderMode = ModeNoExamples;
		  break;
	  case CmdNoSteer: 
		   _data->steerMode = ModeNoSteer;
		   break;
	   case CmdSteerAll: 
		   _data->steerMode = ModeSteerAll;
		   break;
	   case CmdSteerCharactersGoalsOnly:
			_data->steerMode = ModeSteerCharactersGoalsOnly;
		 break;
	  case CmdConstraintToggleIK:
		  if (constraintCt)
		  {
			  //bodyReachCt->useDataDriven = !reachCt->useDataDriven;
			  //bodyReachCt->useBalance = !bodyReachCt->useBalance;
			  if (constraintCt->useIKConstraint)
			  {
				  //bodyReachCt->useIKConstraint = false;
				  constraintCt->setFadeOut(2.0);
			  }
			  else
			  {
				  constraintCt->useIKConstraint = true;
				  constraintCt->setFadeIn(2.0);
			  }
		  }
		  break;
	}
	
	if (applyToCharacter)
	{
		applyToCharacters();						
	}
	render ();
 }



bool FltkViewer::menu_cmd_activated ( MenuCmd c )
 {
   switch ( c )
    { case CmdAsIs     : return _data->rendermode==ModeAsIs? true:false;
      case CmdDefault  : return _data->rendermode==ModeDefault? true:false;
      case CmdSmooth   : return _data->rendermode==ModeSmooth? true:false;
      case CmdFlat     : return _data->rendermode==ModeFlat? true:false;
      case CmdLines    : return _data->rendermode==ModeLines? true:false;
      case CmdPoints   : return _data->rendermode==ModePoints? true:false;
      case CmdShadows   : return _data->shadowmode==ModeShadows? true:false;
      case CmdNoShadows   : return _data->shadowmode==ModeNoShadows? true:false;
	  case CmdTerrain   : return _data->terrainMode==ModeTerrain? true:false;
      case CmdTerrainWireframe   : return _data->terrainMode==ModeTerrainWireframe? true:false;
	  case CmdNoTerrain   : return _data->terrainMode==ModeNoTerrain? true:false;
	  case CmdNoEyeBeams  : return _data->eyeBeamMode==ModeNoEyeBeams? true:false;
      case CmdEyeBeams   : return _data->eyeBeamMode==ModeEyeBeams? true:false;
	  case CmdNoEyeLids  : return _data->eyeLidMode==ModeNoEyeLids? true:false;
      case CmdEyeLids  : return _data->eyeLidMode==ModeEyeLids? true:false;
	  case CmdNoDynamics   : return _data->dynamicsMode==ModeNoDynamics? true:false;
      case CmdShowCOM   : return _data->dynamicsMode==ModeShowCOM? true:false;
	  case CmdShowCOMSupportPolygon   : return _data->dynamicsMode==ModeShowCOMSupportPolygon? true:false;
	  case CmdShowMasses : return _data->showmasses? true:false;
	  case CmdShowBoundingVolume : return _data->showBoundingVolume? true:false;
	  case CmdEnableLocomotion : return _data->locomotionenabled? true:false;
	  case CmdShowLocomotionAll : return _data->showlocomotionall? true:false;
	  case CmdShowVelocity : return _data->showvelocity? true:false;
	  case CmdShowOrientation : return _data->showorientation? true:false;
	  case CmdShowSelection : return _data->showselection? true:false;
	  case CmdShowKinematicFootprints : return _data->showkinematicfootprints? true:false;
	  case CmdShowTrajectory : return _data->showtrajectory ? true:false;
	  case CmdShowLocomotionFootprints : return _data->showlocofootprints? true:false;
      case CmdAxis        : return _data->displayaxis? true:false;
      case CmdBoundingBox : return _data->boundingbox? true:false;
      case CmdStatistics  : return _data->statistics? true:false;
	  case CmdCharacterShowGeometry : return _data->showgeometry? true:false;
	  case CmdCharacterShowCollisionGeometry : return _data->showcollisiongeometry? true:false;
	  case CmdCharacterShowDeformableGeometry : return _data->showdeformablegeometry? true:false;
	  case CmdCharacterShowBones : return _data->showbones? true:false;
	  case CmdCharacterShowAxis : return _data->showaxis? true:false;
      default : return false;
    }
 }

void FltkViewer::update_bbox ()
 {
   _data->bbox_action.apply ( _data->root );
   _data->scenebox->shape().init();
   _data->scenebox->shape().push_box ( _data->bbox_action.get(), true );
 }

void FltkViewer::update_axis ()
 {
   _data->bbox_action.apply ( _data->root );
   SrBox b = _data->bbox_action.get();
   float len1 = SR_MAX3(b.a.x,b.a.y,b.a.z);
   float len2 = SR_MAX3(b.b.x,b.b.y,b.b.z);
   float len = SR_MAX(len1,len2);

   _data->sceneaxis->shape().init();

   _data->sceneaxis->shape().push_axis ( SrPnt::null, len, 3, "xyz" );
 }

void FltkViewer::view_all ()
 {
   _data->camera.center = SrVec::null;
   _data->camera.up = SrVec::j;
   _data->camera.eye.set ( 0, 0, 1.0f );

   if ( _data->root )
    { update_bbox ();
      SrBox box = _data->bbox_action.get();

      if ( _data->displayaxis )
       { SrBox b;
         _data->sceneaxis->get_bounding_box(b);
         box.extend ( b );
       }

      float s = box.max_size();
      // _data->light.constant_attenuation = s;
      //_data->camera.view_all ( box, SR_TORAD(60) );
      _data->camera.scale = 1.0f / s;
      //_data->camera.center = box.center();
      //_data->camera.eye = _data->camera.center;
      //_data->camera.eye.z = s.z;
    }

   render ();
 }

void FltkViewer::render () 
 { 
   redraw(); 
 } 

bool FltkViewer::iconized () 
 { 
   return _data->iconized;
 }

float FltkViewer::fps () 
 { 
   return (float)_data->fcounter.mps(); 
 }

sruint FltkViewer::curframe () 
 { 
   return (sruint) _data->fcounter.measurements(); 
 }

SrColor FltkViewer::background ()
 {
   return _data->bcolor;
 }

void FltkViewer::background ( SrColor c )
 {
   _data->bcolor = c;
 }

SrCamera* FltkViewer::get_camera()
{
	return &_data->camera;
}

void FltkViewer::set_camera ( const SrCamera &cam )
 {
   _data->camera = cam;

 //  if ( _data->viewmode==ModeExaminer )
//    { _data->trackball.init();
  //    _data->trackball.rotation.set ( cam.eye-cam.center, SrVec::k );
   // }

//   if ( _data->root )
  //  { update_bbox ();
    //  SrBox box = _data->bbox_action.get();
      //float s = box.max_size();
    //  _data->light.constant_attenuation = s;
   // }
   //else _data->light.constant_attenuation = 1.0f; // the default value
 }

static void gl_draw_string ( const char* s, float x, float y )
 {
   glMatrixMode ( GL_PROJECTION );
   glLoadIdentity ();
   glMatrixMode ( GL_MODELVIEW );
   glLoadIdentity ();
   glDisable ( GL_LIGHTING );
   glColor ( SrColor::red );
   Fl::set_font(FL_TIMES, 12 ); // from fltk
   fl_draw(s, (int) x, (int) y );      // from fltk
 }

//-- Render  ------------------------------------------------------------------


void FltkViewer::initShadowMap()
{    
	// init basic shader for rendering 
	SbmShaderManager::singleton().addShader("Basic",Std_VS.c_str(),Std_FS.c_str(),false);
	SbmShaderManager::singleton().addShader("BasicShadow","",Shadow_FS.c_str(),false);
	// init shadow map and frame buffer 
	
	int depth_size = SHADOW_MAP_RES;
	//glGenTextures(1, &_data->shadowMapID);
	//LOG("Shadow map ID = %d\n",_data->shadowMapID);	
	glGenFramebuffersEXT(1, &_data->depthFB);	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _data->depthFB);
	glDrawBuffer(GL_FRONT_AND_BACK);
	

	glGenTextures(1, &_data->shadowMapID);
	glBindTexture(GL_TEXTURE_2D, _data->shadowMapID);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, depth_size, depth_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, depth_size, depth_size, 0, GL_RGBA, GL_FLOAT, NULL);	
	glBindTexture(GL_TEXTURE_2D,0);


	glGenTextures(1, &_data->depthMapID);
	glBindTexture(GL_TEXTURE_2D, _data->depthMapID);	
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, depth_size, depth_size, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);	
	
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    //glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);  
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, depth_size, depth_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D,0);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, _data->depthMapID,0);
	//glBindTexture(GL_TEXTURE_2D, _data->shadowMapID);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, _data->shadowMapID, 0);

	GLenum FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	/*
	glGenRenderbuffersEXT(1, &_data->rboID);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _data->rboID);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
		depth_size, depth_size);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	*/
}

void FltkViewer::init_opengl ( int w, int h )
 {   
   //sr_out<<"INIT"<<srnl;
   glViewport ( 0, 0, w, h );
   glEnable ( GL_DEPTH_TEST );
   glEnable ( GL_LIGHT0 ); 
   glEnable ( GL_LIGHTING );

   //glEnable ( GL_BLEND ); // for transparency
   //glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   glCullFace ( GL_BACK );
   glDepthFunc ( GL_LEQUAL );
   glFrontFace ( GL_CCW );

   glEnable ( GL_POLYGON_SMOOTH );

   //glEnable ( GL_LINE_SMOOTH );
   //glHint ( GL_LINE_SMOOTH_HINT, GL_NICEST );

   glEnable ( GL_POINT_SMOOTH );
   glPointSize ( 2.0 );

   glShadeModel ( GL_SMOOTH );

   SrCamera& cam = _data->camera;
   //cam.zfar = 1000000;
   float scale = 1.f/SmartBody::SBScene::getScene()->getScale();
   
   // camera near = 0.1 m, camera far plane is 100 m
   cam.znear = 0.1f*scale;
   cam.zfar  = 100.f*scale;


   // init shader
   //initShadowMap();
   //SbmShaderManager::singleton().addShader("Basic","",Std_FS.c_str(),false);


   updateLights();
 }

void FltkViewer::close_requested ()
 {
   exit ( 0 );
 }

//# include <sr/sr_sphere.h>
//static SrSnSphere* SPH=0;
//   if ( !SPH ) SPH = new SrSnSphere;
//   SPH->shape().center = light.position;
//   SPH->shape().radius = 0.6f;
//   SPH->color ( SrColor::red );
//   _data->render_action.apply ( SPH );

/////////////////////////////////////////////////////////////////////////////////////

void MakeShadowMatrix( GLfloat points[3][3], GLfloat light[4], GLfloat matrix[4][4] )	{
	GLfloat plane[ 2 ][ 3 ];
    GLfloat coeff[4];
    GLfloat dot;

    // Find the plane equation coefficients
    // Find the first three coefficients the same way we find a normal.
//    calcNormal(points,planeCoeff);

	plane[ 0 ][ 0 ] = points[ 1 ][ 0 ] - points[ 0 ][ 0 ];
	plane[ 0 ][ 1 ] = points[ 1 ][ 1 ] - points[ 0 ][ 1 ];
	plane[ 0 ][ 2 ] = points[ 1 ][ 2 ] - points[ 0 ][ 2 ];
	plane[ 1 ][ 0 ] = points[ 2 ][ 0 ] - points[ 1 ][ 0 ];
	plane[ 1 ][ 1 ] = points[ 2 ][ 1 ] - points[ 1 ][ 1 ];
	plane[ 1 ][ 2 ] = points[ 2 ][ 2 ] - points[ 1 ][ 2 ];
	
	coeff[ 0 ] = plane[ 0 ][ 1 ] * plane[ 1 ][ 2 ] - plane[ 0 ][ 2 ] * plane[ 1 ][ 1 ];
	coeff[ 1 ] = plane[ 0 ][ 2 ] * plane[ 1 ][ 0 ] - plane[ 0 ][ 0 ] * plane[ 1 ][ 2 ];
	coeff[ 2 ] = plane[ 0 ][ 0 ] * plane[ 1 ][ 1 ] - plane[ 0 ][ 1 ] * plane[ 1 ][ 0 ];

    // Find the last coefficient by back substitutions
    coeff[3] = -( ( coeff[ 0 ] * points[ 2 ][ 0 ] ) + ( coeff[ 1 ] * points[ 2 ][ 1 ] ) + ( coeff[ 2 ] * points[ 2 ][ 2 ] ) );

    // Dot product of plane and light position
    dot = coeff[0] * light[0] + coeff[1] * light[1] + coeff[2] * light[2] + coeff[3] * light[3];

    matrix[0][0] = dot - light[0] * coeff[0];
    matrix[1][0] = 0.0f - light[0] * coeff[1];
    matrix[2][0] = 0.0f - light[0] * coeff[2];
    matrix[3][0] = 0.0f - light[0] * coeff[3];

    matrix[0][1] = 0.0f - light[1] * coeff[0];
    matrix[1][1] = dot - light[1] * coeff[1];
    matrix[2][1] = 0.0f - light[1] * coeff[2];
    matrix[3][1] = 0.0f - light[1] * coeff[3];

    matrix[0][2] = 0.0f - light[2] * coeff[0];
    matrix[1][2] = 0.0f - light[2] * coeff[1];
    matrix[2][2] = dot - light[2] * coeff[2];
    matrix[3][2] = 0.0f - light[2] * coeff[3];

    matrix[0][3] = 0.0f - light[3] * coeff[0];
    matrix[1][3] = 0.0f - light[3] * coeff[1];
    matrix[2][3] = 0.0f - light[3] * coeff[2];
    matrix[3][3] = dot - light[3] * coeff[3];
}

/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////

void FltkViewer::updateLights()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// get any pawns called 'light#' 
	// if none exist, use the standard lights
	_lights.clear();
	std::map<std::string, SbmPawn*>& pawnMap = mcu.getPawnMap();
	for (std::map<std::string, SbmPawn*>::iterator iter = pawnMap.begin();
		 iter != pawnMap.end();
		 iter++)
	{
		SbmPawn* pawn = (*iter).second;
		const std::string& name = pawn->getName();
		if (name.find("light") == 0)
		{
			SmartBody::SBPawn* sbpawn = dynamic_cast<SmartBody::SBPawn*>(pawn);
			SrLight light;
			light.directional = true;
			light.diffuse = SrColor( 1.0f, 0.95f, 0.8f );
			light.position = sbpawn->getPosition();
			light.constant_attenuation = 1.0f;
			_lights.push_back(light);
		}
	}
	//LOG("light size = %d\n",_lights.size());
	
	if (_lights.size() == 0)
	{
		SrLight light;
		light.directional = false;
		light.directional = true;
		light.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
		light.position = SrVec( 100.0, 250.0, 400.0 );
	//	light.constant_attenuation = 1.0f/cam.scale;
		light.constant_attenuation = 1.0f;
		_lights.push_back(light);

		SrLight light2 = light;
		light2.directional = false;
		light2.diffuse = SrColor( 1.0f, 1.0f, 1.0f );
		light2.position = SrVec( 100.0, 500.0, -200.0 );
	//	light2.constant_attenuation = 1.0f;
	//	light2.linear_attenuation = 2.0f;
		_lights.push_back(light2);
	}
	
}

void cameraInverse(float* dst, float* src)
{
	dst[0] = src[0];
	dst[1] = src[4];
	dst[2] = src[8];
	dst[3] = 0.0f;
	dst[4] = src[1];
	dst[5] = src[5];
	dst[6]  = src[9];
	dst[7] = 0.0f;
	dst[8] = src[2];
	dst[9] = src[6];
	dst[10] = src[10];
	dst[11] = 0.0f;
	dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
	dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
	dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
	dst[15] = 1.0f;
}

void FltkViewer::drawAllGeometries(bool shadowPass)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	// update deformable mesh
    bool hasGPUSupport = SbmShaderManager::getShaderSupport() != SbmShaderManager::NO_GPU_SUPPORT;
	SrMat shadowTexMatrix;
	if (_data->shadowmode == ModeShadows && !shadowPass && hasGPUSupport) // update the texture transform matrix
	{		
		float cam_inverse_modelview[16];
		const float bias[16] = {	0.5f, 0.0f, 0.0f, 0.0f, 
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f	};		
		//glGetFloatv(GL_MODELVIEW_MATRIX, cam_modelview);
		SrMat viewMat;
		viewMat = _data->camera.get_view_mat(viewMat);
		cameraInverse(cam_inverse_modelview, viewMat.pt(0));		
		// since gluLookAt gives us an orthogonal matrix, we speed up the inverse computation
		//cameraInverse(cam_inverse_modelview, cam_modelview);		
		glActiveTexture(GL_TEXTURE7);
		glMatrixMode(GL_TEXTURE);		
		glLoadMatrixf(bias);
		glMultMatrixf(_data->shadowCPM);
		// multiply the light's (bias*crop*proj*modelview) by the inverse camera modelview
		// so that we can transform a pixel as seen from the camera
		glMultMatrixf(cam_inverse_modelview);	
		glCullFace(GL_BACK);
		SbmDeformableMeshGPU::shadowMapID = _data->depthMapID;
	}
	else
	{
		SbmDeformableMeshGPU::shadowMapID = -1;
	}
	
	bool updateSim = mcu.update_timer();
	SbmDeformableMeshGPU::useShadowPass = shadowPass;
	for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		if(pawn->dMesh_p && pawn->dMeshInstance_p)
		{
			//pawn->dMesh_p->update();
			pawn->dMeshInstance_p->update();
		}
	}
	

	_data->fcounter.start();
	if ( _data->displayaxis ) _data->render_action.apply ( _data->sceneaxis );
	if ( _data->boundingbox ) _data->render_action.apply ( _data->scenebox );

	
    if (hasGPUSupport)// && _data->shadowmode == ModeShadows)
    {
        std::string shaderName = _data->shadowmode == ModeShadows && !shadowPass ? "Basic" : "BasicShadow";
	    SbmShaderProgram* basicShader = SbmShaderManager::singleton().getShader(shaderName);
	    GLuint program = basicShader->getShaderProgram();
	    if (_data->shadowmode == ModeShadows && !shadowPass)
			glUseProgram(program);		
	    GLuint useShadowMapLoc = glGetUniformLocation(program,"useShadowMap");
	    if (_data->shadowmode == ModeShadows && !shadowPass) // attach the texture
	    {		
    		
		    glActiveTexture(GL_TEXTURE7);
		    glBindTexture(GL_TEXTURE_2D, _data->depthMapID);
		    //glMatrixMode(GL_TEXTURE);
		    //glLoadMatrixf(shadowTexMatrix.pt(0));
		    glCullFace(GL_BACK);
		    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		    glUniform1i(glGetUniformLocation(program, "tex"), 7); 	
			glUniform1i(glGetUniformLocation(program, "diffuseTex"), 0); 
		    glMatrixMode(GL_MODELVIEW);			
		    glUniform1i(useShadowMapLoc,1);		
	    }
	    else
	    {
			glUniform1i(glGetUniformLocation(program, "diffuseTex"), 0); 
		    glUniform1i(useShadowMapLoc,0);		
	    }
    }

	if( _data->root )	{		
		_data->render_action.apply ( _data->root );
	}	
	
#if USE_OGRE_VIEWER  < 1 // ogre will draw its own floor
	static GLfloat mat_emissin[] = { 0.f,  0.f,    0.f,    1.f };
	static GLfloat mat_ambient[] = { 0.f,  0.f,    0.f,    1.f };
	static GLfloat mat_diffuse[] = { 0.8f,  0.8f,    0.8f,    1.f };
	static GLfloat mat_speclar[] = { 0.f,  0.f,    0.f,    1.f }; 
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emissin );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_speclar );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 0.0 );
	glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	glEnable(GL_LIGHTING);
	float floorSize = 1200;
	float planeY = -0.0f;
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glNormal3f(0,1,0);
	glVertex3f(-floorSize,planeY,floorSize);	
	glTexCoord2f(0,1);
	glNormal3f(0,1,0);
	glVertex3f(floorSize,planeY,floorSize);
	glTexCoord2f(1,1);
	glNormal3f(0,1,0);
	glVertex3f(floorSize,planeY,-floorSize);
	glTexCoord2f(1,0);
	glNormal3f(0,1,0);
	glVertex3f(-floorSize,planeY,-floorSize);	
	glEnd();
#endif

	drawPawns();
	
	glDisable(GL_LIGHTING);
	
	glUseProgram(0);	
}


   
void FltkViewer::draw() 
{	
	if ( !visible() ) return;

	if ( !valid() ) 
	{
		init_opengl ( w(), h() ); // valid() is turned on by fltk after draw() returns
		//hasShaderSupport = SbmShaderManager::initGLExtension();	   
	} 	

	SbmShaderManager& ssm = SbmShaderManager::singleton();
	SbmTextureManager& texm = SbmTextureManager::singleton();

	if (!context_valid())
	{		
		bool hasShaderSupport = ssm.initGLExtension();
        if (hasShaderSupport)
		    initShadowMap();
	}
	

	//make_current();
	//wglMakeCurrent(fl_GetDC(fl_xid(this)),(HGLRC)context());
	//LOG("viewer GL context = %d, current context = %d",context(), wglGetCurrentContext());

	
   
   bool hasOpenGL        = ssm.initOpenGL();
   bool hasShaderSupport = false;
   // init OpenGL extension
   if (hasOpenGL)
   {
	   hasShaderSupport = ssm.initGLExtension();		
   }
   // update the shader map
   if (hasShaderSupport)
   {
	   ssm.buildShaders();
	   texm.updateTexture();
   }	

   if (_objManipulator.hasPicking())
   {
		SrVec2 pick_loc = _objManipulator.getPickLoc();
		_objManipulator.picking(pick_loc.x,pick_loc.y, _data->camera);	   
   }  

   mcuCBHandle& mcu = mcuCBHandle::singleton();
   glViewport ( 0, 0, w(), h() );
   SrLight &light = _data->light;
   SrCamera &cam  = _data->camera;
   SrMat mat ( SrMat::NotInitialized );


   //----- Clear Background --------------------------------------------
   glClearColor ( _data->bcolor );
   glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   //----- Set Projection ----------------------------------------------
   cam.aspect = (float)w()/(float)h();

   glMatrixMode ( GL_PROJECTION );
   glLoadMatrix ( cam.get_perspective_mat(mat) );

   //----- Set Visualisation -------------------------------------------
   glMatrixMode ( GL_MODELVIEW );
   glLoadMatrix ( cam.get_view_mat(mat) );

   glScalef ( cam.scale, cam.scale, cam.scale );

    updateLights();
	glEnable ( GL_LIGHTING );
	for (size_t x = 0; x < _lights.size(); x++)
	{
		glLight ( x, _lights[x] );		
	}

	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.08f);	

	static GLfloat mat_emissin[] = { 0.0,  0.0,    0.0,    1.0 };
	static GLfloat mat_ambient[] = { 0.0,  0.0,    0.0,    1.0 };
	static GLfloat mat_diffuse[] = { 1.0,  1.0,    1.0,    1.0 };
	static GLfloat mat_speclar[] = { 0.0,  0.0,    0.0,    1.0 };
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emissin );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_speclar );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 0.0 );
	glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	glEnable( GL_COLOR_MATERIAL );
	glEnable( GL_NORMALIZE );

	if (_data->terrainMode == FltkViewer::ModeTerrain)
		mcu.render_terrain(0);
	else if (_data->terrainMode == FltkViewer::ModeTerrainWireframe)
		mcu.render_terrain(1);

	glDisable( GL_COLOR_MATERIAL );


	

   //----- Render user scene -------------------------------------------

	//glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	if (_data->shadowmode == ModeShadows && hasShaderSupport)
		makeShadowMap();

	drawAllGeometries();		
   // draw the grid
	//   if (gridList == -1)
	//	   initGridList();	
	drawGrid();
	drawSteeringInfo();
	drawEyeBeams();
	drawEyeLids();
	drawDynamics();
	drawLocomotion();
	

	if (_data->showcollisiongeometry)
		drawCharacterPhysicsObjs();
	if (_data->showBoundingVolume)
		drawCharacterBoundingVolumes();

	drawInteractiveLocomotion();
	//_posControl.Draw();
	_objManipulator.draw(cam);
	// feng : debugging draw for reach controller
	drawReach();

	_data->fcounter.stop();

   if ( _data->message.len() )
    { gl_draw_string ( _data->message, -1, -1 );
    }
   else if ( _data->statistics )
    { _data->message.setf ( "FPS:%5.2f frame(%2.0f):%4.1fms render:%4.1fms", 
                  _data->fcounter.mps(),
                  _data->fcounter.measurements(),
                  _data->fcounter.loopdt()*1000.0,
                  _data->fcounter.meandt()*1000.0 );
      gl_draw_string ( _data->message, -1.0f, -1.0f );
      _data->message.set ( "" ); // will not deallocate the string but set len==0
    }

   //----- Fltk will then flush and swap buffers -----------------------------
 }

// Fl::event_x/y() variates from (0,0) to (w(),h())
// transformed coords in SrEvent are in "normalized device coordinates" [-1,-1]x[1,1]
static void translate_event ( SrEvent& e, SrEvent::EventType t, int w, int h, FltkViewer* viewer )
 {
   e.init_lmouse ();

   e.type = t;

   // put coordinates inside [-1,1] with (0,0) in the middle :
   e.mouse.x  = ((float)Fl::event_x())*2.0f / ((float)w) - 1.0f;
   e.mouse.y  = ((float)Fl::event_y())*2.0f / ((float)h) - 1.0f;
   e.mouse.y *= -1.0f;
   e.width = w;
   e.height = h;
   e.mouseCoord.x = (float)Fl::event_x();
   e.mouseCoord.y = (float)Fl::event_y();  

   if ( t==SrEvent::EventPush)
   {
	   e.button = Fl::event_button();
	   e.origUp = viewer->getData()->camera.up;
	   e.origEye = viewer->getData()->camera.eye;
	   e.origCenter = viewer->getData()->camera.center;
	   e.origMouse.x = e.mouseCoord.x;
	   e.origMouse.y = e.mouseCoord.y;
   }
//    else if ( t==SrEvent::EventDrag)
//    {
// 	   e.origMouse.x = e.mouseCoord.x;
// 	   e.origMouse.y = e.mouseCoord.y;	   
//    }
   else if (t==SrEvent::EventRelease )
   {
	   e.button = Fl::event_button();
	   e.origMouse.x = -1;
	   e.origMouse.y = -1;
   }


   if ( Fl::event_state(FL_BUTTON1) ) e.button1 = 1;
   if ( Fl::event_state(FL_BUTTON2) ) e.button2 = 1;
   if ( Fl::event_state(FL_BUTTON3) ) e.button3 = 1;

   if ( Fl::event_state(FL_ALT)   ) e.alt = 1;
   if ( Fl::event_state(FL_CTRL)  ) e.ctrl = 1;

   if ( Fl::event_state(FL_SHIFT) ) e.shift = 1;
   
   e.key = Fl::event_key();

 }



void FltkViewer::translate_keyboard_state()
{
	_paLocoData->prevJumping = _paLocoData->jumping;
	_paLocoData->prevStarting = _paLocoData->starting;
	_paLocoData->prevStopping = _paLocoData->stopping;
	bool locomotion_cmd = false;
	bool paLocomotionCmd = false;
	float prevV = _paLocoData->v;
	float prevW = _paLocoData->w;
	float scoot = 0.0f;
	char cmd[300];
	cmd[0] = '\0';
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if(_locoData->x_flag == 0 && _locoData->z_flag == 0)
	{
		_locoData->rps_flag = 0;
		_locoData->z_flag = 1;
		_locoData->x_flag = 0;
		_locoData->spd = _locoData->z_spd;
		sprintf(_locoData->t_direction, "forward ");
	}

	int counter = 0;
	SbmCharacter* character = NULL;
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		if (counter == _locoData->char_index)
		{
			character = (*iter).second;
			sprintf(_locoData->character, "char %s ", character->getName().c_str());
		}
		counter++;
	}
	if (!character)
		return;


	sprintf(cmd, "test loco ");
	strcat(cmd, _locoData->character);


	if(Fl::event_key('r'))
	{
		_locoData->height_disp += _locoData->height_disp_delta;
		//if(height_disp > 0.0f) height_disp = 0.0f;
		if (character->get_locomotion_ct())
			character->get_locomotion_ct()->set_target_height_displacement(_locoData->height_disp);
	}

// 	if (Fl::event_key('c'))
// 	{
// 		LOG("push c");
//  		MeCtExampleBodyReach* bodyReachCt = getCurrentCharacterBodyReachController();
// 		if (bodyReachCt)
// 		{
// 			bodyReachCt->simplexIndex = (bodyReachCt->simplexIndex + 1)%bodyReachCt->simplexList.size();
// 		}
// 	}

	if(Fl::event_key('f'))
	{
		_locoData->height_disp -= _locoData->height_disp_delta;
		//if(height_disp < -50.0f) height_disp = -50.0f;
		if (character && character->get_locomotion_ct())
			character->get_locomotion_ct()->set_target_height_displacement(_locoData->height_disp);
	}
	if(Fl::event_key('k'))
	{
		++_locoData->off_height_comp;
	}
	if(Fl::event_key('m'))
	{
		--_locoData->off_height_comp;
	}
	if(Fl::event_key('x'))
	{
		SbmCharacter* actor = NULL;		
		//for(int i = 0; i < mcu.character_map.get_num_entries(); ++i)
		{
			++_locoData->char_index;
			if(_locoData->char_index >= mcu.getNumCharacters())
			{
				_locoData->char_index = 0;
			}
			int counter = 0;
			for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
				iter != mcu.getCharacterMap().end();
				iter++)
			{
				if (counter != _locoData->char_index)
				{
					counter++;
					continue;
				}
				if (_paLocoData->character)
					_paLocoData->character->unregisterObserver(this);
				_paLocoData->character = actor;
				if (_paLocoData->character)
					_paLocoData->character->registerObserver(this);
				break;
			}

		}

		//_data->showselection = !_data->showselection;
		// check the widget
		/*int numChildren = _data->menubut->children();
		for (int c = 0; c < numChildren; c++)
		{
			Widget* wchild = _data->menubut->child(c);
			if (strcmp(wchild->label(), "&locomotion") == 0)
			{
				Group* group = dynamic_cast<Group*>(wchild);
				if (group)
				{
					int numGrandChildren = group->children();
					for (int g = 0; g < numGrandChildren; g++)
					{
						std::cout << group->label() << std::endl;
						Widget* grandChild = group->child(g);
						if (strcmp(grandChild->label(), "&show selection") == 0)
						{
							int vals[1] = { g };
							
							grandChild->click_to_focus();
							grandChild->set_selected();
							grandChild->redraw();
							_data->menubut->set_item(vals, 1);
							fltk::ToggleItem* item = dynamic_cast<fltk::ToggleItem*>(grandChild);
							int y = 0;
							if (item)
							{
								
							}
						}
						
					}
				}
				std::cout << group->label() << std::endl;
				
			}
			
		}*/

	}

	if(Fl::event_key('w'))
	{
		if(_locoData->z_flag != 0) _locoData->z_spd += 10;
		else if(_locoData->x_flag != 0) _locoData->x_spd += 1;
	}
	if(Fl::event_key('s'))
	{
		if(_locoData->z_flag != 0) _locoData->z_spd -= 10;
		else if(_locoData->x_flag != 0) _locoData->x_spd -= 1;
		if(_locoData->z_spd < 0) _locoData->z_spd = 0;
		if(_locoData->x_spd < 0) _locoData->x_spd = 0;
	}

	if(Fl::event_key('l'))
	{
		if(character->get_locomotion_ct()->is_freezed())
			strcat(cmd, "unfreeze");
		else strcat(cmd, "freeze");
		mcu.execute(cmd);
		return;
	}

	if(Fl::event_key('p'))
	{
		if(character->get_locomotion_ct()->is_freezed())
		{
			strcat(cmd, "nextframe");
			mcu.execute(cmd);
		}
		return;
	}

	//direction control
	PABlendData* blendData = NULL;
	if (_paLocoData->character && _paLocoData->character->param_animation_ct)
		if (_paLocoData->character->param_animation_ct)
			blendData = _paLocoData->character->param_animation_ct->getCurrentPABlendData();

#ifdef OLD_LOCOMOTION_CONTROL
	if(Fl::event_key(FL_Up))
	{
		if(!_locoData->upkey)
		{
			_locoData->rps_flag = 0;
			_locoData->z_flag = 1;
			_locoData->x_flag = 0;
			_locoData->spd = _locoData->z_spd;
			_locoData->kmode = 0;
			sprintf(_locoData->t_direction, "forward ");
			_locoData->upkey = true;
		}
		if (_paLocoData->character->locomotion_type == SbmCharacter::Example)
		{

			if (Fl::event_state(FL_ALT))
				_paLocoData->starting = true;
			if (!_paLocoData->starting)
				if (_paLocoData->v < -9990 && state)
					state->getParameter(_paLocoData->v, _paLocoData->w, scoot);
				else
					_paLocoData->v += _paLocoData->linearVelocityIncrement;
			paLocomotionCmd = true;
		}
	}
	else
	{
		_locoData->upkey = false;
	}
	if(Fl::event_key(FL_Down))
	{
		if(!_locoData->downkey)
		{
			_locoData->z_flag = -1;
			_locoData->x_flag = 0;
			_locoData->rps_flag = 0;
			_locoData->spd = _locoData->z_spd;
			_locoData->kmode = 0;
			sprintf(_locoData->t_direction, "backward ");
			_locoData->downkey = true;
		}
		if (_paLocoData->character->locomotion_type == SbmCharacter::Example)
		{
			if (Fl::event_state(FL_ALT))
				_paLocoData->stopping = true;
			if (!_paLocoData->stopping)
				if (_paLocoData->v < -9990 && state)
					state->getParameter(_paLocoData->v, _paLocoData->w, scoot);
				else
					_paLocoData->v -= _paLocoData->linearVelocityIncrement;
			paLocomotionCmd = true;
		}
	}
	else
	{
		_locoData->downkey = false;
	}
	if(Fl::event_key(FL_Left))
	{
		if(!_locoData->leftkey)
		{
			_locoData->rps_flag = -1;
			_locoData->leftkey = true;
		}
		if (_paLocoData->character->locomotion_type == SbmCharacter::Example)
		{
			if (_paLocoData->w < -9990 && state)
				state->getParameter(_paLocoData->v, _paLocoData->w, scoot);
			else
				_paLocoData->w += _paLocoData->angularVelocityIncrement;
			paLocomotionCmd = true;
		}
	}
	else
	{
		_locoData->leftkey = false;
	}
	if(Fl::event_key(FL_Right))
	{
		if(!_locoData->rightkey)
		{
			_locoData->rps_flag = 1;
			_locoData->rightkey = true;
		}
		if (_paLocoData->character->locomotion_type == SbmCharacter::Example)
		{
			if (_paLocoData->w < -9990 && state)
				state->getParameter(_paLocoData->v, _paLocoData->w, scoot);
			else
				_paLocoData->w -= _paLocoData->angularVelocityIncrement;
			paLocomotionCmd = true;
		}
	}
	else
	{
		_locoData->rightkey = false;
	}
	if (Fl::event_key(' '))
	{
		_paLocoData->jumping = true;
		_paLocoData->starting = false;
		_paLocoData->stopping = false;
		paLocomotionCmd = true;
	}

	if(Fl::event_key('a'))//speed control
	{
		if(!_locoData->a_key)
		{
			_locoData->x_flag = 1;
			_locoData->z_flag = 0;
			_locoData->rps_flag = 0;
			_locoData->spd = _locoData->x_spd;
			sprintf(_locoData->t_direction, "leftward ");
			_locoData->a_key = true;
		}
	}
	else
	{
		_locoData->a_key = false;
	}

	if(Fl::event_key('d'))//speed control
	{
		if(!_locoData->d_key)
		{
			_locoData->x_flag = -1;
			_locoData->z_flag = 0;
			_locoData->rps_flag = 0;
			_locoData->spd = _locoData->x_spd;
			sprintf(_locoData->t_direction, "rightward ");
			_locoData->d_key = true;
		}
	}
	else
	{
		_locoData->d_key = false;
	}

	if(!_locoData->rightkey && !_locoData->leftkey)
	{
		_locoData->rps_flag = 0;
	}

		if(_locoData->upkey
		|| _locoData->downkey
		|| _locoData->rightkey
		|| _locoData->leftkey
		|| _locoData->a_key
		|| _locoData->d_key)
	{
		locomotion_cmd = true;
	}

	char tt[200];
	
	strcat(cmd, _locoData->t_direction);
	//sprintf(tt, "spd %f rps %f time 0.5", spd, rps_flag * rps);

	if(_locoData->kmode == 0) sprintf(tt, "spd 0 rps %f time 0.7", _locoData->rps_flag * _locoData->rps);
	else sprintf(tt, "spd 0 lrps %f angle 3.14159265 time 1.0", _locoData->rps_flag * _locoData->rps);

	if (_paLocoData->character && _paLocoData->character->locomotion_type == SbmCharacter::Example)
 		locomotion_cmd = false;
	if(locomotion_cmd) 
	{
		strcat(cmd, tt);
		//printf("\n%s", cmd);
		mcu.execute(cmd);
	}
	if (paLocomotionCmd)
	{
		// p.s. quite confused about the logic here: _paLocoData->starting && _paLocoData->prevStarting? why this works?
		//if (_paLocoData->starting && _paLocoData->prevStarting && state && state->stateName == PseudoIdleState)
		if (_paLocoData->starting && state && state->stateName == PseudoIdleState)
		{
			std::stringstream command1;			
			command1 << "panim schedule char " << _paLocoData->character->getName() << " state allLocomotion loop true playnow false additive false joint null";
			//std::stringstream command2;
			//command2 << "panim schedule char " << _paLocoData->character->getName() << " state UtahLocomotion loop true playnow false additive false joint null";
			mcu.execute((char*)command1.str().c_str());
			//mcu.execute((char*)command2.str().c_str());			
			_paLocoData->starting = false;
		}
		else if (_paLocoData->stopping && _paLocoData->prevStopping && state && state->stateName == "allLocomotion")
		{
			std::stringstream command;
			command << "panim schedule char " << _paLocoData->character->getName() << " state UtahWalkToStop loop false playnow false additive false joint null ";
			mcu.execute((char*)command.str().c_str());
			_paLocoData->stopping = false;
		}
		else if (_paLocoData->jumping && _paLocoData->prevJumping && state && state->stateName == "allLocomotion")
		{
			std::stringstream command1;
			command1 << "panim schedule char " << _paLocoData->character->getName() << " state UtahJump loop false playnow false additive false joint null ";
			mcu.execute((char*)command1.str().c_str());	
			std::stringstream command2;
			command2 << "panim schedule char " << _paLocoData->character->getName() << " state UtahLocomotion loop true playnow false additive false joint null ";
			for (int i = 0; i < state->getNumMotions(); i++)
				command2 << state->weights[i] << " ";
			mcu.execute((char*)command2.str().c_str());
 			_paLocoData->jumping = false;
		}
		else
		{
			if (state)
			{
				if (_paLocoData->v < -9990 || _paLocoData->w < -9990)
					state->getParameter(_paLocoData->v, _paLocoData->w, scoot);
				bool success = state->setWeight(_paLocoData->v, _paLocoData->w, scoot);

				// in case scoot value comes as non-zero
				float x = 0.0f, y = 0.0f, z = 0.0f;
				state->getParameter(x, y, z);

				if (!success || z != 0.0f)
				{
					_paLocoData->v = prevV;
					_paLocoData->w = prevW;
					state->setWeight(_paLocoData->v, _paLocoData->w, scoot);
				}
				_paLocoData->character->param_animation_ct->updateWeights();
			}
		}
	}
#else
	
	static std::map<int,int> keyMap;
	if (keyMap.size() == 0)
	{
		keyMap['w'] = PALocomotionData::KEY_UP;
		keyMap['s'] = PALocomotionData::KEY_DOWN;
		keyMap['a'] = PALocomotionData::KEY_LEFT;
 		keyMap['d'] = PALocomotionData::KEY_RIGHT;
		keyMap['q'] = PALocomotionData::KEY_TURNLEFT;
		keyMap['e'] = PALocomotionData::KEY_TURNRIGHT;
	}

	std::map<int,int>::iterator mi;
	for ( mi  = keyMap.begin();
		  mi != keyMap.end();
		  mi++)
	{
		if (Fl::event_key(mi->first))
		{
			_paLocoData->pressKey(mi->second);
		}
		else
		{
			_paLocoData->releaseKey(mi->second);
		}
	}	
#endif
}


/*
static void translate_keyboard_event ( SrEvent& e, SrEvent::Type t, int w, int h)
{
	e.type = t;
	bool not_locomotion = false;
	e.key = Fl::event_key();
	char cmd[300];
	cmd[0] = '\0';
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if(_locoData->x_flag == 0 && _locoData->z_flag == 0)
	{
		_locoData->rps_flag = 0;
		_locoData->z_flag = 1;
		_locoData->x_flag = 0;
		_locoData->spd = _locoData->z_spd;
		sprintf(_locoData->t_direction, "forward ");
	}

	SbmCharacter* actor = NULL;
	mcu.character_map.reset();
	for(int i = 0; i <= _locoData->char_index; ++i)
	{
		actor = mcu.character_map.next();
		sprintf(_locoData->character, "char %s ", actor->name);
	}

	sprintf(cmd, "test loco ");

	// locomotion control
	switch (e.key)
	{
	case FL_UP: //move forward
		_locoData->rps_flag = 0;
		_locoData->z_flag = 1;
		_locoData->x_flag = 0;
		_locoData->spd = _locoData->z_spd;
		sprintf(_locoData->t_direction, "forward ");
		break;

    case FL_DOWN://move back
		_locoData->z_flag = -1;
		_locoData->x_flag = 0;
		_locoData->rps_flag = 0;
		_locoData->spd = _locoData->z_spd;
		sprintf(_locoData->t_direction, "backward ");
		break;

	case FL_Left://turn left
		_locoData->rps_flag = -1;
		break;

	case 'x':
		++_locoData->char_index;
		if(_locoData->char_index >= mcu.character_map.get_num_entries())
		{
			_locoData->char_index = 0;
		}
		not_locomotion = true;
		break;

	case FL_Right://turn right
		rps_flag = 1;
		break;

	case 'w'://speed control
		if(z_flag != 0) z_spd += 10;
		else if(x_flag != 0) x_spd += 1;
		not_locomotion = true;
		break;

	case 's'://speed control
		if(z_flag != 0) z_spd -= 10;
		else if(x_flag != 0) x_spd -= 1;
		if(z_spd < 0) z_spd = 0;
		if(x_spd < 0) x_spd = 0;
		not_locomotion = true;
		break;

	case 'a'://speed control
		x_flag = 1;
		z_flag = 0;
		rps_flag = 0;
		spd = x_spd;
		sprintf(t_direction, "leftward ");
		break;

	case 'd'://speed control
		x_flag = -1;
		z_flag = 0;
		rps_flag = 0;
		spd = x_spd;
		sprintf(t_direction, "rightward ");
		break;

	case ' ':// stop
		sprintf(cmd, "test loco stop");
		break;
	default:
		not_locomotion = true;
		break;
	}
	char tt[200];
	strcat(cmd, character);
	strcat(cmd, t_direction);
	//sprintf(tt, "spd %f rps %f time 0.5", spd, rps_flag * rps);

	sprintf(tt, "spd 0 rps %f time 0.5", rps_flag * rps);
	if(not_locomotion == false) 
	{
		strcat(cmd, tt);
		//printf("\n%s", cmd);
		mcu.execute(cmd);
	}
}
*/


void FltkViewer::processDragAndDrop( std::string dndMsg, float x, float y )
{
	static int characterCount = 0;
	static int pawnCount = 0;
	//LOG("dndMsg = %s",dndMsg.c_str());
	std::vector<std::string> toks;
	vhcl::Tokenize(dndMsg,toks,":");
	//if (toks.size() != 2)
	//	return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBScene* scene = mcu._scene;
	char cmdStr[256];
	SrVec p1;
	SrVec p2;
	_data->camera.get_ray(x,y, p1, p2);
	SrPlane ground(SrVec(0,0,0), SrVec(0, 1, 0));
	SrVec dest = ground.intersect(p1, p2);
	//dest.y = _paLocoData->character->getHeight() / 100.0f;			
	if (toks[0] == "SKELETON")
	{
		//dest.y = 102;		
		std::string skelName = toks[1].c_str();
		SBSkeleton* skel = mcu._scene->getSkeleton(skelName);
		if (skel)
		{
			sprintf(cmdStr,"char defaultChar%d init %s",characterCount,toks[1].c_str());
			mcu.execute(cmdStr);
			float yOffset = -skel->getBoundingBox().a.y;
			dest.y = yOffset;
			sprintf(cmdStr,"set char defaultChar%d world_offset x %f y %f z %f",characterCount,dest.x,dest.y,dest.z);
			//LOG("setCmd = %s",cmdStr);
			mcu.execute(cmdStr);
			characterCount++;
		}
		else
		{
			LOG("Error : Drag and drop,  skeleton %s not found",skelName.c_str());
		}		
	}	
	else if (toks[0] == "PAWN")
	{
		dest.y = 10;
		sprintf(cmdStr,"pawn defaultPawn%d init",pawnCount);
		mcu.execute(cmdStr);
		sprintf(cmdStr,"set pawn defaultPawn%d world_offset x %f y %f z %f",pawnCount,dest.x,dest.y,dest.z);
		mcu.execute(cmdStr);
		pawnCount++;
	}
	else // drag a file from explorer
	{	
		boost::filesystem::path dndPath(dndMsg);
		std::string fullPathName = dndMsg;
		std::string filebasename = boost::filesystem::basename(dndMsg);
		std::string fileextension = boost::filesystem::extension(dndMsg);	
		
		std::string skelName = filebasename+fileextension;
		std::string meshName = filebasename;
		std::string fullPath = dndPath.parent_path().string();

		LOG("path name = %s, base name = %s, extension = %s",fullPath.c_str(), filebasename.c_str(), fileextension.c_str());
		bool hasMesh = false;
		bool hasSkeleton = false;
		// copy the file over
		std::string meshDir = "../../../../data/retarget/mesh/";
		std::string retargetDir = "../../../../data/retarget/";

		// create the folder if they do not exist
		if (!boost::filesystem::exists(retargetDir))
			boost::filesystem::create_directory(retargetDir);
		if (!boost::filesystem::exists(meshDir))
			boost::filesystem::create_directory(meshDir);		
		boost::filesystem::create_directory(meshDir+meshName);

		std::string targetMeshFile = meshDir+meshName+"/"+filebasename+fileextension;
		std::string targetSkelFile = retargetDir+"/"+filebasename+fileextension;
		
		
		if (!boost::filesystem::exists(targetMeshFile))
			boost::filesystem::copy_file(fullPathName,targetMeshFile);
		if (!boost::filesystem::exists(targetSkelFile))
			boost::filesystem::copy_file(fullPathName,targetSkelFile);

		boost::filesystem::directory_iterator dirIter(dndPath.parent_path());
		boost::filesystem::directory_iterator endIter;
		cout << " dirIter initial : " << dirIter->path().string() << "\n";
		for ( ;
			  dirIter != endIter;
			  ++dirIter )
		{
			cout << " dirIter : " << dirIter->path().string() << "\n";
			if ( boost::filesystem::is_regular_file( *dirIter ) )
			{
				std::string filename = dirIter->path().string();
				cout << filename << " : " << boost::filesystem::file_size( dirIter->path() ) << "\n";
				std::string basename = boost::filesystem::basename(filename);
				std::string extname = boost::filesystem::extension(filename);
				if ( boost::iequals(extname,".jpg") || boost::iequals(extname,".bmp") || boost::iequals(extname,".png") 
					|| boost::iequals(extname,".dds") || boost::iequals(extname,".tga"))
				{
					// copy over the file if it is an image file
					std::string targetImgFile = meshDir+meshName+"/"+basename+extname;
					if (!boost::filesystem::exists(targetImgFile))
						boost::filesystem::copy_file(filename,targetImgFile);
				}
			}
		}
		

		
		// load the new skeleton
 		mcu.load_skeletons(retargetDir.c_str(), false);
 		//boost::filesystem::copy_file()
 		SBSkeleton* skel = mcu._scene->getSkeleton(skelName);
 		float yOffset = -skel->getBoundingBox().a.y;
 		dest.y = yOffset;		
		sprintf(cmdStr,"createDragAndDropCharacter('defaultChar%d','%s','%s',SrVec(%f,%f,%f))",characterCount,skelName.c_str(),meshName.c_str(),
			    dest.x,dest.y,dest.z);	
		LOG("pythonCmd = %s",cmdStr);
		mcu.executePythonFile("drag-and-drop.py");
		characterCount++;		
		mcu.executePython(cmdStr);
		
	}
}


int FltkViewer::handle ( int event ) 
 {
   # define POPUP_MENU(e) e.ctrl && e.button3

   SrEvent &e = _data->event;
   e.type = SrEvent::EventNone;
  
   translate_keyboard_state();   
   int ret = 0;
   std::string dndText;
   static float dndX,dndY;
   switch ( event )
   {   
	   case FL_DND_RELEASE:
		   //LOG("DND Release");
	       ret = 1;
	       break;
	   case FL_DND_ENTER:          // return(1) for these events to 'accept' dnd
		   //LOG("DND Enter");
		   Fl::belowmouse(this); // send the leave events first
		   Fl::focus(this);
		   handle(FL_FOCUS);		
		   ret = 1;
		   break;
	   case FL_DND_DRAG:
		   //LOG("DND Drag");
		   translate_event ( e, SrEvent::EventPush, w(), h(), this );
		   dndX = e.mouse.x;
		   dndY = e.mouse.y;
		   ret = 1;
		   break;
	   case FL_DND_LEAVE:
		   //LOG("DND Leave");
		   ret = 1;
		   break;	  
	   case FL_PASTE:              // handle actual drop (paste) operation		   
		   label(Fl::event_text());
		   //fprintf(stderr, "PASTE: %s\n", Fl::event_text());
		   LOG("PASTE: %s\n", Fl::event_text());
		   dndText = Fl::event_text();
		   processDragAndDrop(dndText,dndX,dndY);
		   ret = 1;
		   break;		
       case FL_PUSH:
       { //SR_TRACE1 ( "Mouse Push : but="<<Fl::event_button()<<" ("<<Fl::event_x()<<", "<<Fl::event_y()<<")" <<" Ctrl:"<<Fl::event_state(FL_CTRL) );
         translate_event ( e, SrEvent::EventPush, w(), h(), this );
         if ( POPUP_MENU(e) ) { show_menu(); e.type=SrEvent::EventNone; }
		 // process picking
		 //printf("Mouse Push\n");

		 //char exe_cmd[256];
		 if (e.button1)
		 {
			 /*
			 if (Fl::event_clicks())
			 {
				 // pick-up object
				 makeGLContext();
				 std::vector<int> hitList;
				 SbmCharacter* curChar = getCurrentCharacter();		
				 SbmPawn* selectedPawn = _objManipulator.getPickingPawn(e.mouse.x, e.mouse.y, _data->camera, hitList);
				 if (selectedPawn && curChar)
				 {
					 //std::string cmd;
					 //cmd = "bml char " + curChar->name + " <sbm:reach sbm:handle=\"r" + curChar->name + "\" action=\"pick-up\" target=\""+ selectedPawn->name + "\" />";
					 //sprintf(exe_cmd,"bml char %s <sbm:reach sbm:handle=\"r%s\" sbm:reach-duration=\"0.01\" sbm:action=\"pick-up\" target=\"%s\"/>",curChar->name,curChar->name,selectedPawn->name);
					 sprintf(exe_cmd,"bml char %s <sbm:reach sbm:reach-duration=\"-1.0\" sbm:action=\"touch\" target=\"%s\"/>",curChar->getName().c_str(),selectedPawn->getName().c_str());

					 mcuCBHandle& mcu = mcuCBHandle::singleton();
					 mcu.execute(exe_cmd);
				 }
			 }
			 else
			 */
			 {				 			 
				 makeGLContext();
				 _objManipulator.picking(e.mouse.x, e.mouse.y, _data->camera);
				 SbmPawn* selectedPawn = _objManipulator.get_selected_pawn();
				 if (selectedPawn)
				 {
					 SbmCharacter* isCharacter = dynamic_cast<SbmCharacter*> (selectedPawn);
					 if (isCharacter)
					 {
						 _lastSelectedCharacter = isCharacter->getName();
						 if (_paLocoData->character)
							 _paLocoData->character->unregisterObserver(this);
						 _paLocoData->character = isCharacter;
						 _paLocoData->character->registerObserver(this);
					 }
				 }
			 }			 
		 }

		 if (e.button3 && Fl::event_clicks() && e.alt)
		 {
			 // put-down object
			 /*
			 SbmCharacter* curChar = getCurrentCharacter();			 
			 SrVec p1;
			 SrVec p2;
			 
			 if (curChar)
			 {
				 _data->camera.get_ray(e.mouse.x, e.mouse.y, p1, p2);
				 SrPlane ground(SrVec(0,curChar->getHeight()*0.0f,0), SrVec(0, 1, 0));
				 SrVec dest = ground.intersect(p1, p2);
				 dest.y = curChar->getHeight()*0.6f;
				 sprintf(exe_cmd,"bml char %s <sbm:reach sbm:handle=\"r%s\" sbm:action=\"put-down\" sbm:target-pos=\"%f %f %f\"/>",curChar->getName().c_str(),curChar->getName().c_str(),dest.x,dest.y,dest.z);
				 mcuCBHandle& mcu = mcuCBHandle::singleton();
				 mcu.execute(exe_cmd);	 
			 }
			 */
			
		 }
		 else if (mcuCBHandle::singleton()._scene->getSteerManager()->getEngineDriver()->isInitialized() && e.button3 && !e.alt)
		 {
			 if (_paLocoData->character)
			 {
				 if (_paLocoData->character->steeringAgent)
				 {
					_paLocoData->character->steeringAgent->setTargetAgent(NULL);
					SrVec p1;
					SrVec p2;
					_data->camera.get_ray(e.mouse.x, e.mouse.y, p1, p2);
					SrPlane ground(SrVec(0,0,0), SrVec(0, 1, 0));
					SrVec dest = ground.intersect(p1, p2);
					dest.y = _paLocoData->character->getHeight() / 100.0f;
					std::stringstream command;
					command << "steer move " << _paLocoData->character->getName() << " normal " << dest.x << " " << dest.y << " " << dest.z;
					mcuCBHandle::singleton().execute((char*)command.str().c_str());
				 }
			 }
		 }
       } break;

      case FL_RELEASE:
        //SR_TRACE1 ( "Mouse Release : ("<<Fl::event_x()<<", "<<Fl::event_y()<<") buts: "
         //            <<(Fl::event_state(FL_BUTTON1)?1:0)<<" "<<(Fl::event_state(FL_BUTTON2)?1:0) );
        translate_event ( e, SrEvent::EventRelease, w(), h(), this);
		// process picking
		//if (!e.button1)	
		//printf("Mouse Release\n");
		//LOG("Mouse release");
        break;

      case FL_MOVE:
        //SR_TRACE2 ( "Move buts: "<<(Fl::event_state(FL_BUTTON1)?1:0)<<" "<<(Fl::event_state(FL_BUTTON2)?1:0) );
        if ( !Fl::event_state(FL_BUTTON1) && !Fl::event_state(FL_BUTTON2) ) break;
        // otherwise, this is a drag: enter in the drag case.
        // not sure if this is a hack or a feature.
      case FL_DRAG:
        //SR_TRACE2 ( "Mouse Drag : ("<<Fl::event_x()<<", "<<Fl::event_y()<<") buts: "
        //             <<(Fl::event_state(FL_BUTTON1)?1:0)<<" "<<(Fl::event_state(FL_BUTTON2)?1:0) );
        translate_event ( e, SrEvent::EventDrag, w(), h(), this );
		
		
        break;

      case FL_SHORTCUT: // not sure the relationship between a shortcut and keyboard event...
        //SR_TRACE1 ( "Shortcut : "<< Fl::event_key() <<" "<<fltk::event_text() );
        //translate_event ( e, SrEvent::Keyboard, w(), h() );
        //break;

	  case FL_KEYBOARD:
        //SR_TRACE1 ( "Key Pressed : "<< Fl::event_key() <<" "<<fltk::event_text() );
        //translate_keyboard_event ( e, SrEvent::Keyboard, w(), h());
        break;

//      case FL_KEYBOARDBOARD:
        //SR_TRACE1 ( "Key Pressed : "<< Fl::event_key() <<" "<<fltk::event_text() );
//        translate_event ( e, SrEvent::Keyboard, w(), h() );
     //   break;

      case FL_HIDE: // Called when the window is iconized
        { //SR_TRACE1 ( "Hide" );
          _data->iconized = true;
          // the opengl lists need to be re-created when the window appears again, so
          // we mark already here all shapes as changed:
          _data->scenebox->changed(true);
          _data->sceneaxis->changed(true);
          srSaSetShapesChanged sa;
          sa.apply ( _data->root );
        } break;

      case FL_SHOW: // Called when the window is de-iconized or when show() is called
        //SR_TRACE1 ( "Show" );
        _data->iconized = false;
        show ();
        break;	 
      // Other events :
      case FL_ENTER:          
		  //SR_TRACE2 ( "Enter" );         
		  break;
      case FL_LEAVE:          
		  //SR_TRACE2 ( "Leave" );         
		  break;
      case FL_FOCUS:          
		  //SR_TRACE2 ( "Focus" );         
		  break;
      case FL_UNFOCUS:        
		  //SR_TRACE2 ( "Unfocus" );       
		  break;
     // case FL_CLOSE:          
		  //SR_TRACE2 ( "Close");          
	//	  break;
      case FL_ACTIVATE:       
		  //SR_TRACE2 ( "Activate");       
		  break;
      case FL_DEACTIVATE:     
		  //SR_TRACE2 ( "Deactivate");     
		  break;
	  //case FL_PASTE:          
		  //SR_TRACE2 ( "Paste");          
		 // break;
 //     case FL_SELECTIONCLEAR: 
		  //SR_TRACE2 ( "SelectionClear"); 
	//	  break;
    }

   //SR_TRACE3 ( e );

	if (ret == 1)  // a drag and drop event
	{	
		//LOG("ret == 1");
		return ret;
	}

   if ( e.type == SrEvent::EventNone ) return 0; // not an interesting event

   if ( event==FL_PUSH || event==FL_DRAG )
    { SrPlane plane ( _data->camera.center, SrVec::k );
      _data->camera.get_ray ( e.mouse.x, e.mouse.y, e.ray.p1, e.ray.p2 );
      _data->camera.get_ray ( e.lmouse.x, e.lmouse.y, e.lray.p1, e.lray.p2 );
      e.mousep = plane.intersect ( e.ray.p1, e.ray.p2 );
      e.lmousep = plane.intersect ( e.lray.p1, e.lray.p2 );
	  if ( event==FL_PUSH  ) // update picking precision
       { // define a and b with 1 pixel difference:
         SrPnt2 a ( ((float)w())/2.0f, ((float)h())/2.0f ); // ( float(Fl::event_x()), float(Fl::event_y()) );
         SrPnt2 b (a+SrVec2::one);// ( float(Fl::event_x()+1), float(Fl::event_y()+1) );
         // put coordinates inside [-1,1] with (0,0) in the middle :
         a.x  = a.x*2.0f / float(w()) - 1.0f;
         a.y  = a.y*2.0f / float(h()) - 1.0f; a.y *= -1.0f;
         b.x  = b.x*2.0f / float(w()) - 1.0f;
         b.y  = b.y*2.0f / float(h()) - 1.0f; b.y *= -1.0f;
         //sr_out << "a,b: " << a << srspc << b <<srnl;
         SrLine aray, bray;
         _data->camera.get_ray ( a.x, a.y, aray.p1, aray.p2 );
         _data->camera.get_ray ( b.x, b.y, bray.p1, bray.p2 );
         SrPnt pa = plane.intersect ( aray.p1, aray.p2 );
         SrPnt pb = plane.intersect ( bray.p1, bray.p2 );
         //sr_out << "pa,pb: " << pa << srspc << pb <<srnl;
         e.pixel_size = (SR_DIST(pa.x,pb.x)+SR_DIST(pa.y,pb.y))/2.0f;
		 interactivePoint = e.lmousep;
         //sr_out << "pixel_size: " << e.pixel_size <<srnl;
       }
    }

   
    return handle_event ( e );
}

//== handle sr event =======================================================

int FltkViewer::handle_event ( const SrEvent &e )
 {
   int res=0;   

   if ( e.alt && e.mouse_event() )
    { 
        res = handle_examiner_manipulation ( e );

      if ( res ) return res;
    }
   
   if (e.ctrl && e.mouse_event() )
   {
	   res = handle_object_manipulation ( e );
	   if ( res ) return res;
   }

   if (e.shift && e.mouse_event() )
   {
	   res = handle_object_manipulation ( e );
	   if ( res ) return res;
   }

   if ( e.mouse_event() ) return handle_scene_event ( e );


   return res; // this point should not be reached
 }

//== Object Manipulation event =======================================================

int FltkViewer::handle_object_manipulation( const SrEvent& e)
{
	if (e.type==SrEvent::EventPush)
	 {
		 if (e.button1)
		 {
			 //_objManipulator.picking(e.mouse.x,e.mouse.y, _data->camera);
			 //_objManipulator.hasPicking(true);
			SrVec2 mouseVec(e.mouse.x, e.mouse.y);
			 _objManipulator.setPicking(mouseVec);
			 if (e.ctrl)
				 _objManipulator.setPickingType(ObjectManipulationHandle::CONTROL_POS);
			 else if (e.shift)
				 _objManipulator.setPickingType(ObjectManipulationHandle::CONTROL_ROT);
			 /*
			 // unify the pawn selection and the locomotion selection
			 SbmPawn* pawn = _objManipulator.get_selected_pawn();
			 SbmCharacter* character = dynamic_cast<SbmCharacter*>(pawn);
			 if (character)
			 {
				 mcuCBHandle& mcu = mcuCBHandle::singleton();
				 int index = 0;
				 mcu.character_map.reset();
				 while (SbmCharacter* c = mcu.character_map.next())
				 {
					 if (character == c)
					 {
						 _locoData->char_index = index;
						 break;
					 }
					 index++;
				 }	
			 }
			 */
		 }
		 if (e.button3 && e.shift)
		 {
			SrVec2 pickVec(e.mouse.x, e.mouse.y);
			_objManipulator.setPicking(pickVec);
			_objManipulator.picking(e.mouse.x, e.mouse.y, _data->camera);
			SbmPawn* selectedPawn = _objManipulator.get_selected_pawn();
			if (selectedPawn)
				if (selectedPawn->getName() != _paLocoData->character->getName())
				{
					SbmCharacter* selectedCharacter = dynamic_cast<SbmCharacter*> (selectedPawn);
					if (selectedCharacter)
						_paLocoData->character->steeringAgent->setTargetAgent(selectedCharacter);
				}
		 }
		return 1;
	 }
	else if (e.type==SrEvent::EventDrag)
	{
		if (e.button1)// && _posControl.dragging)
		{			
			_objManipulator.drag(_data->camera,e.lmouse.x,e.lmouse.y,e.mouse.x,e.mouse.y);			
		}
	}
	else if (e.type==SrEvent::EventRelease)
	{
		if (e.button == 1)
		{			
			//_posControl.dragging = false;
		}
	}
	return 0;
}



void FltkViewer::create_pawn()
{
	const char* pawn_name = fl_input("Input Pawn Name","foo");
	if (!pawn_name) // no name is input
		return;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	char cmd_pawn[256];
	sprintf(cmd_pawn,"pawn %s init",pawn_name);
	mcu.execute(cmd_pawn);
}


void FltkViewer::set_reach_target( int itype, const char* targetname )
{
	char exe_cmd[256];
	SbmCharacter* actor = NULL;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	int counter = 0;
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		if (counter == _locoData->char_index)
		{
			actor = (*iter).second;
			break;
		}
		counter++;
	}	

	SbmPawn* pawn = _objManipulator.get_selected_pawn();
	static char reach_type[NUM_REACH_TYPES][20] = { "right", "left" };	
	if (actor)
	{
		char pawn_name[30];
		if (strcmp(targetname,"selected pawn")==0)
		{
			if (pawn)
				strcpy(pawn_name,pawn->getName().c_str());
			else
			{
				// handle user error : call set target command without selecting a pawn target.
			}
		}
		else
			strcpy(pawn_name,targetname);

		sprintf(exe_cmd,"bml char %s <sbm:reach target=\"%s\" reach-arm=\"%s\"/>",actor->getName().c_str(),pawn_name,reach_type[itype]);
		mcu.execute_later(exe_cmd,1.0); // delay execution for one second to avoid popping
	}
}

void FltkViewer::set_gaze_target(int itype, const char* label)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmPawn* pawn = this->getObjectManipulationHandle().get_selected_pawn();

	SbmCharacter* actor = this->getCurrentCharacter();
	if (!actor)
		return;

	if (itype == -1)
	{
		std::stringstream strstr;
		strstr << "char " << actor->getName() << "gazefade out 0";
		mcu.execute((char*) strstr.str().c_str());
		return;
	}
		
	static char gaze_type[NUM_GAZE_TYPES][20] = { "EYES", "EYES NECK", "EYES CHEST", "EYES BACK" };
	//if (actor)
	//	printf("current char %s ", actor->name);

	if (actor)
	{
		char pawn_name[30];
		if (strcmp(label,"selected pawn")==0)
		{
			if (pawn)
				strcpy(pawn_name, pawn->getName().c_str());
			else
			{
				// handle user error : call set target command without selecting a pawn target.
			}
		}
		else
			strcpy(pawn_name,label);

		std::stringstream strstr;
		strstr << "bml char " << actor->getName() << " <gaze target=\"" << pawn_name << "\" sbm:joint-range=\"" << gaze_type[itype] << "\"/>";
		mcu.execute((char*) strstr.str().c_str());
	}
}

//== Examiner ==============================================================

# define ROTATING2(e)    (e.alt && e.button1)
# define ROTATING(e)   (e.alt && e.shift && e.button1)
//# define ZOOMING(e)   (e.alt && e.button3)
# define ZOOMING(e)     (e.shift && e.ctrl && e.button3)
# define DOLLYING(e)     (e.alt && e.button3)
# define TRANSLATING(e) (e.alt && e.button2)

int FltkViewer::handle_examiner_manipulation ( const SrEvent &e )
 {
   if ( e.type==SrEvent::EventDrag )
    { 
      float dx = e.mousedx() * _data->camera.aspect;
      float dy = e.mousedy() / _data->camera.aspect;

		if ( ZOOMING(e) )
		{
			_data->camera.fovy += (dx+dy);//40.0f;
			_data->camera.fovy = SR_BOUND ( _data->camera.fovy, 0.001f, srpi );
			redraw();
		}
		else if ( DOLLYING(e) )
		{ 
			float amount = dx-dy;
			SrVec cameraPos(_data->camera.eye);
			SrVec targetPos(_data->camera.center);
			SrVec diff = targetPos - cameraPos;
			float distance = diff.len();
			diff.normalize();

			if (amount >= distance)
				amount = distance - .000001f;

			SrVec diffVector = diff;
			SrVec adjustment = diffVector * distance * amount;
			cameraPos += adjustment;
			SrVec oldEyePos = _data->camera.eye;
			_data->camera.eye = cameraPos;
			SrVec cameraDiff = _data->camera.eye - oldEyePos;
			_data->camera.center += cameraDiff;
			redraw();
		}
      else if ( TRANSLATING(e) )
       { _data->camera.apply_translation_from_mouse_motion ( e.lmouse.x, e.lmouse.y, e.mouse.x, e.mouse.y );
		redraw();
       }
      else if ( ROTATING(e) )
       { 
#if 1
		   float deltaX = -(e.mouseCoord.x - e.origMouse.x) / e.width;
		   float deltaY = -(e.mouseCoord.y -  e.origMouse.y) / e.height;
		   SrVec origUp = e.origUp;
		   SrVec origCenter = e.origCenter;
		   SrVec origCamera = e.origEye;
#else

		   float deltaX = -(e.mouse.x - e.lmouse.x) ;
		   float deltaY = (e.mouse.y -  e.lmouse.y) ;
		   SrVec origUp = _data->camera.up;
		   SrVec origCenter = _data->camera.center;
		   SrVec origCamera = _data->camera.eye;
#endif
		   if (deltaX == 0.0 && deltaY == 0.0)
			   return 1;

		   SrVec forward =origCenter - origCamera; 		   
		   SrQuat q = SrQuat(origUp, vhcl::DEG_TO_RAD()*deltaX*150.f);			   
		   forward = forward*q;
		   _data->camera.center = _data->camera.eye + forward;

		   SrVec cameraRight = cross(forward,origUp);
		   cameraRight.normalize();		   
		   q = SrQuat(cameraRight, vhcl::DEG_TO_RAD()*deltaY*150.f);	
		   _data->camera.up = origUp*q;
		   forward = forward*q;
		   _data->camera.center = _data->camera.eye + forward;		  
		   redraw();
       }
      else if ( ROTATING2(e) )
       { 
 		float deltaX = -(e.mouseCoord.x - e.origMouse.x) / e.width;
		float deltaY = -(e.mouseCoord.y -  e.origMouse.y) / e.height;
		if (deltaX == 0.0 && deltaY == 0.0)
			return 1;

		SrVec origUp = e.origUp;
		SrVec origCenter = e.origCenter;
		SrVec origCamera = e.origEye;

		SrVec dirX = origUp;
		SrVec  dirY;
		dirY.cross(origUp, (origCenter - origCamera));
		dirY /= dirY.len();

		SrVec camera = rotatePoint(origCamera, origCenter, dirX, -deltaX * float(M_PI));
		camera = rotatePoint(camera, origCenter, dirY, deltaY * float(M_PI));

		_data->camera.eye = camera;
		redraw();
	  }
    }
   else if ( e.type==SrEvent::EventRelease )
    { 
    }
   return 1;
 }

//== Apply Scene action ==========================================================

int FltkViewer::handle_scene_event ( const SrEvent& e )
 {
   SrSaEvent ea(e);
   ea.apply ( _data->root );
   int used = ea.result();
   if ( used ) render();
   return used;
 }

//== Keyboard ==============================================================

void FltkViewer::label_viewer(const char* str)
{
	label(str);
}

void FltkViewer::show_viewer()
{
	
	show();
}

void FltkViewer::hide_viewer()
{
	if (this->shown())
		this->hide();
}

void FltkViewer::initGridList()
{
	glDeleteLists(gridList, 1);
	gridList = glGenLists(1);
	if ( gridList == GL_INVALID_VALUE || gridList == GL_INVALID_OPERATION)
		return;
	glNewList(gridList, GL_COMPILE);
	drawGrid();
	glEndList();
}

void FltkViewer::drawGrid()
{
	if (_data->gridMode == ModeNoGrid)
		return;
//	if( gridList != -1 )	{
//		glCallList( gridList );
//		return;
//	}

	GLfloat floor_height = 0.0f;

	glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
	bool colorChanged = false;
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);

	glColor4f(gridColor[0], gridColor[1], gridColor[2], gridColor[3]);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glLineWidth(1);
//	glLineStipple(1, 0xAAAA);
	glBegin(GL_LINES);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	float sceneScale =  mcu._scene->getScale();
	float adjustedGridStep = gridStep;
	if (sceneScale > 0.f)
	{
		adjustedGridStep *= adjustedGridStep / .01f;
	}

	for (float x = -gridSize; x <= gridSize; x += adjustedGridStep)
	{
		if (x == 0.0) {
			colorChanged = true;
			glColor4f(gridHighlightColor[0], gridHighlightColor[1], gridHighlightColor[2], 1.0f);
		}
		glVertex3f(x, floor_height, -gridSize);
		glVertex3f(x, floor_height, gridSize);
		
		if (colorChanged) {
			colorChanged = false;
			glColor4f(gridColor[0], gridColor[1], gridColor[2], gridColor[3]);
		}

	}
	for (float x = -gridSize; x <= gridSize; x += gridStep)
	{
		if (x == 0) {
			colorChanged = true;
			glColor4f(gridHighlightColor[0], gridHighlightColor[1], gridHighlightColor[2], 1.0f );
		}
		glVertex3f(-gridSize, floor_height, x);
		glVertex3f(gridSize, floor_height, x);
		if (colorChanged) {
			colorChanged = false;
			glColor4f(gridColor[0], gridColor[1], gridColor[2], gridColor[3]);
		}
	}

	glEnd();
	glDisable(GL_BLEND);
//	glDisable(GL_LINE_STIPPLE);

	glPopAttrib();
}

void FltkViewer::drawEyeBeams()
{
	if (_data->eyeBeamMode == ModeNoEyeBeams)
		return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		character->getSkeleton()->invalidate_global_matrices();
		character->getSkeleton()->update_global_matrices();
		SkJoint* eyeRight = character->getSkeleton()->search_joint("eyeball_right");
		float eyebeamLength = 100 * character->getHeight() / 175.0f;
		if (eyeRight)
		{
			SrMat gmat = eyeRight->gmat();
			SrVec localAxis = eyeRight->localGlobalAxis(2)*eyebeamLength;
			glPushMatrix();
			glMultMatrixf((const float*) gmat);
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(localAxis[0],localAxis[1],localAxis[2]);
			glEnd();
			glPopMatrix();
		}
		SkJoint* eyeLeft = character->getSkeleton()->search_joint("eyeball_left");
		if (eyeLeft)
		{
			SrMat gmat = eyeLeft->gmat();
			SrVec localAxis = eyeLeft->localGlobalAxis(2)*eyebeamLength;
			glPushMatrix();
			glMultMatrixf((const float*) gmat);
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			//glVertex3f(0, 0, eyebeamLength);
			glVertex3f(localAxis[0],localAxis[1],localAxis[2]);
			glEnd();
			glPopMatrix();
		}
	}

}

void FltkViewer::drawEyeLids()
{
	if (_data->eyeLidMode == ModeNoEyeLids)
		return;

	glPushAttrib(GL_LIGHTING_BIT | GL_POINT_BIT);
	glDisable(GL_LIGHTING);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		MeControllerTreeRoot* controllerTree = character->ct_tree_p;
		int numControllers = controllerTree->count_controllers();
	
		MeCtEyeLid* eyelidCt = NULL;
		for (int c = 0; c < numControllers; c++)
		{
			MeController* controller = controllerTree->controller(c);
			eyelidCt = dynamic_cast<MeCtEyeLid*>(controller);
			if (eyelidCt)
				break;
		}
		if (!eyelidCt)
		{
			continue;
		}

		character->getSkeleton()->update_global_matrices();
		
		float upperHi;
		float upperLo;
		eyelidCt->get_upper_lid_range(upperLo, upperHi);

		float lowerHi;
		float lowerLo;
		eyelidCt->get_lower_lid_range(lowerLo, lowerHi);

		SkJoint* eyeLidUpperRight = character->getSkeleton()->search_joint("upper_eyelid_right");
		SkJoint* eyeLidUpperLeft = character->getSkeleton()->search_joint("upper_eyelid_left");
		SkJoint* eyeLidLowerRight = character->getSkeleton()->search_joint("lower_eyelid_right");
		SkJoint* eyeLidLowerLeft = character->getSkeleton()->search_joint("lower_eyelid_left");

		glPointSize(10);
		float range = character->getHeight() / 175.0f * 2.0f;
		if (eyeLidUpperRight)
		{
			const SkJoint* parent = eyeLidUpperRight->parent();
			if (parent)
			{
				SrMat gmat = parent->gmat();
				glPushMatrix();
				glMultMatrixf((const float*) gmat);
				glColor3f(1.0, 0.0, 0.0);
				SrVec offset = eyeLidUpperRight->offset();
				glBegin(GL_POINTS);
				glVertex3f(offset.x, offset.y, offset.z);
				glEnd();

				glTranslatef(offset.x, offset.y, offset.z);

				// add the up/down offsets
				glColor3f(1.0, 1.0, 0.0);
				glPushMatrix();
				glBegin(GL_LINE_LOOP);
				glVertex3f(-range, upperHi, 0);
				glVertex3f(range, upperHi, 0);
				glVertex3f(range, upperLo, 0);
				glVertex3f(-range, upperLo, 0);
				glEnd();
				glPopMatrix();

				glPopMatrix();
			}
		}
		if (eyeLidUpperLeft)
		{
			const SkJoint* parent = eyeLidUpperLeft->parent();
			if (parent)
			{
				SrMat gmat = parent->gmat();
				glPushMatrix();
				glMultMatrixf((const float*) gmat);
				glColor3f(1.0, 0.0, 0.0);
				SrVec offset = eyeLidUpperLeft->offset();
				glBegin(GL_POINTS);
				glVertex3f(offset.x, offset.y, offset.z);
				glEnd();

				glTranslatef(offset.x, offset.y, offset.z);

				// add the up/down offsets
				glColor3f(1.0, 1.0, 0.0);
				glPushMatrix();
				glBegin(GL_LINE_LOOP);
				glVertex3f(-range, upperHi, 0);
				glVertex3f(range, upperHi, 0);
				glVertex3f(range, upperLo, 0);
				glVertex3f(-range, upperLo, 0);
				glEnd();
				glPopMatrix();

				glPopMatrix();
			}
		}
		if (eyeLidLowerRight)
		{
			const SkJoint* parent = eyeLidLowerRight->parent();
			if (parent)
			{
				SrMat gmat = parent->gmat();
				glPushMatrix();
				glMultMatrixf((const float*) gmat);
				glColor3f(1.0, 0.0, 0.0);
				SrVec offset = eyeLidLowerRight->offset();
				glBegin(GL_POINTS);
				glVertex3f(offset.x, offset.y, offset.z);
				glEnd();

				glTranslatef(offset.x, offset.y, offset.z);

				// add the up/down offsets
				glColor3f(0.0, 1.0, 0.0);
				glPushMatrix();
				glBegin(GL_LINE_LOOP);
				glVertex3f(-range, lowerHi, 0);
				glVertex3f(range, lowerHi, 0);
				glVertex3f(range, lowerLo, 0);
				glVertex3f(-range, lowerLo, 0);
				glEnd();
				glPopMatrix();

				glPopMatrix();
			}
		}
		if (eyeLidLowerLeft)
		{
			const SkJoint* parent = eyeLidLowerLeft->parent();
			if (parent)
			{
				SrMat gmat = parent->gmat();
				glPushMatrix();
				glMultMatrixf((const float*) gmat);
				glColor3f(1.0, 0.0, 0.0);
				SrVec offset = eyeLidLowerLeft->offset();
				glBegin(GL_POINTS);
				glVertex3f(offset.x, offset.y, offset.z);
				glEnd();

				glTranslatef(offset.x, offset.y, offset.z);

				// add the up/down offsets
				glColor3f(0.0, 1.0, 0.0);
				glPushMatrix();
				glBegin(GL_LINE_LOOP);
				glVertex3f(-range, lowerHi, 0);
				glVertex3f(range, lowerHi, 0);
				glVertex3f(range, lowerLo, 0);
				glVertex3f(-range, lowerLo, 0);
				glEnd();
				glPopMatrix();

				glPopMatrix();
			}
		}
	}

	glPopAttrib();

}


void FltkViewer::drawCharacterBoundingVolumes()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		if (character && character->getGeomObject())
		{
			SrMat gmat = character->getGeomObject()->getGlobalTransform().gmat();
			this->drawColObject(character->getGeomObject(), gmat);
		}
	}
}

void FltkViewer::drawCharacterPhysicsObjs()
{
	float pawnSize = 1.0;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phyEngine = SbmPhysicsSim::getPhysicsEngine();
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		SbmPhysicsCharacter* phyChar = phyEngine->getPhysicsCharacter(character->getName());//character->getPhysicsCharacter();				
		if (!phyChar) 
		{			
			continue;
		}


		std::map<std::string,SbmJointObj*>& jointPhyObjs = phyChar->getJointObjMap();
		std::map<std::string,SbmJointObj*>::iterator mi;
		float totalMass = 0.f;
		for ( mi  = jointPhyObjs.begin();
			  mi != jointPhyObjs.end();
			  mi++)
		{
			SbmJointObj* obj = mi->second;
			totalMass += obj->getMass();
		}
		if (totalMass == 0.f) totalMass = 1.f;

		for ( mi  = jointPhyObjs.begin();
			  mi != jointPhyObjs.end();
			  mi++)
		{
			SbmJointObj* obj = mi->second;
			SrMat gmat = obj->getGlobalTransform().gmat();
			SBJoint* joint = obj->getSBJoint();	
			SbmPhysicsSim* physics = SbmPhysicsSim::getPhysicsEngine();
#if 1
			if (physics)
			{
				SrVec jointPos = physics->getJointConstraintPos(obj->getPhyJoint());
				SrSnSphere sphere;				
				sphere.shape().center = jointPos;//SrVec(0,-cap->extent,0);
				sphere.shape().radius = character->getHeight()*0.022f;
				float axisScale = character->getHeight()*0.01f;
				sphere.color(SrColor(1.f,0.f,0.f));
				glEnable(GL_LIGHTING);
				sphere.render_mode(srRenderModeSmooth);
				//SrGlRenderFuncs::render_sphere(&sphere);
				glDisable(GL_LIGHTING);		

				
				glBegin(GL_LINES);
				SrVec axis = physics->getJointRotationAxis(obj->getPhyJoint(),0)*axisScale;
				//axis = SrVec(gmat.get(0),gmat.get(1),gmat.get(2))*axisScale;
				glColor3f(1.f,0.f,0.f);
				glVertex3f(jointPos[0],jointPos[1],jointPos[2]);
				glVertex3f(jointPos[0]+axis[0],jointPos[1]+axis[1],jointPos[2]+axis[2]);

				glColor3f(0.f,1.f,0.f);
				axis = physics->getJointRotationAxis(obj->getPhyJoint(),1)*axisScale;
				//axis = SrVec(gmat.get(4),gmat.get(5),gmat.get(6))*axisScale;
				glVertex3f(jointPos[0],jointPos[1],jointPos[2]);
				glVertex3f(jointPos[0]+axis[0],jointPos[1]+axis[1],jointPos[2]+axis[2]);

				glColor3f(0.f,0.f,1.f);				
				axis = physics->getJointRotationAxis(obj->getPhyJoint(),2)*axisScale;
				//axis = SrVec(gmat.get(8),gmat.get(9),gmat.get(10))*axisScale;
				glVertex3f(jointPos[0],jointPos[1],jointPos[2]);
				glVertex3f(jointPos[0]+axis[0],jointPos[1]+axis[1],jointPos[2]+axis[2]);
				glEnd();


				// draw torque
// 				glBegin(GL_LINES);
// 				glColor3f(1.f,0.f,1.f);								
// 				axis = obj->getPhyJoint()->getJointTorque()*1.f;
// 				glVertex3f(jointPos[0],jointPos[1],jointPos[2]);
// 				glVertex3f(jointPos[0]+axis[0],jointPos[1]+axis[1],jointPos[2]+axis[2]);
// 				glEnd();				
			}
#endif

			if (_data->showmasses)
			{
				glPushAttrib(GL_LIGHTING_BIT);
				glEnable(GL_LIGHTING);
				glColor3f(1.0f, 1.0f, 0.0f);
				SrSnSphere sphere;
				float height = 200.0;
				float mass = obj->getMass();
				if (mass > 0)
				{
					float proportion = mass/totalMass;
					// draw a sphere of proportionate size to entire character to show mass distribution
					SrMat gmat = obj->getGlobalTransform().gmat();					
					sphere.shape().center = SrPnt(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
					sphere.shape().radius = proportion * character->getHeight();
					sphere.color(SrColor(0.f,1.f,1.f));
					SrGlRenderFuncs::render_sphere(&sphere);					
				}
				glPopAttrib();				
			}
			this->drawColObject(obj->getColObj(), gmat);
		}		

	}
}

void FltkViewer::drawPawns()
{
	if (_data->pawnmode == ModeNoPawns)
		return;


	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// determine the size of the pawns relative to the size of the characters
	float pawnSize = 1.0;
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		pawnSize = character->getHeight()/ 30.0f;
		break;
	}

	for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		if (!pawn->getSkeleton()) 
			continue;
		SbmCharacter* character = dynamic_cast<SbmCharacter*>(pawn);
		if (character)
			continue;

		const bool isVisible = pawn->getBoolAttribute("visible");
		if (!isVisible)
			continue;

		SrMat gmat = pawn->get_world_offset();//pawn->get_world_offset_joint()->gmat();		
		if (pawn->getGeomObject() && dynamic_cast<SbmGeomNullObject*>(pawn->getGeomObject()) == NULL)
		{
			//pawn->colObj_p->updateTransform(gmat);
			//gmat = pawn->colObj_p->worldState.gmat();
			//if (pawn->getPhysicsObject())
			//{
			//	gmat = pawn->getPhysicsObject()->getGlobalTransform().gmat();
			//}
			//SrMat gmatPhy = pawn->getPhysicsObject()->getGlobalTransform().gmat();
			drawColObject(pawn->getGeomObject(),gmat);
		}
		else
		{
			// draw default sphere
			glPushAttrib(GL_LIGHTING_BIT);
			glDisable(GL_LIGHTING);
			glPushMatrix();
			glMultMatrixf((const float*) gmat);
			glColor3f(1.0, 0.0, 0.0);
			SrSnSphere sphere;
			glPushMatrix();
			sphere.shape().center = SrPnt(0, 0, 0);
			sphere.shape().radius = pawnSize;
			sphere.render_mode(srRenderModeLines);
			SrGlRenderFuncs::render_sphere(&sphere);
			//glEnd();
			glPopMatrix();
			glPopMatrix();
			glPopAttrib();
		}		
	}
}


void FltkViewer::drawPoint( float cx, float cy, float cz, float size, SrVec& color )
{
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND); 
	glColor4f(color.x, color.y, color.z, 1.0f);
	glPointSize(5.0);
	glBegin(GL_POINTS); 
	glVertex3f(cx, cy, cz);//output vertex 
	glEnd(); 
	glDisable(GL_BLEND); 
}

void FltkViewer::drawTetra( SrVec vtxPos[4], SrVec& color )
{
	// draw a wireframe tetrahedron
	static int edgeIdx[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };
	glColor4f(color.x,color.y,color.z,1.f);
	glBegin(GL_LINES);
	for (int i=0;i<6;i++)
	{
		SrVec& p1 = vtxPos[edgeIdx[i][0]];
		SrVec& p2 = vtxPos[edgeIdx[i][1]];
		glVertex3f(p1.x,p1.y,p1.z);
		glVertex3f(p2.x,p2.y,p2.z);
	}
	glEnd();	
}


void FltkViewer::drawCircle(float cx, float cy, float cz, float r, int num_segments, SrVec& color) 
{ 
	float theta = 2.0f * 3.1415926f / float(num_segments); 
	float tangetial_factor = tanf(theta);//calculate the tangential factor 

	float radial_factor = cosf(theta);//calculate the radial factor 
	
	float x = r;//we start at angle = 0 

	float z = 0; 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND); 
	glColor4f(color.x, color.y, color.z, 0.3f);
	glBegin(GL_LINE_LOOP); 
	for(int ii = 0; ii < num_segments; ii++) 
	{ 
		glVertex3f(x + cx, cy, z + cz);//output vertex 
        
		float tx = -z; 
		float tz = x; 

		x += tx * tangetial_factor; 
		z += tz * tangetial_factor; 

		x *= radial_factor; 
		z *= radial_factor; 
	} 
	
	glEnd(); 
	glDisable(GL_BLEND); 
}
static float spin_angle = 0.0f;
void FltkViewer::drawActiveArrow(SrVec& from, SrVec& to, int num, float width, SrVec& color, bool spin)
{
	spin_angle += 0.02f;
	if(spin_angle >= 3.1415926535f*2.0f) spin_angle = 0.0f;
	SrVec di = (to - from)/(float)num;


	float speed = 40.0f;
	float acc = -80.0f;
	float latency = 0.10f;
	
	_arrowTime += 0.01666f;

	SrVec p[6];
	p[5] = to;
	SrMat mat;
	mat.rot(di, spin_angle);

	p[1].x = width/2.0f;
	p[1].z = width/2.0f;
	p[2].x = width/2.0f;
	p[2].z = -width/2.0f;
	p[3].x = -width/2.0f;
	p[3].z = -width/2.0f;
	p[4].x = -width/2.0f;
	p[4].z = width/2.0f;

	p[1] = p[1]*mat;
	p[2] = p[2]*mat;
	p[3] = p[3]*mat;
	p[4] = p[4]*mat;
	p[1] = p[5]+p[1];
	p[2] = p[5]+p[2];
	p[3] = p[5]+p[3];
	p[4] = p[5]+p[4];

	p[0] = p[5]+di;

	float t_time = 0.0f;
	float t_speed = 0.0f;
	float dis = 0.0f;
	float s_time;
	float prev_dis = 0.0f;

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND); 
	glColor4f(color.x, color.y, color.z, 0.5f);
	glBegin(GL_TRIANGLES);
	//printf("\n");
	for(int i = 0; i < num; ++i)
	{

		p[0] -= di;
		p[1] -= di;
		p[2] -= di;
		p[3] -= di;
		p[4] -= di;
		p[5] -= di;
		t_time = _arrowTime + latency*i;
		t_speed = speed+acc*t_time;
		if(abs(t_speed) > abs(speed))
		{
			t_time = (t_speed+speed)/acc;
		}
		if(i == 0) s_time = t_time;
		prev_dis = dis;
		dis = speed*t_time+0.5f*acc*t_time*t_time;
		t_speed = speed+acc*t_time;
		
		glColor4f(color.x, color.y, color.z, 0.5f*abs(t_speed)/abs(speed));

		//printf("\n%f", dis);

		p[0].y += dis-prev_dis;
		p[1].y += dis-prev_dis;
		p[2].y += dis-prev_dis;
		p[3].y += dis-prev_dis;
		p[4].y += dis-prev_dis;
		p[5].y += dis-prev_dis;

		glVertex3f(p[0].x, p[0].y, p[0].z);
		glVertex3f(p[2].x, p[2].y, p[2].z);
		glVertex3f(p[1].x, p[1].y, p[1].z);

		glVertex3f(p[0].x, p[0].y, p[0].z);
		glVertex3f(p[3].x, p[3].y, p[3].z);
		glVertex3f(p[2].x, p[2].y, p[2].z);

		glVertex3f(p[0].x, p[0].y, p[0].z);
		glVertex3f(p[4].x, p[4].y, p[4].z);
		glVertex3f(p[3].x, p[3].y, p[3].z);

		glVertex3f(p[0].x, p[0].y, p[0].z);
		glVertex3f(p[1].x, p[1].y, p[1].z);
		glVertex3f(p[4].x, p[4].y, p[4].z);

		glVertex3f(p[1].x, p[1].y, p[1].z);
		glVertex3f(p[2].x, p[2].y, p[2].z);
		glVertex3f(p[3].x, p[3].y, p[3].z);

		glVertex3f(p[1].x, p[1].y, p[1].z);
		glVertex3f(p[3].x, p[3].y, p[3].z);
		glVertex3f(p[4].x, p[4].y, p[4].z);
	}
	_arrowTime = s_time;
	glEnd();
	glDisable(GL_BLEND); 
}
/*void FltkViewer::drawActiveArrow(SrVec& from, SrVec& to, int num, float width, SrVec& color, bool spin)
{
	spin_angle += 0.02f;
	if(spin_angle >= 3.1415926535f*2.0f) spin_angle = 0.0f;
	SrVec di = (to - from)/num;


	float speed = 40.0f;
	float acc = -80.0f;
	float latency = 0.06f;
	
	time += 0.01666f;

	SrVec p[4];
	p[3] = to;
	SrMat mat;
	mat.rot(di, spin_angle);

	p[1].x = width/2.0f;
	p[2].x = -width/2.0f;

	p[1] = p[1]*mat;
	p[2] = p[2]*mat;
	p[1] = p[3]+p[1];
	p[2] = p[3]+p[2];
	p[0] = p[3]+di;

	float t_time = 0.0f;
	float t_speed = 0.0f;
	float dis = 0.0f;
	float s_time;
	float prev_dis = 0.0f;

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND); 
	glColor4f(color.x, color.y, color.z, 0.3f);
	glBegin(GL_TRIANGLES);
	//printf("\n");
	for(int i = 0; i < num; ++i)
	{
		p[0] -= di;
		p[3] -= di;
		p[1] -= di;
		p[2] -= di;
		t_time = time+latency*i;
		t_speed = speed+acc*t_time;
		if(abs(t_speed) > abs(speed))
		{
			t_time = (t_speed+speed)/acc;
		}
		if(i == 0) s_time = t_time;
		prev_dis = dis;
		dis = speed*t_time+0.5f*acc*t_time*t_time;

		//printf("\n%f", dis);

		p[0].y += dis-prev_dis;
		p[1].y += dis-prev_dis;
		p[2].y += dis-prev_dis;
		p[3].y += dis-prev_dis;

		glVertex3f(p[0].x, p[0].y, p[0].z);
		glVertex3f(p[1].x, p[1].y, p[1].z);
		glVertex3f(p[2].x, p[2].y, p[2].z);

		glVertex3f(p[0].x, p[0].y, p[0].z);
		glVertex3f(p[2].x, p[2].y, p[2].z);
		glVertex3f(p[1].x, p[1].y, p[1].z);
	}
	time = s_time;
	glEnd();
	glDisable(GL_BLEND); 
}*/

#define FOOT_PRINT_NUM 20
static SrVec footprintpos[3][FOOT_PRINT_NUM][2];
static int footprintstart = 0;
//static int footprintend = 0;
static SrVec footprintcolor[FOOT_PRINT_NUM][2];
static SrVec footprintorientation[FOOT_PRINT_NUM][2];
static SrVec footprintnormal[FOOT_PRINT_NUM][2];
static float footprinttime[FOOT_PRINT_NUM][2];
static int footprintside[FOOT_PRINT_NUM][2];
static SrVec footprint[3][12][2];
static float fadeouttime = 9.0f;
static float footprintscacle = 1.0f;
static SrVec footprintoffset;


void FltkViewer::ChangeOffGroundHeight(Fl_Widget* widget, void* data)
{
	FltkViewer* window = (FltkViewer*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	const char* motionName = window->off_height_window->value();

	window->redraw();
}

void FltkViewer::init_foot_print()
{
	footprintoffset = SrVec(0.0f, 0.0f, -16.0f);
	int i = -1;
	footprint[0][++i][0].x = 8.0f;
	footprint[0][i][0].z = 4.0f;

	footprint[0][++i][0].x = 4.0f;

	footprint[0][++i][0].x = -6.0f;

	footprint[0][++i][0].x = -10.0f;
	footprint[0][i][0].z = 4.0f;

	footprint[0][++i][0].x = -9.0f;
	footprint[0][i][0].z = 17.0f;

	footprint[0][++i][0].x = 7.0f;
	footprint[0][i][0].z = 17.0f;

	i = -1;

	footprint[1][++i][0].x = 7.0f;
	footprint[1][i][0].z = 20.0f;

	footprint[1][++i][0].x = 7.0f;
	footprint[1][i][0].z = 20.0f;

	footprint[1][++i][0].x = -9.0f;
	footprint[1][i][0].z = 20.0f;

	footprint[1][++i][0].x = -22.0f;
	footprint[1][i][0].z = 55.0f;

	footprint[1][++i][0].x = 10.0f;
	footprint[1][i][0].z = 50.0f;

	footprint[1][++i][0].x = 10.0f;
	footprint[1][i][0].z = 50.0f;

	i = -1;

	footprint[2][++i][0].x = -22.0f;
	footprint[2][i][0].z = 55.0f;

	footprint[2][++i][0].x = -22.0f;
	footprint[2][i][0].z = 55.0f;

	footprint[2][++i][0].x = -12.0f;
	footprint[2][i][0].z = 69.0f;

	footprint[2][++i][0].x = -6.0f;
	footprint[2][i][0].z = 69.0f;

	footprint[2][++i][0].x = 10.0f;
	footprint[2][i][0].z = 50.0f;

	footprint[2][++i][0].x = 10.0f;
	footprint[2][i][0].z = 50.0f;
	

	footprintscacle = 0.3f;

	float d_z;
	float d_x = 2.0f;
	for(int i = 0; i < 3; ++i)
	{
		if(i == 0) d_z = 0.0f;
		else if(i == 1) d_z = -55.0f-footprintoffset.z;
		else if(i == 2) d_z = -69.0f-footprintoffset.z;
		for(int j = 0; j < 6; ++j)
		{
			footprint[i][j][0].x += d_x;
			footprint[i][j][0].z += d_z;
			footprint[i][j][0] += footprintoffset;
			footprint[i][j][0] *= footprintscacle;
			footprint[i][5-j][1] = footprint[i][j][0];
			footprint[i][5-j][1].x = -footprint[i][5-j][1].x;
		}
	}

}

void FltkViewer::drawKinematicFootprints(int index)
{
	int i = 0;
	SrVec vertex;
	SrMat mat, tmat;
	SrVec forward(0.0f ,0.0f, 1.0f);
	SrVec up(0.0f, 1.0f, 0.0f);

	SrVec color;

	//glDisable(GL_DEPTH_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND); 

	for(int j = 0; j < FOOT_PRINT_NUM; ++j)
	{
		//footprintpos[j] = SrVec(40, 0, 40);
		if(footprinttime[j][index] > fadeouttime) continue;
		up.set(0.0f, 1.0f, 0.0f);
		mat.rot(forward, footprintorientation[j][index]);
		up = up * mat;
		tmat.rot(up, footprintnormal[j][index]);

		//if(footprintside[j] == 0) color.set(0.0f, 1.0f, 0.4f);
		//else color.set(0.0f, 0.4f, 1.0f);

		glColor4f(footprintcolor[j][index].x, footprintcolor[j][index].y, footprintcolor[j][index].z, 0.6f*(fadeouttime-footprinttime[j][index])/fadeouttime);
		
		for(int k = 0; k < 3; ++k)
		{
			if(footprintpos[k][j][index].len() == 0.0f) continue;
			glBegin(GL_POLYGON);
			for(int i = 0; i < 6; ++i)
			{
				vertex = footprint[k][i][footprintside[j][index]];
				vertex = vertex * mat;
				vertex = vertex * tmat;
				vertex += footprintpos[k][j][index];
				vertex.y += index;
				glVertex3f(vertex.x, vertex.y, vertex.z);
			}
			glEnd();
		}
		
	}
	glDisable(GL_BLEND); 

	for(int i = 0; i < FOOT_PRINT_NUM; ++i)
	{
		if( i == footprintstart) continue;
		if(footprinttime[i][index] > fadeouttime) continue;
		footprinttime[i][index] += 0.0166666f;
	}
	//glEnable(GL_DEPTH_TEST);
}


void FltkViewer::newPrints(bool newprint, int index, SrVec& pos, SrVec& orientation, SrVec& normal, SrVec& color, int side, int type)
{
	if(newprint)
	{
		++footprintstart;
		if(footprintstart >= FOOT_PRINT_NUM) footprintstart = 0;

		SrVec v;
		bool j = false;
		for(int i = 0; i < FOOT_PRINT_NUM; ++i)
		{
			v = footprintpos[index][i][type];
			v.y = 0.0f;
			v = v - pos;
			if(v.len()<5.0f)
			{
				footprintstart = i;
				j = true;
				break;
			}
			/*else if(footprinttime[i]> fadeouttime) 
			{
				footprintstart = i;
			}*/
		}
		if(!j)
		{
			for(int i = 0; i < 3; ++i)
			{
				footprintpos[i][footprintstart][type].set(0.0f, 0.0f, 0.0f);
			}
		}
	}
	footprintcolor[footprintstart][type] = color;
	footprintpos[index][footprintstart][type] = pos;
	footprinttime[footprintstart][type] = 0.0f;
	footprintorientation[footprintstart][type] = orientation;
	footprintnormal[footprintstart][type] = normal;
	footprintside[footprintstart][type] = side;
}

void FltkViewer::drawArrow(SrVec& from, SrVec& to, float width, SrVec& color)
{
	SrVec p[2];
	SrVec c[4];
	SrMat mat;
	SrVec normal;
	int seg = 5;
	
	SrVec di = from - to;
	di.normalize();
	normal = di;
	di *= width;
	mat.roty(3.15159265f/6);
	p[0] = di*mat + to;
	c[0] = (di/2.0f)*mat + to;
	mat.roty(-3.15159265f/6);
	p[1] = di*mat + to;
	c[1] = (di/2.0f)*mat + to;

	c[2] = c[0] + (from-to) - normal*di.len()/2.0f;
	c[3] = c[1] + (from-to) - normal*di.len()/2.0f;


	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND); 

	glColor4f(color.x, color.y, color.z, 1.0f);

	glBegin(GL_TRIANGLES);
	glVertex3f(to.x, to.y, to.z);
	glVertex3f(p[0].x, p[0].y, p[0].z);
	glVertex3f(p[1].x, p[1].y, p[1].z);
	glVertex3f(to.x, to.y, to.z);
	glVertex3f(p[1].x, p[1].y, p[1].z);
	glVertex3f(p[0].x, p[0].y, p[0].z);

	SrVec t[4];
	SrVec gap;
	gap = (from-to)/((float)seg*6.0f);
	t[3] = c[0]+gap;
	t[2] = c[1]+gap;
	for(int i = 0; i < seg; ++i)
	{
		glColor4f(color.x, color.y, color.z, (float)((float)(seg-i)/(float)(seg+1)));

		t[0] = t[3]+gap;
		t[1] = t[2]+gap;
		t[3] = t[0]+gap*2;
		t[2] = t[1]+gap*2;
		glVertex3f(t[0].x, t[0].y, t[0].z);
		glVertex3f(t[1].x, t[1].y, t[1].z);
		glVertex3f(t[2].x, t[2].y, t[2].z);
		glVertex3f(t[0].x, t[0].y, t[0].z);
		glVertex3f(t[2].x, t[2].y, t[2].z);
		glVertex3f(t[1].x, t[1].y, t[1].z);
	
		glVertex3f(t[0].x, t[0].y, t[0].z);
		glVertex3f(t[3].x, t[3].y, t[3].z);
		glVertex3f(t[2].x, t[2].y, t[2].z);
		glVertex3f(t[0].x, t[0].y, t[0].z);
		glVertex3f(t[2].x, t[2].y, t[2].z);
		glVertex3f(t[3].x, t[3].y, t[3].z);
	}
	glEnd();
	glDisable(GL_BLEND); 
}

static int pre_dominant = 0;
void FltkViewer::drawLocomotion()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	int counter = 0;
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		//if(!character->get_locomotion_ct()->is_valid()) continue;
		float x, y, z, yaw, pitch, roll;
		character->get_world_offset(x, y, z, yaw, pitch, roll);
		SrVec arrow_start(x, y, z);
		SrVec arrow_end;
		if(_data->showvelocity)
		{
			if(character->get_locomotion_ct()->get_limb_list()->size() > character->get_locomotion_ct()->get_dominant_limb_index())
			{
				SrVec velocity = character->get_locomotion_ct()->get_navigator()->get_global_velocity();
				velocity.normalize();
				velocity *= character->get_locomotion_ct()->get_current_speed()/2.0f;
				float default_speed = character->get_locomotion_ct()->get_limb_list()->get(character->get_locomotion_ct()->get_dominant_limb_index())->blended_anim.global_info->speed/2.0f;
				arrow_end = arrow_start + velocity;
				SrVec color(0.1f, 0.3f, 1.0f);
				drawArrow(arrow_start, arrow_end, 15, color);
				drawCircle(arrow_start.x, arrow_start.y, arrow_start.z, default_speed, 72, color);
			}
		}
		if(_data->showselection)
		{
			if(counter == _locoData->char_index)
			{
				float height = character->getHeight();
				SrVec color;
				//float base_height = character->get_locomotion_ct()->translation_joint_height;
				float base_height = character->getHeight() / 2.0f;
				arrow_end = arrow_start;
				arrow_end.y += height - base_height;
				arrow_start.y += height - base_height + 30.0f * character->getHeight() / 200.0f;
				if(character->get_locomotion_ct()->is_enabled())
				{
					if(character->get_locomotion_ct()->get_limb_list()->get(0)->walking_list.size() < 2)
					{
						color.set(1.0f, 1.0f, 0.0f);
					}
					else color.set(0.0f, 1.0f, 0.2f);
				}
				else color.set(1.0f, 0.0f, 0.0f);
				drawActiveArrow(arrow_start, arrow_end, 3, 10.0f, color, false);
			}
		}
		if(_data->showkinematicfootprints)
		{
			//int cur_dominant = character->get_locomotion_ct()->get_dominant_limb_index();
			//if(i == char_index && character->get_locomotion_ct()->limb_list.size()>cur_dominant)
			if(counter == _locoData->char_index)
			{
				//if(cur_dominant != pre_dominant && character->get_locomotion_ct()->limb_list.get(cur_dominant)->space_time >= 0.0f
				//	&& character->get_locomotion_ct()->limb_list.get(cur_dominant)->space_time < 1.0f)
				for(int k = 0; k < character->get_locomotion_ct()->limb_list.size();++k)
				{
					SrMat mat;
					mat.rot(SrVec(0,1,0), character->get_locomotion_ct()->limb_list.get(k)->curr_rotation+character->get_locomotion_ct()->get_navigator()->get_orientation_angle());
					SrVec orientation = SrVec(0,0,1)*mat;
					SrVec normal;
					bool newprint = true;
					float off_height = 0.0f;
					int j = 0;
					//printf("\n");
					for(int j = 0; j < 3; ++j)
					{
						off_height = character->get_locomotion_ct()->limb_list.get(k)->get_off_ground_height(j+2);
						if(character->get_locomotion_ct()->limb_list.get(0)->walking_list.size() < 2) off_height -= _locoData->off_height_comp;
						//printf("%f ", off_height);
						if(off_height > 0.0f) continue;
						SrVec pos = character->get_locomotion_ct()->get_supporting_joint_pos(j, k, &orientation, &normal);
						pos.y += 0.1f;
						SrVec color(.2f, (float) k, (float) 1-k);
						newPrints(newprint, j, pos, orientation, normal, color, k, 0);
						newprint = false;
					}
					//pre_dominant = k;
				}
				drawKinematicFootprints(0);
			}
		}
		if(_data->showlocofootprints)
		{
			int cur_dominant = character->get_locomotion_ct()->get_dominant_limb_index();
			if (counter == _locoData->char_index && character->get_locomotion_ct()->limb_list.size()>cur_dominant)
			{
				if(cur_dominant != pre_dominant && character->get_locomotion_ct()->limb_list.get(cur_dominant)->get_space_time() >= 0.0f
					&& character->get_locomotion_ct()->limb_list.get(cur_dominant)->get_space_time() < 1.0f)
				{
					SrMat mat;
					mat.rot(SrVec(0,1,0), character->get_locomotion_ct()->limb_list.get(cur_dominant)->curr_rotation+character->get_locomotion_ct()->get_navigator()->get_orientation_angle());
					SrVec orientation = SrVec(0,0,1)*mat;
					SrVec normal;
					bool newprint = true;
					//float off_height = 0.0f;
					//int j = 0;
					//printf("\n");
					for(int j = 0; j < 3; ++j)
					{
						//off_height = character->get_locomotion_ct()->limb_list.get(cur_dominant)->get_off_ground_height(j+2);
						//printf("%f ", off_height);
						//if(off_height > 0.0f) continue;
						SrVec pos = character->get_locomotion_ct()->get_supporting_joint_pos(j, cur_dominant, &orientation, &normal);
						pos.y += 0.1f;
						SrVec color(0.0f, (float) cur_dominant*0.3f, (float)(1-cur_dominant)*0.3f);
						newPrints(newprint, j, pos, orientation, normal, color,  cur_dominant, 1);
						newprint = false;
					}
					pre_dominant = cur_dominant;
				}
				drawKinematicFootprints(1);
			}
		}
		if (_data->showtrajectory)
		{
			glDisable(GL_LIGHTING);
			if (!character->param_animation_ct)
				return;
			glDisable(GL_LIGHTING);
			std::string baseJointName = character->param_animation_ct->getBaseJointName();
			SkJoint* baseJ = character->getSkeleton()->search_joint(baseJointName.c_str());
			if (!baseJ) return;
			character->getSkeleton()->update_global_matrices();
			SrMat baseGM = baseJ->gmat();
			SrVec baseVec = SrVec(baseGM.get(12), baseGM.get(13), baseGM.get(14));
			if (character->trajectoryBuffer.size() >= SbmCharacter::trajectoryLength)
				character->trajectoryBuffer.pop_front();
			SrVec prevBaseVec = baseVec;
			if (character->trajectoryBuffer.size())
				prevBaseVec = character->trajectoryBuffer.back();
			//if ((baseVec-prevBaseVec).len() > character->getHeight()*0.01f)
				character->trajectoryBuffer.push_back(baseVec);
			std::list<SrVec>::iterator iter = character->trajectoryBuffer.begin();
			glColor3f(1.0f, 1.0f, 0.0f);
			glBegin(GL_LINES);
			for (; iter != character->trajectoryBuffer.end(); iter++)
			{
				std::list<SrVec>::iterator iter1 = iter;
				iter1++;
				if (iter1 != character->trajectoryBuffer.end())
				{
					glVertex3f(iter->x, 0.5f, iter->z);
					glVertex3f(iter1->x, 0.5f, iter1->z);
				}
			}
			glEnd();

			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_LINES);
			int num = int(character->trajectoryGoalList.size() / 3) - 1;
			if (num >= 1)
				for (int i = 0; i < num; i++)
				{
					glVertex3f(character->trajectoryGoalList[(size_t)i * 3 + 0], 0.5f, character->trajectoryGoalList[(size_t)i * 3 + 2]);
					glVertex3f(character->trajectoryGoalList[((size_t)i + 1) * 3 + 0], 0.5f, character->trajectoryGoalList[((size_t)i + 1) * 3 + 2]);
				}
			glEnd();
			glEnable(GL_LIGHTING);

			glDisable(GL_LIGHTING);
			float scale = (float)1.0/SmartBody::SBScene::getScene()->getScale(); // if it's in meter
			if (character->steeringAgent)
			{
				SteeringAgent* agent = character->steeringAgent;

				SrVec color1(0.1f, 0.3f, 1.0f);
				SrVec steerDir = agent->curSteerPos + agent->curSteerDir * 0.5f*scale;
				drawArrow(agent->curSteerPos, steerDir, 0.15f*scale, color1);

				SrVec color2(0.f,1.f,0.f);
				drawCircle(agent->nextSteerPos.x,agent->nextSteerPos.y,agent->nextSteerPos.z, 0.3f*scale, 72, color2);
				SrVec nextSteerPos = agent->nextSteerPos + agent->nextSteerDir * 0.5*scale;
				drawArrow(agent->nextSteerPos, nextSteerPos, 0.15f*scale, color2);

				SrVec color3(1.f,0.f,0.f);
				drawCircle(agent->nextPtOnPath.x, agent->nextPtOnPath.y, agent->nextPtOnPath.z, 0.3f*scale, 72, color3);											
			}
			glEnable(GL_LIGHTING);
		}
	}
}

void FltkViewer::drawInteractiveLocomotion()
{
	if (!_data->interactiveLocomotion)
		return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	float pawnSize = 1.0;
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		pawnSize = character->getHeight() / 8.0f;
		break;
	}

	glPushMatrix();
	glTranslatef(interactivePoint[0], 0.0, interactivePoint[2]);
	SrSnSphere sphere;
	sphere.shape().center = SrPnt(0, 0, 0);
	sphere.shape().radius = pawnSize;
	sphere.render_mode(srRenderModeLines);
	SrGlRenderFuncs::render_sphere(&sphere);
	glPopMatrix();
}

void FltkViewer::drawDynamics()
{
	if (_data->dynamicsMode == ModeNoDynamics && !_data->showmasses)
		return;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
			iter != mcu.getCharacterMap().end();
			iter++)
	{
		SbmCharacter* character = (*iter).second;
		character->getSkeleton()->update_global_matrices();

		const std::vector<SkJoint*>& joints = character->getSkeleton()->joints();
		
		int numJoints = 0;
		float totalMass = 0;
		SrVec com(0, 0, 0);
		for (size_t j = 0; j < joints.size(); j++)
		{
			float mass = joints[j]->mass();
			if (mass > 0)
			{
				totalMass += mass;
				SrMat gmat = joints[j]->gmat();
				SrVec loc(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14)); 
				com += mass * loc;
				numJoints++;
			}
		}
		if (totalMass != 0)
				com /= totalMass;

		glPushMatrix();
		glPushAttrib(GL_POINT_BIT);
		glPointSize(4.0);
		
		if (_data->dynamicsMode == ModeShowCOM ||
			_data->dynamicsMode == ModeShowCOMSupportPolygon)
		{
			
			// draw the center of mass of the character
			glColor3f(1.0, 1.0, 0.0);
			glPushMatrix();
			glTranslatef(com[0], com[1], com[2]);
			glBegin(GL_POINTS);
			glVertex3f(0.0, 0.0, 0.0);
			glEnd();
			glPopMatrix();

			
		}

		if (_data->dynamicsMode == ModeShowCOMSupportPolygon)
		{
			// draw the support polygon of the character
			// get the heel/toe points for the left and right foot
			SrVec polygon[4];
			// left heel, toe
			SkJoint* leftFoot = character->getSkeleton()->search_joint("l_ankle");
			if (leftFoot)
			{
				SrMat gmat = leftFoot->gmat();
				polygon[0].set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14)); 
			}
			SkJoint* leftToe = character->getSkeleton()->search_joint("l_toe");
			if (leftToe)
			{
				SrMat gmat  = leftToe->gmat();
				polygon[1].set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14)); 
			}			
			// right heel, toe
			SkJoint* rightFoot =character->getSkeleton()->search_joint("r_ankle");
			if (rightFoot)
			{
				SrMat gmat = rightFoot->gmat();
				polygon[3].set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14)); 
			}
			SkJoint* rightToe = character->getSkeleton()->search_joint("r_toe");
			if (rightToe)
			{
				SrMat gmat = rightToe->gmat();
				polygon[2].set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14)); 
			}			

			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (int x = 0; x < 4; x++)
			{
				glVertex3f(polygon[x][0], polygon[x][1], polygon[x][2]);
			}
			glEnd();

			// show the center of mass projected on the ground as well
			float yLoc = 0;
			for (int i = 0; i < 4; i++)
				yLoc += polygon[i][1];
			yLoc /= 4.0;
			glColor3f(1.0, 1.0, 0.0);
			glPushMatrix();
			glTranslatef(com[0], yLoc, com[2]);
			glBegin(GL_POINTS);
			glVertex3f(0.0, 0.0, 0.0);
			glEnd();
			glPopMatrix();

		}

		// should be rendered based on physics geometry.
		/*
		if (_data->showmasses && totalMass > 0)
		{
			glPushAttrib(GL_LIGHTING_BIT);
			glEnable(GL_LIGHTING);
			glColor3f(1.0f, 1.0f, 0.0f);
			SrSnSphere sphere;
			float height = 200.0;
			for (size_t j = 0; j < joints.size(); j++)
			{
				float mass = joints[j]->mass();
				if (mass > 0)
				{
					float proportion = mass / totalMass;
					// draw a sphere of proportionate size to entire character to show mass distribution
					SrMat gmat = joints[j]->gmat();
					glPushMatrix();
					sphere.shape().center = SrPnt(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
					sphere.shape().radius = proportion * height;
					SrGlRenderFuncs::render_sphere(&sphere);
					// draw sphere with size (proportion * height)
					glPopMatrix();
				}
			}
			
			glPopAttrib();
		}
		*/
		
		glPopAttrib();
		glPopMatrix();
	}
}


SbmCharacter* FltkViewer::getCurrentCharacter()
{
	 mcuCBHandle& mcu = mcuCBHandle::singleton();
	 SbmPawn* selectedPawn = getObjectManipulationHandle().get_selected_pawn();
	 if (!selectedPawn)
	 {
		 SbmCharacter* character = mcu.getCharacter(_lastSelectedCharacter);
		 if (character)
			 return character;
		 else
		 {
			 // get the first character
			 std::map<std::string, SbmCharacter*>& characterMap = mcu.getCharacterMap();
			 std::map<std::string, SbmCharacter*>::iterator iter = characterMap.begin();
			 if (iter !=  characterMap.end())
			 {
				 _lastSelectedCharacter = (*iter).second->getName();
				  SbmCharacter* character = mcu.getCharacter(_lastSelectedCharacter);
				 return character;
			 }
			 else
			 {
				 return NULL;
			 }
		 }
	 }

	 SbmCharacter* character = dynamic_cast<SbmCharacter*> (selectedPawn);
	 return character;
}


MeCtExampleBodyReach* FltkViewer::getCurrentCharacterBodyReachController()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = getCurrentCharacter();

	MeCtExampleBodyReach* reachCt = NULL;
	if ( character )
	{
		MeCtSchedulerClass* reachSched = character->reach_sched_p;
		if (!reachSched)
			return NULL;
		MeCtSchedulerClass::VecOfTrack reach_tracks = reachSched->tracks();		
		MeCtReach* tempCt = NULL;
		for (unsigned int c = 0; c < reach_tracks.size(); c++)
		{
			MeController* controller = reach_tracks[c]->animation_ct();		
			//reachCt = dynamic_cast<MeCtConstraint*>(controller);
			reachCt = dynamic_cast<MeCtExampleBodyReach*>(controller);
			//tempCt  = dynamic_cast<MeCtReach*>(controller);
			if (reachCt)
				break;
		}		
	}
	return reachCt;
}



MeCtConstraint* FltkViewer::getCurrentCharacterConstraintController()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = getCurrentCharacter();

	MeCtConstraint* reachCt = NULL;
	if ( character )
	{
		MeCtSchedulerClass* reachSched = character->constraint_sched_p;
		if (!reachSched)
			return NULL;
		MeCtSchedulerClass::VecOfTrack reach_tracks = reachSched->tracks();		
		MeCtReach* tempCt = NULL;
		for (unsigned int c = 0; c < reach_tracks.size(); c++)
		{
			MeController* controller = reach_tracks[c]->animation_ct();		
			reachCt = dynamic_cast<MeCtConstraint*>(controller);
			//tempCt  = dynamic_cast<MeCtReach*>(controller);
			if (reachCt)
				break;
		}		
	}
	return reachCt;
}

// visualize example data and other relevant information for reach controller
void FltkViewer::drawReach()
{	
	if (_data->reachRenderMode == ModeNoExamples)
		return;
	
	glPushAttrib(GL_LIGHTING_BIT | GL_POINT_BIT);
	glDisable(GL_LIGHTING);
	
	//MeCtDataDrivenReach* reachCt = getCurrentCharacterReachController();

	MeCtExampleBodyReach* reachCt = getCurrentCharacterBodyReachController();
	
	SbmCharacter* character = getCurrentCharacter();
	if (!character)
		return;
	float sphereSize = character->getHeight() / 50.0f;	
	if (reachCt)
	{	
		// draw reach Example data
		/*
		const VecOfPoseExample& exampleData = reachCt->ExamplePoseData().PoseData();
		const VecOfPoseExample& resampledData = reachCt->ResampledPosedata().PoseData();
		character->skeleton_p->update_global_matrices();		
		//SkJoint* root = reachCt->getRootJoint();		
		*/
		

		MeCtReachEngine* re = reachCt->getReachEngine();
		if (!re)
			return;

		ReachStateData* rd = re->getReachData();
		if (!rd)
			return;
		
		SkJoint* root = character->getSkeleton()->root();
		SrMat rootMat = rd->gmat;//root->gmat();
		//rootMat.translation(root->gmat().get(12),root->gmat().get(13),root->gmat().get(14));
		const std::vector<SrVec>& exampleData = re->examplePts;
		const std::vector<SrVec>& resampleData = re->resamplePts;		

		SrPoints srExamplePts;	
		srExamplePts.init_with_attributes();
		for (unsigned int i=0;i<exampleData.size();i++)
		{
			//const PoseExample& exPose = exampleData[i];
			SrVec lPos = exampleData[i];//SrVec((float)exPose.poseParameter[0],(float)exPose.poseParameter[1],(float)exPose.poseParameter[2]);
			SrVec gPos = lPos*rootMat; // transform to global coordinate
			//drawCircle(gPos[0],gPos[1],gPos[2],1.0,5,SrVec(1.0,0.0,0.0));			
			//drawPoint(gPos[0],gPos[1],gPos[2],5.0,SrVec(0.0,0.0,1.0));
			glColor3f(0.0, 0.0, 1.0);
			SrSnSphere sphere;			
			sphere.shape().center = SrPnt(gPos[0], gPos[1], gPos[2]);
			sphere.shape().radius = sphereSize;
			sphere.color(SrColor(0.f,0.f,1.f));
			sphere.render_mode(srRenderModeLines);
			SrGlRenderFuncs::render_sphere(&sphere);
			//PositionControl::drawSphere(gPos,1.0f);			
		}	

	    // tetra hedron rendering, disabled for now.
// 		const VecOfSimplex& simplexList = reachCt->simplexList;
// 		SrVec tetraVtx[4];
// 		for (unsigned int i=0;i<simplexList.size();i++)
// 		{
// 			if (i != reachCt->simplexIndex )
// 				continue;
// 			const Simplex& sp = simplexList[i];
// 			for (int k=0;k<4;k++)
// 			{
// 				tetraVtx[k] = exampleData[sp.vertexIndices[k]];//*rootMat;
// 				//tetraVtx[k].z *= 3.f;
// 				tetraVtx[k] = tetraVtx[k]*rootMat;
// 			}
// 
// 			drawTetra(tetraVtx,SrVec(1,0,0));
// 		}
	
		for (unsigned int i=0;i<resampleData.size();i++)
		{			
			SrVec lPos = resampleData[i];
			SrVec gPos = lPos*rootMat; // transform to global coordinate
			//drawCircle(gPos[0],gPos[1],gPos[2],1.0,5,SrVec(1.0,0.0,0.0));			
			SrVec yaxis(0.0, 1.0, 0.0);
			drawPoint(gPos[0],gPos[1],gPos[2],1.5, yaxis);
			//PositionControl::drawSphere(gPos,1.0f);			
		}	
// 		if (reachCt->posPlanner && reachCt->posPlanner->path())
// 		{
// 			if (reachCt->posPlanner->path())
// 			{
// 				SrSnLines pathLines;			
// 				reachCt->posPlanner->draw_path(*reachCt->posPlanner->path(),pathLines.shape());
// 				pathLines.color(SrColor(1,0,0,1));
// 				SrGlRenderFuncs::render_lines(&pathLines);		
// 			}
// 
// 			SrSnBox box;
// 			box.shape() = reachCt->posPlanner->cman()->SkPosBound();
// 			//SrGlRenderFuncs::render_box(&box);
// 		}

// 		if (reachCt->blendPlanner)
// 		{
// 			if (reachCt->blendPlanner->path())
// 			{
// 				SrSnLines pathLines;			
// 				reachCt->blendPlanner->draw_path(*reachCt->blendPlanner->path(),pathLines.shape());
// 				pathLines.color(SrColor(1,0,0,1));
// 				SrGlRenderFuncs::render_lines(&pathLines);		
// 			}	
// 
// 			if (reachCt->blendPlanner->cman())
// 			{
// 				SrSnLines tree1;
// 				reachCt->blendPlanner->draw_edges(tree1.shape(),0);
// 
// 				SrSnLines tree2;
// 				reachCt->blendPlanner->draw_edges(tree2.shape(),1);
// 				SrGlRenderFuncs::render_lines(&tree1);
// 				SrGlRenderFuncs::render_lines(&tree2);
// 			}			
// 		}
// 
// 
// 
// 
// 		if (reachCt->blendPlanner && reachCt->blendPlanner->cman())
// 		{
// 			std::vector<CollisionJoint>& colJointList = reachCt->blendPlanner->cman()->colJoints;
// 			for (unsigned int i=0;i<colJointList.size();i++)
// 				drawColObject(colJointList[i].colGeo);
// 		}

		EffectorState& es = rd->effectorState;
// 		SrVec reachTraj = es.curState.tran;
// 		PositionControl::drawSphere(reachTraj,sphereSize,SrVec(0,1,1));
		SrVec ikTraj = es.curIKTargetState.tran;		
		PositionControl::drawSphere(ikTraj,sphereSize,SrVec(1,0,1));
		SrVec ikTarget = es.ikTargetState.tran;
		

// 		glColor3f(1.0, 0.0, 0.0);
// 		glBegin(GL_LINES);
// 		glVertex3f(reachTraj.x,reachTraj.y,reachTraj.z);
// 		glVertex3f(ikTarget.x,ikTarget.y,ikTarget.z);
// 		glEnd();
	}

	glPopAttrib();
	
	
}

void FltkViewer::makeGLContext()
{
	make_current();
}




void FltkViewer::drawColObject( SbmGeomObject* colObj, SrMat& gmat )
{
	glEnable(GL_LIGHTING);
	glPushMatrix();
	//SrMat gMat = colObj->worldState.gmat();
	SrColor objColor;
	objColor.set(colObj->color.c_str());
	objColor.a = (srbyte)64;
	

	glEnable ( GL_ALPHA_TEST );
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc ( GL_GREATER, 0.1f ) ;
	
	SrMat gMat = gmat;
	SrMat lMat = colObj->getLocalTransform().gmat();	
	glMultMatrixf((const float*) gMat);
	glMultMatrixf((const float*) lMat);
	if (dynamic_cast<SbmGeomSphere*>(colObj))
	{
		// draw sphere
		SbmGeomSphere* sph = dynamic_cast<SbmGeomSphere*>(colObj);
		SrSnSphere sphere;					
		sphere.shape().radius = sph->radius;
		sphere.color(objColor);
		sphere.render_mode(srRenderModeSmooth);
		SrGlRenderFuncs::render_sphere(&sphere);		
	}
	else if (dynamic_cast<SbmGeomBox*>(colObj))
	{
		SbmGeomBox* box = dynamic_cast<SbmGeomBox*>(colObj);
		SrSnBox sbox;					
		sbox.shape().a = -box->extent;
		sbox.shape().b = box->extent;
		sbox.color(objColor);
		sbox.render_mode(srRenderModeSmooth);
		SrGlRenderFuncs::render_box(&sbox);
	}
	else if (dynamic_cast<SbmGeomCapsule*>(colObj))
	{
		SbmGeomCapsule* cap = dynamic_cast<SbmGeomCapsule*>(colObj);
		// render two end cap
		SrSnSphere sphere;				
		sphere.shape().center = cap->endPts[0];//SrVec(0,-cap->extent,0);
		sphere.shape().radius = cap->radius;
		sphere.color(objColor);
		sphere.render_mode(srRenderModeSmooth);
		SrGlRenderFuncs::render_sphere(&sphere);

		sphere.shape().center = cap->endPts[1];//SrVec(0,cap->extent,0);
		SrGlRenderFuncs::render_sphere(&sphere);

		SrSnCylinder cyc;
		cyc.shape().a = cap->endPts[0];//SrVec(0,-cap->extent,0);
		cyc.shape().b = cap->endPts[1];//SrVec(0,cap->extent,0);
		cyc.shape().radius = cap->radius;
		cyc.color(objColor);
		cyc.render_mode(srRenderModeSmooth);
		SrGlRenderFuncs::render_cylinder(&cyc);
	}
	else if (dynamic_cast<SbmGeomTriMesh*>(colObj))
	{
		SbmGeomTriMesh* mesh = dynamic_cast<SbmGeomTriMesh*>(colObj);
		if (mesh->geoMesh)
		{
			SrSnModel model;
			model.shape(*mesh->geoMesh);
			model.color(objColor);
			model.render_mode(srRenderModeSmooth);
			SrGlRenderFuncs::render_model(&model);
		}
	}
	glPopMatrix();	
	glDisable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST) ;
	glDisable(GL_BLEND);
}


void FltkViewer::drawSteeringInfo()
{
	if (_data->steerMode == ModeNoSteer)
		return;

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!scene->getSteerManager()->getEngineDriver()->isInitialized() || 
		!scene->getSteerManager()->getEngineDriver()->_engine)
		return;

	glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
	glPushMatrix();
	glDisable(GL_LIGHTING);

	glScalef(1 / scene->getScale(), 1 / scene->getScale(), 1 / scene->getScale());

	//comment out for now, have to take a look at the steering code
	const std::vector<SteerLib::AgentInterface*>& agents = mcu._scene->getSteerManager()->getEngineDriver()->_engine->getAgents();
	for (size_t x = 0; x < agents.size(); x++)
	{
		mcu._scene->getSteerManager()->getEngineDriver()->_engine->selectAgent(agents[x]);
		agents[x]->draw();
	}

	const std::set<SteerLib::ObstacleInterface*>& obstacles = mcu._scene->getSteerManager()->getEngineDriver()->_engine->getObstacles();
	for (std::set<SteerLib::ObstacleInterface*>::const_iterator iter = obstacles.begin();
		iter != obstacles.end();
		iter++)
	{
		(*iter)->draw();
	}
	
	if (_data->steerMode == ModeSteerAll)
	{
		mcu._scene->getSteerManager()->getEngineDriver()->_engine->getSpatialDatabase()->draw();
	}

	glPopMatrix();
	glPopAttrib();

}

void FltkViewer::notify(SmartBody::SBSubject* subject)
{
	SbmPawn* pawn = dynamic_cast<SbmPawn*>(subject);
	if (pawn)
	{
		if (_paLocoData)
		{
			if (_paLocoData->character == pawn)
			{
				_paLocoData->character = NULL;
			}
		}
	}
}

void FltkViewer::makeShadowMap()
{

	SrLight &light = _data->light;
	float shad_modelview[16];
	
	//	glDisable(GL_LIGHTING);
	//glDisable(GL_TEXTURE_2D);
	// since the shadow maps have only a depth channel, we don't need color computation
	// glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	
	glLoadIdentity();
	gluPerspective(70,1,30.f,3000.f);
	
	SrLight& shadowLight = _lights[0]; // assume direction light
	SrVec dir = shadowLight.position;
	dir.normalize();
	float distance = 800;
	dir*= distance;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	// the light is from position to origin
	//gluLookAt(light.position[0], light.position[1], light.position[2], 0.f, 0.f, 0.f, 0.0f, 1.0f, 0.0f);
	//gluLookAt(light.position[0], light.position[1], light.position[2], 0,0,0, -1.0f, 0.0f, 0.0f);
	//gluLookAt(0, 0, 0, -light.position[0], -light.position[1], -light.position[2], -1.0f, 0.0f, 0.0f);
	//gluLookAt(0, 600, 700, 0,0,0, 0.0f, 1.0f, 0.0f);
	gluLookAt(dir[0],dir[1],dir[2],0,0,0,0,1,0);
	glGetFloatv(GL_MODELVIEW_MATRIX, shad_modelview);
	// redirect rendering to the depth texture
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _data->depthFB);
	// store the screen viewport
	glPushAttrib(GL_VIEWPORT_BIT);
	int depth_size = SHADOW_MAP_RES;
	// and render only to the shadowmap
	glViewport(0, 0, depth_size, depth_size);
	// offset the geometry slightly to prevent z-fighting
	// note that this introduces some light-leakage artifacts
	//glPolygonOffset( 1.0f, 4096.0f);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	// draw all faces since our terrain is not closed.
	//
	glDisable(GL_CULL_FACE);	
	//glCullFace(GL_FRONT);
	// for all shadow maps:
	// make the current depth map a rendering target
	//glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, depth_tex_ar, 0, i);
	//glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, _data->shadowMapID, 0);
	//glBindTexture(GL_TEXTURE_2D, _data->depthMapID);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
	// clear the depth texture from last time
	//glEnable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);	
	// draw the scene
	//terrain->Draw(minZ);	
	drawAllGeometries(true);
	glMatrixMode(GL_PROJECTION);
	// store the product of all shadow matries for later
	glMultMatrixf(shad_modelview);
	glGetFloatv(GL_PROJECTION_MATRIX, _data->shadowCPM);
	

	// revert to normal back face culling as used for rendering
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPopAttrib(); 
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	//SbmShaderProgram::printOglError("shadowMapError");

}

void FltkViewer::timerUpdate( void* data )
{
	PALocomotionData* paLocoData = (PALocomotionData*)data;	
	paLocoData->updateKeys(0.01f);
	Fl::repeat_timeout(0.01,timerUpdate,paLocoData);	
}

PALocomotionData::PALocomotionData()
{
	w = 0;
	v = 0;
	s = 0;
	character = NULL;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		character = (*iter).second;
		break;
	}
	
	starting = false;
	stopping = false;
	jumping = false;
	prevStarting = false;
	prevStopping = false;
	prevJumping = false;
	linearVelocityIncrement = 5.0f;
	angularVelocityIncrement = 10.0f;	
	keyControl = false;

	keyPressMap[KEY_LEFT] = false;
	keyPressMap[KEY_RIGHT] = false;
	keyPressMap[KEY_TURNLEFT] = false;
	keyPressMap[KEY_TURNRIGHT] = false;
	keyPressMap[KEY_UP] = false;
	keyPressMap[KEY_DOWN] = false;
	keyPressMap[KEY_SHIFT] = false;
}

PALocomotionData::~PALocomotionData()
{
}

void PALocomotionData::pressKey( int keyID )
{
	keyPressMap[keyID] = true;	
	keyControl = true;
}

void PALocomotionData::releaseKey( int keyID )
{
	keyPressMap[keyID] = false;
}

std::string PALocomotionData::getLocomotionStateName()
{
	if (!character)
		return "";
	SmartBody::SBSteerManager* manager = SmartBody::SBScene::getScene()->getSteerManager();
// 	SmartBody::SBSteerAgent* steerAgent = manager->getSteerAgent(character->getName());
// 	if (!steerAgent)
// 		return "";
// 	const std::string& prefix = steerAgent->getSteerStateNamePrefix();
	//std::string stateName = prefix + "Locomotion";
	std::string stateName = "ChrMarineLocomotion";
	return stateName;
}

void PALocomotionData::updateKeys(float dt)
{
	//LOG("paLocomotionUpdateKeys");
	// acceleration when the key is pressed
	if (!keyControl) return;

	std::string locoStateName = getLocomotionStateName();
	if (locoStateName == "")
		return;
	PABlendData* blendData = NULL;
	if (character && character->param_animation_ct)
		if (character->param_animation_ct)
			blendData = character->param_animation_ct->getCurrentPABlendData();	

	if (!blendData) return;
	float unitScale = 1.f/SmartBody::SBScene::getScene()->getScale();
	float scale = 0.05f;
	if (blendData->state->stateName == locoStateName)
		scale = 1.f;
	
	float linearAcc = 1.f*scale*unitScale;
	float angularAcc = 200.f*scale;
	float strifeAcc = 1.f*scale*unitScale;

	// automatic de-acceleration when the key is not pressed
	float linearDcc = linearAcc*2.5f;
	float angularDcc = angularAcc*2.0f;
	float straifeDc = strifeAcc*2.5f;
	
	if (keyPressMap[KEY_UP] || keyPressMap[KEY_DOWN])
	{
		if (keyPressMap[KEY_UP])
		{
			v += linearAcc*dt;
		}
		else if (keyPressMap[KEY_DOWN])
		{
			v -= linearAcc*dt;
		}
	}
	else // gradually de-accelerate
	{
		if (v > 0)
		{
			v -= linearDcc*dt; 			
		}		
	}

	if (keyPressMap[KEY_LEFT] || keyPressMap[KEY_RIGHT])
	{
		if (keyPressMap[KEY_RIGHT])
		{
			s += strifeAcc*dt;
		}
		else if (keyPressMap[KEY_LEFT])
		{
			s -= strifeAcc*dt;
		}
	}
	else // gradually de-accelerate
	{
		if (s > 0) 
		{
			s -= straifeDc*dt; 
			if (s < 0) s = 0;
		}
		else if (s < 0) 
		{
			s += straifeDc*dt;
			if (s > 0) w =s;
		}	
	}

	if (keyPressMap[KEY_TURNLEFT] || keyPressMap[KEY_TURNRIGHT])
	{
		if (keyPressMap[KEY_TURNLEFT])
		{
			w += angularAcc*dt;		
		}
		else 
		{
			w -= angularAcc*dt;
		}
	}
	else // gradually de-accelerate
	{
		if (w > 0) 
		{
			w -= angularDcc*dt; 
			if (w < 0) w = 0;
		}
		else if (w < 0) 
		{
			w += angularDcc*dt;
			if (w > 0) w =0;
		}
	}

	// speed limits	
	float runSpeedLimit = 4.f*unitScale;
	float walkSpeedLimit = 1.2f*unitScale;
	float angSpeedLimit = 140.f;
	float strifeSpeedLimit = 1.f*unitScale;
	// make sure the control parameter does not exceed limits
	

	if (w > angSpeedLimit) w = angSpeedLimit;
	if (w < -angSpeedLimit) w = -angSpeedLimit;

	if (s > strifeSpeedLimit) s = strifeSpeedLimit;
	if (s < -strifeSpeedLimit) s = -strifeSpeedLimit;
	//if (v > runSpeedLimit) v = runSpeedLimit;		
	if (keyPressMap[KEY_SHIFT])
	{
		if (v > runSpeedLimit)
			v = runSpeedLimit;

		if (v <= runSpeedLimit && v > walkSpeedLimit)
		{
			v -= (linearAcc*dt+linearDcc*dt);	
			if (v < walkSpeedLimit)
				v = walkSpeedLimit;
		}
	}
	else
	{
		if (v > runSpeedLimit)
			v = runSpeedLimit;
	}
	if (v < 0) v = 0; // we don't allow backward walking....yet 
	
	// update speed 	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	//float pv, pw, ps;
	//state->getParameter(pv,pw,ps);
	float speedEps = 0.001f*unitScale;	
	float angleEps = 0.01f;
	if ( (fabs(v) > 0 || fabs(w) > 0 || fabs(s)) && 
		blendData->state->stateName == PseudoIdleState && !starting)
	{
		std::string locomotionStateName =  getLocomotionStateName();
		std::stringstream command1;			
		PABlend* locoState = mcu.lookUpPABlend(locomotionStateName);
		if (!locoState)
			return;
		std::vector<double> weights;
		weights.resize(locoState->getNumMotions());
		locoState->getWeightsFromParameters(0, 0, 0, weights);
		command1 << "panim schedule char " << character->getName() << " state " <<  locomotionStateName << " loop true playnow false additive false joint null ";		
		for (int i = 0; i < locoState->getNumMotions(); i++)
			command1 << weights[i] << " ";
		//LOG("startLocomotion, %s",command1.str().c_str());
		mcu.execute((char*)command1.str().c_str());	
		starting = true;
		stopping = false;
	}
	else if (fabs(v) < speedEps && 
		     fabs(w) < angleEps && 
			 fabs(s) < speedEps && 
			 blendData->state->stateName == locoStateName && 
			 !stopping)
	{	
		//LOG("stop, v = %f, w = %f, s = %f",v,w,s);
		v = w = 0;
		std::vector<double> weights;
		weights.resize(blendData->state->getNumMotions());
		bool success = blendData->state->getWeightsFromParameters(0,0,0, weights);
		std::stringstream command;
		command << "panim schedule char " << character->getName() << " state " << "null" << " loop true playnow true additive false joint null ";
		
		//LOG("stopLocomotion, %s",command.str().c_str());
		mcu.execute((char*)command.str().c_str());				
		stopping = true;
		starting = false;
		keyControl = false;
	}
	else // update moving parameter
	{		
		std::vector<double> weights;
		weights.resize(blendData->state->getNumMotions());
		bool success = blendData->state->getWeightsFromParameters(v, w, s, weights);		
 		character->param_animation_ct->updateWeights(weights);
	}	
		
}

//================================ End of File =================================================
