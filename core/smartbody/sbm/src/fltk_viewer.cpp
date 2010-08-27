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

# include "fltk_viewer.h"


# include <fltk/events.h>
# include <fltk/gl.h>
# include <fltk/run.h>
# include <fltk/visual.h>
# include <fltk/compat/FL/Fl_Menu_Item.H>
# include <fltk/draw.h>
# include <fltk/PopupMenu.h>
# include <fltk/ColorChooser.H>
# include <fltk/FileChooser.H>
# include <fltk/Browser.H>

# include <SR/sr_box.h>
# include <SR/sr_quat.h>
# include <SR/sr_line.h>
# include <SR/sr_plane.h>
# include <SR/sr_event.h>
# include <SR/sr_timer.h>
# include <SR/sr_string.h>

# include <SR/sr_gl.h>
# include <SR/sr_light.h>
# include <SR/sr_camera.h>
# include <SR/sr_trackball.h>
# include <SR/sr_lines.h>
# include <SR/sr_color.h>

# include <SR/sr_sn.h>
# include <SR/sr_sn_group.h>

# include <SR/sr_sa.h>
# include <SR/sr_sa_event.h>
# include <SR/sr_sa_bbox.h>
# include <SR/sr_sa_gl_render.h>

#include <sbm/mcontrol_util.h>
#include "vhcl_log.h"

////# define SR_USE_TRACE1  // basic fltk events
////# define SR_USE_TRACE2  // more fltk events
////# define SR_USE_TRACE3  // sr translated events
////# define SR_USE_TRACE4  // view_all
////# define SR_USE_TRACE5  // timeout
//# include <SR/sr_trace.h>

//=============================== srSaSetShapesChanged ===========================================

class srSaSetShapesChanged : public SrSa
 { public :
    virtual bool shape_apply ( SrSnShapeBase* s ) { s->changed(true); return true; }
 };

//================================= help window ===================================================

fltk::Window* make_help_window ()
 {
   fltk::Window* win = new fltk::Window ( 300, 200, "FltkViewer Help" );
   win->set_non_modal();
   return win;
 }

fltk::Browser* make_help_browser ()
 {
  fltk::Browser* b = new fltk::Browser ( 5, 5, 290, 190 );

   b->add ( "Left Click: Select\n" );
   b->add ( "Left Click + ALT: Rotate\n" );
   b->add ( "Middle Click + ALT: Translate\n" );
   b->add ( "Right Click + ALT: Zoom\n" );
   b->add ( "Left Click + Alt: Y Rotation\n" );
   b->add ( "Ctrl + Shift + m: Menu\n" );
   b->add ( "Ctrl + Shift + x: Exit\n" );
   b->add ( "Ctrl + Shift + c: Print camera\n" );
   b->add ( "Ctrl + Shift + e: EPS export (2D only)\n" );
   b->add ( "Ctrl + Shift + s: Snapshot current frame\n" );
   b->add ( "Ctrl + Shift + a: All frames snapshot on/off\n" );
   b->add ( "Ctrl + Shift + o: Export all models in scene\n" );

   return b;
 }

//================================= popup menu ===================================================

static void menucb ( fltk::Widget* o, void* v ) 
 {
	 fltk::Widget* widget = o->parent();
	 while (widget && widget->parent() != NULL)
		 widget = widget->parent();
	 FltkViewer* viewer = dynamic_cast<FltkViewer*>(widget);
	 if (viewer)
		 viewer->menu_cmd((FltkViewer::MenuCmd)(int)v);
 }

# define MCB     ((fltk::Callback*)menucb)
# define CMD(c)  ((void*)FltkViewer::c)

static Fl_Menu_Item MenuTable[] =
 { 
   { "&help",       0, MCB, CMD(CmdHelp) },
   { "&view all",   0, MCB, CMD(CmdViewAll) },
   { "&background", 0, MCB, CMD(CmdBackground) }, // FL_MENU_DIVIDER

   { "&mode", 0, 0, 0, FL_SUBMENU },
         { "&examiner", 0, MCB, CMD(CmdExaminer), FL_MENU_RADIO },
         { "&planar",   0, MCB, CMD(CmdPlanar),   FL_MENU_RADIO },
         { 0 },
   { "&draw style", 0, 0, 0, FL_SUBMENU },
         { "&as is",   0, MCB, CMD(CmdAsIs),    FL_MENU_RADIO },
         { "d&efault", 0, MCB, CMD(CmdDefault), FL_MENU_RADIO },
         { "&smooth",  0, MCB, CMD(CmdSmooth),  FL_MENU_RADIO },
         { "&flat",    0, MCB, CMD(CmdFlat),    FL_MENU_RADIO },
         { "&lines",   0, MCB, CMD(CmdLines),   FL_MENU_RADIO },
         { "&points",  0, MCB, CMD(CmdPoints),  FL_MENU_RADIO },
         { 0 },
  { "&characters", 0, 0, 0, FL_SUBMENU },
         { "&geometry", 0, MCB, CMD(CmdCharacterShowGeometry), FL_MENU_RADIO },
         { "&collision geometry", 0, MCB, CMD(CmdCharacterShowCollisionGeometry),   FL_MENU_RADIO },
         { "&deformable geometry", 0, MCB, CMD(CmdCharacterShowDeformableGeometry),   FL_MENU_RADIO },
         { "&bones",   0, MCB, CMD(CmdCharacterShowBones),   FL_MENU_RADIO },
         { "&axis",   0, MCB, CMD(CmdCharacterShowAxis),   FL_MENU_RADIO },
         { 0 },
   { "p&references", 0, 0, 0, FL_SUBMENU },
         { "&axis",         0, MCB, CMD(CmdAxis),        FL_MENU_TOGGLE },
         { "b&ounding box", 0, MCB, CMD(CmdBoundingBox), FL_MENU_TOGGLE },
         { "&statistics",   0, MCB, CMD(CmdStatistics),  FL_MENU_TOGGLE },
         { "spi&n anim",    0, MCB, CMD(CmdSpinAnim),    FL_MENU_TOGGLE },
         { 0 },
   { 0 }
 };

# undef CMD
# undef MCB

// need to set/get data to be able to share the same popup menu with many instances of viewers

static void set_menu_data ( FltkViewer::ViewMode v, FltkViewer::RenderMode r, FltkViewer::CharacterMode c,
                            bool axis, bool bbox, bool stat, bool spin )
 {
   # define SET(i,b)  if(b) MenuTable[i].set(); else MenuTable[i].clear();
   # define SETO(i)   MenuTable[i].setonly();
   # define CMD(i)    ((int)(MenuTable[i].user_data_))

   int i=0;
   while ( CMD(i)!=FltkViewer::CmdExaminer ) i++;      SETO (  i+(int)v );
   while ( CMD(i)!=FltkViewer::CmdAsIs ) i++;          SETO (  i+(int)r );
   while ( CMD(i)!=FltkViewer::CmdCharacterShowGeometry ) i++; SETO (  i+(int)c );
   while ( CMD(i)!=FltkViewer::CmdAxis ) i++;          SET  ( i, axis );
   while ( CMD(i)!=FltkViewer::CmdBoundingBox ) i++;   SET  ( i, bbox );
   while ( CMD(i)!=FltkViewer::CmdStatistics ) i++;    SET  ( i, stat );
   while ( CMD(i)!=FltkViewer::CmdSpinAnim ) i++;      SET  ( i, spin );

   # undef CMD
   # undef SETO
   # undef SET
 }

//======================================= timeout ===================================

// The timeout is called by fltk even when the app is iconized.
static void spin_timeout_func ( void* udata ) 
 {
   SrQuat delta;
   double interval;
   double activation;

   ////SR_TRACE5 ( "TIMOUT FUNC\n" );

   FltkViewer* v = (FltkViewer*)udata;
   v->get_spin_data ( delta, interval, activation );

//   if ( !v->root() && !v->menu_cmd_activated(FltkViewer::CmdAxis) )
//     v->spinning ( false );

   if ( v->spinning() && !v->iconized() ) 
    { v->increment_model_rotation ( delta );
      v->redraw ();
	  fltk::repeat_timeout( float(interval), spin_timeout_func, udata );
    }

   v->spin_animation_occured ();
 }

//================================= Internal Structures =================================

struct SpinData
 { SrQuat rotdelta;    // spin current rotation delta used for the spinning animation
   double lasttime;    // last spin time
   double activation;  // spin activation
   double interval;    // spin interval
   SpinData ()  { lasttime=0; init(); }
   void init () { activation=0.1f; interval=0.01f; rotdelta=SrQuat::null; }
   void set_interval ( double i ) { interval = i<0.01? 0.01:i; }
   void set_activation ( double a ) { activation = a<0.01? 0.01:a; }
 };

class FltkViewerData
 { public :
   SrSn*  root;              // contains the user scene
   FltkViewer::ViewMode viewmode;     // viewer mode, initially Examiner
   FltkViewer::RenderMode rendermode; // render mode
   FltkViewer::CharacterMode charactermode; // render mode


   bool iconized;      // to stop processing while the window is iconized
   bool spinning;      // indicates if the model is currently spinning
   bool allowspinanim; // allows spin animation or not
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

   SrString message;   // user msg to display in the window
   SrLight light;

   int spin_inc_count;    // counts increments applied to spin animation (for normalization)
   SpinData   spindata;   // Data for spin animation
   SrTrackball trackball; // To process spin rotations
   SrTimer    fcounter;   // To count frames and measure frame rate
   SrEvent    event;      // The translated event from fltk to sr format
   SrColor    bcolor;     // Background color currently used
   SrBox      bbox;       // Bounding box of the root, calculated with viewall
   SrCamera   camera;     // The current camera parameters

   SrSnLines* scenebox;  // contains the bounding box to display, and use in view_all
   SrSnLines* sceneaxis; // the current axis being displayed

   fltk::PopupMenu* menubut; // the ctrl+shift+m or button3 menu
   fltk::Window* helpwin;
   fltk::Browser* helpbrowser;

   SrSaGlRender render_action;
   SrSaBBox bbox_action;
 };

//===================================== FltkViewer =================================

// Called when the small "cross" button to close the window is pressed
static void _callback_func ( fltk::Widget* win, void* pt )
 {
   //LOG("DBG callback_func!\n");
   FltkViewer* v = (FltkViewer*)pt;
   v->close_requested ();
 }

FltkViewer::FltkViewer ( int x, int y, int w, int h, const char *label )
         : SrViewer(x, y, w, h) , fltk::GlWindow ( x, y, w, h, label )
 {
   fltk::glVisual( fltk::RGB | fltk::DOUBLE_BUFFER | fltk::DEPTH_BUFFER );//| FL_ALPHA );

   callback ( _callback_func, this );

   resizable(this);

   _data = new FltkViewerData;

   _data->root = new SrSnGroup; // we maintain root pointer always valid
   _data->viewmode = ModeExaminer;
   _data->rendermode = ModeAsIs;
   _data->charactermode = ModeShowGeometry;

   _data->iconized    = false;
   _data->spinning    = false;
   _data->allowspinanim = true;
   _data->statistics  = false;
   _data->displayaxis = false;
   _data->boundingbox = false;
   _data->scene_received_event = false;
   _data->showgeometry = true;
   _data->showcollisiongeometry = false;
   _data->showbones = false;
   _data->showaxis = false;

   _data->light.init();

   _data->bcolor = SrColor(.63f, .63f, .63f);
   _data->spin_inc_count = 0;

   _data->scenebox = new SrSnLines;
   _data->sceneaxis = new SrSnLines;

   user_data ( (void*)(this) ); // to be retrieved by the menu callback
   
   begin();
   _data->menubut = new fltk::PopupMenu(0,0,50,50);
   _data->menubut->type(fltk::PopupMenu::POPUP23);
   _data->menubut->menu(MenuTable);
   _data->menubut->textsize(12);
   _data->helpwin = make_help_window ();
   _data->helpbrowser = make_help_browser ();
   end();

   gridColor[0] = .5;
   gridColor[1] = .5;
   gridColor[2] = .5;
   gridHighlightColor[0] = .0;
   gridHighlightColor[1] = .0;
   gridHighlightColor[2] = .0;
   gridSize = 200.0;
   gridStep = 20.0;
   gridList = -1;
 }

FltkViewer::~FltkViewer ()
 {
   fltk::remove_timeout ( spin_timeout_func, this );
   _data->root->unref ();
   if (_data->helpbrowser)
	   delete _data->helpbrowser;
   delete _data->helpwin;
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
	 set_menu_data ( _data->viewmode, _data->rendermode, _data->charactermode, _data->displayaxis,
                   _data->boundingbox, _data->statistics, _data->allowspinanim );
   _data->menubut->popup();
 }

void FltkViewer::menu_cmd ( MenuCmd s )
 {
	 bool applyToCharacter = false;

   switch ( s )
    { case CmdHelp : _data->helpwin->show(); _data->helpwin->active(); break;

      case CmdViewAll : view_all (); break;

      case CmdExaminer : _data->viewmode = ModeExaminer; 
                         update_axis(); spinning(false); view_all();
                         break;
      case CmdPlanar   : _data->viewmode = ModePlanar;
                         update_axis(); spinning(false); view_all();
                         break;
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

      case CmdBoundingBox : SR_SWAPB(_data->boundingbox); 
                            if ( _data->boundingbox ) update_bbox();
                            break;

      case CmdStatistics : SR_SWAPB(_data->statistics); break;
      case CmdSpinAnim   : SR_SWAPB(_data->allowspinanim); 
                           if (!_data->allowspinanim) _data->spinning=false;
                           break;
	  case CmdCharacterShowGeometry:
						_data->showgeometry = true;
						_data->showcollisiongeometry = false;
						_data->showdeformablegeometry = false;
						_data->showbones = false;
						_data->showaxis = false;
						applyToCharacter = true;
						break;
	  case CmdCharacterShowCollisionGeometry: 
						_data->showgeometry = false;
						_data->showcollisiongeometry = true;
						_data->showdeformablegeometry = false;
						_data->showbones = false;
						_data->showaxis = false;
						applyToCharacter = true;
						break;
	  case CmdCharacterShowDeformableGeometry: 
						_data->showgeometry = false;
						_data->showcollisiongeometry = false;
						_data->showdeformablegeometry = true;
						_data->showbones = false;
						_data->showaxis = false;
						applyToCharacter = true;
						break;
	  case CmdCharacterShowBones: 
						_data->showgeometry = false;
						_data->showcollisiongeometry = false;
						_data->showdeformablegeometry = false;
						_data->showbones = true;
						_data->showaxis = false;
						applyToCharacter = true;
						break;
	  case CmdCharacterShowAxis: 
						_data->showgeometry = false;
						_data->showcollisiongeometry = false;
						_data->showdeformablegeometry = false;
						_data->showbones = false;
						_data->showaxis = true;
						applyToCharacter = true;
						break;
    }
	
	if (applyToCharacter)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		srHashMap<SbmCharacter>& character_map = mcu.character_map;
		character_map.reset();
		SbmCharacter* character = character_map.next();
		while ( character )
		{	
			// set the visibility parameters of the scene
			character->scene_p->set_visibility(_data->showbones,_data->showgeometry, _data->showcollisiongeometry, _data->showaxis);
			character->dMesh_p->set_visibility(_data->showdeformablegeometry);
			character = character_map.next();
		}
						
	}

   render ();
 }

bool FltkViewer::menu_cmd_activated ( MenuCmd c )
 {
   switch ( c )
    { case CmdExaminer : return _data->viewmode==ModeExaminer? true:false;
      case CmdPlanar   : return _data->viewmode==ModePlanar? true:false;

      case CmdAsIs     : return _data->rendermode==ModeAsIs? true:false;
      case CmdDefault  : return _data->rendermode==ModeDefault? true:false;
      case CmdSmooth   : return _data->rendermode==ModeSmooth? true:false;
      case CmdFlat     : return _data->rendermode==ModeFlat? true:false;
      case CmdLines    : return _data->rendermode==ModeLines? true:false;
      case CmdPoints   : return _data->rendermode==ModePoints? true:false;

      case CmdAxis        : return _data->displayaxis? true:false;
      case CmdBoundingBox : return _data->boundingbox? true:false;
      case CmdStatistics  : return _data->statistics? true:false;
      case CmdSpinAnim    : return _data->allowspinanim? true:false;
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

   if ( _data->viewmode==ModePlanar )
    _data->sceneaxis->shape().push_axis ( SrPnt::null, len, 2, "xy" );
   else
    _data->sceneaxis->shape().push_axis ( SrPnt::null, len, 3, "xyz" );
 }

void FltkViewer::view_all ()
 {
   _data->spindata.init ();
   _data->spinning = false;

   _data->camera.center = SrVec::null;
   _data->camera.up = SrVec::j;
   _data->camera.eye.set ( 0, 0, 1.0f );

   _data->trackball.init ();

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
   if ( !_data->spinning ) redraw(); 
 } 

bool FltkViewer::iconized () 
 { 
   return _data->iconized;
 }

bool FltkViewer::spinning ()
 { 
   return _data->spinning;
 }

void FltkViewer::set_spin_data ( const SrQuat &delta, float interval, float activation )
 { 
   SpinData &s = _data->spindata;
   s.set_interval ( interval );
   s.rotdelta = delta;
   s.set_activation ( activation );
 }

void FltkViewer::get_spin_data ( SrQuat &delta, double &interval, double &activation )
 {
   SpinData &s = _data->spindata;
   delta = s.rotdelta;
   interval = s.interval;
   activation = s.activation;
 }

void FltkViewer::spinning ( bool onoff )
 { 
   _data->spinning=onoff;
   if ( _data->spinning )
	   fltk::add_timeout( float(_data->spindata.interval), spin_timeout_func, (void*)this );
 }

void FltkViewer::allow_spin_animation ( bool b )
 { 
   _data->allowspinanim = b;
 }

void FltkViewer::increment_model_rotation ( const SrQuat &dq )
 {
   const SrQuat& rotation = _data->trackball.rotation;
   _data->camera *= rotation.inverse() * dq.inverse() * rotation;
   _data->trackball.increment_rotation ( dq );

   if ( _data->spin_inc_count++%300==0 )
    { _data->trackball.rotation.normalize();
      _data->trackball.last_spin.normalize();
      _data->spin_inc_count = 0;
    }
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

FltkViewer::ViewMode FltkViewer::get_view_mode ()
 {
   return _data->viewmode;
 }

void FltkViewer::get_camera ( SrCamera &cam )
 {
   cam = _data->camera;
 }

void FltkViewer::set_camera ( const SrCamera &cam )
 {
   _data->camera = cam;

   if ( _data->viewmode==ModeExaminer )
    { _data->trackball.init();
      _data->trackball.rotation.set ( cam.eye-cam.center, SrVec::k );
    }

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
   fltk::setfont(fltk::TIMES, 12 ); // from fltk
   fltk::drawtext(s, x, y );      // from fltk
 }

//-- Render  ------------------------------------------------------------------

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
 }

void FltkViewer::close_requested ()
 {
   exit ( 0 );
 }

//# include <SR/sr_sphere.h>
//static SrSnSphere* SPH=0;
//   if ( !SPH ) SPH = new SrSnSphere;
//   SPH->shape().center = light.position;
//   SPH->shape().radius = 0.6f;
//   SPH->color ( SrColor::red );
//   _data->render_action.apply ( SPH );
   
void FltkViewer::draw() 
 {
   if ( !visible() ) return;
   if ( !valid() ) init_opengl ( w(), h() ); // valid() is turned on by fltk after draw() returns
//   sr_out<<"DRAW\n";
 
   SrLight &light = _data->light;
   SrCamera &cam  = _data->camera;
   SrMat mat ( SrMat::NotInitialized );

   light.directional = false;
   light.position = cam.eye;
   light.constant_attenuation = 1.0f/cam.scale;

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
   glLight ( 0, light );
   glScalef ( cam.scale, cam.scale, cam.scale );
//   glRotate ( _model_rotation );

   // draw the grid
   if (gridList == -1)
	   initGridList();
   drawGrid();

   //----- Render user scene -------------------------------------------
   //glClearColor ( _data->bcolor );
   //glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   _data->fcounter.start();
   if ( _data->displayaxis ) _data->render_action.apply ( _data->sceneaxis );
   if ( _data->boundingbox ) _data->render_action.apply ( _data->scenebox );
   if ( _data->root ) _data->render_action.apply ( _data->root );
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

// fltk::event_x/y() variates from (0,0) to (w(),h())
// transformed coords in SrEvent are in "normalized device coordinates" [-1,-1]x[1,1]
static void translate_event ( SrEvent& e, SrEvent::Type t, int w, int h, FltkViewer* viewer )
 {
   e.init_lmouse ();

   e.type = t;

   // put coordinates inside [-1,1] with (0,0) in the middle :
   e.mouse.x  = ((float)fltk::event_x())*2.0f / ((float)w) - 1.0f;
   e.mouse.y  = ((float)fltk::event_y())*2.0f / ((float)h) - 1.0f;
   e.mouse.y *= -1.0f;
   e.width = w;
   e.height = h;
   e.mouseCoord.x = fltk::event_x();
   e.mouseCoord.y = fltk::event_y();

   if ( t==SrEvent::Push)
   {
	   e.button = fltk::event_button();
	   e.origUp = viewer->getData()->camera.up;
	   e.origEye = viewer->getData()->camera.eye;
	   e.origCenter = viewer->getData()->camera.center;
	   e.origMouse.x = e.mouseCoord.x;
	   e.origMouse.y = e.mouseCoord.y;
   }
   else if (t==SrEvent::Release )
   {
	   e.button = fltk::event_button();
	   e.origMouse.x = -1;
	   e.origMouse.y = -1;
   }


   if ( fltk::event_state(fltk::BUTTON1) ) e.button1 = 1;
   if ( fltk::event_state(fltk::BUTTON2) ) e.button2 = 1;
   if ( fltk::event_state(fltk::BUTTON3) ) e.button3 = 1;

   if ( fltk::event_state(fltk::ALT)   ) e.alt = 1;
   if ( fltk::event_state(fltk::CTRL)  ) e.ctrl = 1;

   if ( fltk::event_state(fltk::SHIFT) ) e.shift = 1;
   
   e.key = fltk::event_key();




 }

static float rps = 1.0f;
static int x_flag = 1;
static int z_flag = 1;
static int rps_flag = 0;
static float spd;
static float x_spd = 7;
static float z_spd = 70;
static char t_direction[200];
static char character[100];
static int char_index = 0;

static void translate_keyboard_event ( SrEvent& e, SrEvent::Type t, int w, int h)
{
	e.type = t;
	bool not_locomotion = false;
	e.key = fltk::event_key();
	char cmd[300];
	cmd[0] = '\0';
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SbmCharacter* actor = NULL;
	mcu.character_map.reset();
	for(int i = 0; i <= char_index; ++i)
	{
		actor = mcu.character_map.next();
		sprintf(character, "char %s ", actor->name);
	}

	sprintf(cmd, "test loco ");

	// locomotion control
	switch (e.key)
	{
	case fltk::UpKey: //move forward
		rps_flag = 0;
		z_flag = 1;
		x_flag = 0;
		spd = z_spd;
		sprintf(t_direction, "forward ");
		break;

    case fltk::DownKey://move back
		z_flag = -1;
		x_flag = 0;
		rps_flag = 0;
		spd = z_spd;
		sprintf(t_direction, "backward ");
		break;

	case fltk::LeftKey://turn left
		rps_flag = -1;
		break;

	case 'x':
		++char_index;
		if(char_index >= mcu.character_map.get_num_entries())
		{
			char_index = 0;
		}
		not_locomotion = true;
		break;

	case fltk::RightKey://turn right
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
	sprintf(tt, "spd %f rps %f time 2.0", spd, rps_flag * rps);
	if(not_locomotion == false) 
	{
		strcat(cmd, tt);
		//printf("\n%s", cmd);
		mcu.execute(cmd);
	}
}



int FltkViewer::handle ( int event ) 
 {
   # define POPUP_MENU(e) e.ctrl && e.button3

   SrEvent &e = _data->event;
   e.type = SrEvent::None;

   switch ( event )
   { case fltk::PUSH:
       { //SR_TRACE1 ( "Mouse Push : but="<<fltk::event_button()<<" ("<<fltk::event_x()<<", "<<fltk::event_y()<<")" <<" Ctrl:"<<fltk::event_state(FL_CTRL) );
         translate_event ( e, SrEvent::Push, w(), h(), this );
         if ( POPUP_MENU(e) ) { show_menu(); e.type=SrEvent::None; }
          else _data->spinning=false; 
       } break;

      case fltk::RELEASE:
        //SR_TRACE1 ( "Mouse Release : ("<<fltk::event_x()<<", "<<fltk::event_y()<<") buts: "
        //             <<(fltk::event_state(fltk::BUTTON1)?1:0)<<" "<<(fltk::event_state(fltk::BUTTON2)?1:0) );
        translate_event ( e, SrEvent::Release, w(), h(), this);
        break;

      case fltk::MOVE:
        //SR_TRACE2 ( "Move buts: "<<(fltk::event_state(FL_BUTTON1)?1:0)<<" "<<(fltk::event_state(FL_BUTTON2)?1:0) );
        if ( !fltk::event_state(fltk::BUTTON1) && !fltk::event_state(fltk::BUTTON2) ) break;
        // otherwise, this is a drag: enter in the drag case.
        // not sure if this is a hack or a feature.
      case fltk::DRAG:
        //SR_TRACE2 ( "Mouse Drag : ("<<fltk::event_x()<<", "<<fltk::event_y()<<") buts: "
        //             <<(fltk::event_state(FL_BUTTON1)?1:0)<<" "<<(fltk::event_state(FL_BUTTON2)?1:0) );
        translate_event ( e, SrEvent::Drag, w(), h(), this );
        break;

      case fltk::SHORTCUT: // not sure the relationship between a shortcut and keyboard event...
        //SR_TRACE1 ( "Shortcut : "<< fltk::event_key() <<" "<<fltk::event_text() );
        //translate_event ( e, SrEvent::Keyboard, w(), h() );
        //break;

	  case fltk::KEY:
        //SR_TRACE1 ( "Key Pressed : "<< fltk::event_key() <<" "<<fltk::event_text() );
        translate_keyboard_event ( e, SrEvent::Keyboard, w(), h());
        break;

//      case fltk::KEYBOARD:
        //SR_TRACE1 ( "Key Pressed : "<< fltk::event_key() <<" "<<fltk::event_text() );
//        translate_event ( e, SrEvent::Keyboard, w(), h() );
     //   break;

      case fltk::HIDE: // Called when the window is iconized
        { //SR_TRACE1 ( "Hide" );
          _data->iconized = true;
          // the opengl lists need to be re-created when the window appears again, so
          // we mark already here all shapes as changed:
          _data->scenebox->changed(true);
          _data->sceneaxis->changed(true);
          srSaSetShapesChanged sa;
          sa.apply ( _data->root );
        } break;

      case fltk::SHOW: // Called when the window is de-iconized or when show() is called
        //SR_TRACE1 ( "Show" );
        _data->iconized = false;
		if ( _data->spinning ) fltk::add_timeout( float(_data->spindata.interval), spin_timeout_func, (void*)this );
        show ();
        break;

      // Other events :
      case fltk::ENTER:          
		  //SR_TRACE2 ( "Enter" );         
		  break;
      case fltk::LEAVE:          
		  //SR_TRACE2 ( "Leave" );         
		  break;
      case fltk::FOCUS:          
		  //SR_TRACE2 ( "Focus" );         
		  break;
      case fltk::UNFOCUS:        
		  //SR_TRACE2 ( "Unfocus" );       
		  break;
     // case FL_CLOSE:          
		  //SR_TRACE2 ( "Close");          
	//	  break;
      case fltk::ACTIVATE:       
		  //SR_TRACE2 ( "Activate");       
		  break;
      case fltk::DEACTIVATE:     
		  //SR_TRACE2 ( "Deactivate");     
		  break;
      case fltk::PASTE:          
		  //SR_TRACE2 ( "Paste");          
		  break;
 //     case FL_SELECTIONCLEAR: 
		  //SR_TRACE2 ( "SelectionClear"); 
	//	  break;
    }

   //SR_TRACE3 ( e );

   if ( e.type == SrEvent::None ) return 0; // not an interesting event

   if ( event==fltk::PUSH || event==fltk::DRAG )
    { SrPlane plane ( _data->camera.center, SrVec::k );
      _data->camera.get_ray ( e.mouse.x, e.mouse.y, e.ray.p1, e.ray.p2 );
      _data->camera.get_ray ( e.lmouse.x, e.lmouse.y, e.lray.p1, e.lray.p2 );
      e.mousep = plane.intersect ( e.ray.p1, e.ray.p2 );
      e.lmousep = plane.intersect ( e.lray.p1, e.lray.p2 );
	  if ( event==fltk::PUSH  ) // update picking precision
       { // define a and b with 1 pixel difference:
         SrPnt2 a ( ((float)w())/2.0f, ((float)h())/2.0f ); // ( float(fltk::event_x()), float(fltk::event_y()) );
         SrPnt2 b (a+SrVec2::one);// ( float(fltk::event_x()+1), float(fltk::event_y()+1) );
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
      if ( _data->viewmode==ModeExaminer )
        res = handle_examiner_manipulation ( e );
      else if ( _data->viewmode==ModePlanar )
        res = handle_planar_manipulation ( e );

      if ( res ) return res;
    }

   if ( e.mouse_event() ) return handle_scene_event ( e );

   if ( e.type == SrEvent::Keyboard )
    { if ( handle_keyboard(e)==0 ) res = handle_scene_event ( e );
      if ( res==0 && e.key==SrEvent::KeyEsc ) res=1; // to avoid exiting with ESC
    }

   return res; // this point should not be reached
 }

//== Examiner ==============================================================

# define ROTATING2(e)    (e.alt && e.button1)
# define ROTATING(e)   (e.alt && e.shift && e.button1)
# define ZOOMING(e)   (e.alt && e.button3)
//# define ZOOMING(e)     (e.alt && e.ctrl && e.button3)
//# define DOLLYING(e)     (e.alt && e.button3)
# define TRANSLATING(e) (e.alt && e.button2)

SrVec rotatePoint(SrVec point, SrVec origin, SrVec direction, float angle)
{
	float originalLength = point.len();

	SrVec v = direction;
	SrVec o = origin;
	SrVec p = point;
	float c = cos(angle);
	float s = sin(angle);
	float C = 1.0f - c;

	SrMat mat;
	mat.e11() = v[0] * v[0] * C + c;
	mat.e12() = v[0] * v[1] * C - v[2] * s;
	mat.e13() = v[0] * v[2] * C + v[1] * s;
	mat.e21() = v[1] * v[0] * C + v[2] * s;
	mat.e22() = v[1] * v[1] * C + c;
	mat.e23() = v[1] * v[2] * C - v[0] * s;
	mat.e31() = v[2] * v[0] * C - v[1] * s;
	mat.e32() = v[2] * v[1] * C + v[0] * s;
	mat.e33() = v[2] * v[2] * C + c;

	mat.transpose();

	SrVec result = origin + mat * (point - origin);

	return result;
}

int FltkViewer::handle_examiner_manipulation ( const SrEvent &e )
 {
   if ( e.type==SrEvent::Drag )
    { 
      float dx = e.mousedx() * _data->camera.aspect;
      float dy = e.mousedy() / _data->camera.aspect;

      if ( ZOOMING(e) )
       { _data->camera.fovy += (dx+dy);//40.0f;
         _data->camera.fovy = SR_BOUND ( _data->camera.fovy, 0.001f, srpi );
       }
	/*  else if ( DOLLYING(e) )
       { 
		    float amount = .2f;
			SrVec cameraPos(_data->camera.eye);
			SrVec targetPos(_data->camera.center);
			SrVec diff = targetPos - cameraPos;
			diff.normalize();
			
			float virtualSize = ;

			SrVec oldCameraEye = _data->camera.eye;
			_data->camera.eye = cameraPos;
			_data->camera.center += (_data->camera.eye - oldCameraEye);
       }
	   */
      else if ( TRANSLATING(e) )
       { _data->camera.apply_translation_from_mouse_motion ( e.lmouse.x, e.lmouse.y, e.mouse.x, e.mouse.y );
       }
      else if ( ROTATING(e) )
       { SrQuat drot;
         SrTrackball::get_spin_from_mouse_motion ( e.lmouse.x, e.lmouse.y, e.mouse.x, e.mouse.y, drot );
         increment_model_rotation ( drot );
        _data->spindata.lasttime = _data->fcounter.time();
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
	  }
    }
   else if ( e.type==SrEvent::Release )
    { if ( e.button==1 && _data->allowspinanim && _data->spindata.lasttime>0 ) 
       { _data->spindata.set_interval ( _data->fcounter.time()-_data->spindata.lasttime );
         _data->spindata.rotdelta = _data->trackball.last_spin;
         //SR_TRACE5 ( _data->spindata.interval<<" < "<<_data->spindata.activation );
         if ( _data->spindata.interval<_data->spindata.activation )
          { _data->spinning=true;
			fltk::add_timeout ( float(_data->spindata.interval), spin_timeout_func, (void*)this );
          }
       }
    }
   return 1;
 }

//== Planar =============================================================

int FltkViewer::handle_planar_manipulation ( const SrEvent& e )
 {
   SrCamera& c = _data->camera;

   if ( e.type==SrEvent::Drag )
    { 
      float dx = e.mousedx() * c.aspect;
      float dy = e.mousedy() / c.aspect;

      if ( ZOOMING(e) ) // scaling effect in planar mode
       { c.fovy += (dx+dy)*2;
         c.fovy = SR_BOUND ( c.fovy, 0.01f, srpi );
       }
      else if ( TRANSLATING(e) ) // this will translate in planar mode
       { _data->camera.apply_translation_from_mouse_motion ( e.lmouse.x, e.lmouse.y, e.mouse.x, e.mouse.y );
       }
      else if ( ROTATING(e) ) // planar mode
       { 
         SrQuat drot;
         SrPnt p1 ( e.lmousep.x, e.lmousep.y, 0 );
         SrPnt p2 ( e.mousep.x, e.mousep.y, 0 );
         drot.set ( p1, p2 );
         c.up = c.up * drot;
       }

      redraw();
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

int FltkViewer::handle_keyboard ( const SrEvent &e )
 {
   if ( e.ctrl && e.shift )
    { switch ( e.key )
       { case 'm' : show_menu (); return 1;
         case 'x' : hide (); return 1;
	   }
    }
   return 0;
 }

//== Spin Animation ========================================================

void FltkViewer::spin_animation_occured ()
{
}

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
	glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
	bool colorChanged = false;
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);

	glColor3f(gridColor[0], gridColor[1], gridColor[2]);			
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glLineWidth(1);
//	glLineStipple(1, 0xAAAA);
	glBegin(GL_LINES);
	for (float x = -gridSize; x <= gridSize; x += gridStep)
	{
		if (x == 0.0) {
			colorChanged = true;
			glColor3f(gridHighlightColor[0], gridHighlightColor[1], gridHighlightColor[2]);
		}
		glVertex3f(x, 0.0, -gridSize);
		glVertex3f(x, 0.0, gridSize);
		
		if (colorChanged) {
			colorChanged = false;
			glColor3f(gridColor[0], gridColor[1], gridColor[2]);
		}

	}
	for (float x = -gridSize; x <= gridSize; x += gridStep)
	{
		if (x == 0) {
			colorChanged = true;
			glColor3f(gridHighlightColor[0], gridHighlightColor[1], gridHighlightColor[2]);
		}
		glVertex3f(-gridSize, 0.0, x);
		glVertex3f(gridSize, 0.0, x);
		if (colorChanged) {
			colorChanged = false;
			glColor3f(gridColor[0], gridColor[1], gridColor[2]);
		}
	}

	glEnd();
	glDisable(GL_BLEND);
//	glDisable(GL_LINE_STIPPLE);

	glPopAttrib();
}


//== Viewer Factory ========================================================


FltkViewer* FltkViewerFactory::s_viewer = NULL;

FltkViewerFactory::FltkViewerFactory()
{
	s_viewer = NULL;
}

SrViewer* FltkViewerFactory::create(int x, int y, int w, int h)
{
	if (!s_viewer)
		s_viewer = new FltkViewer(x, y, w, h);
	return s_viewer;
}

//================================ End of File =================================================
