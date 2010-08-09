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

# include <fltk/GlWindow.H>
# include <sr/sr_viewer.h>
# include <sr/sr_color.h>

class SrQuat;
class SrEvent;
class SrCamera;
class SrSn;
class SrViewer;
class FltkViewerData;

/*! \class SrViewer sr_viewer.h
    \brief A fltk-opengl viewer

    SrViewer implements a viewer to render objects derived from SrSn.
    The viewer has currently a planar and a tridimensional examiner mode.
    In ModePlanar, only transformation on the XY plane are accepted.
    In all modes, mouse interaction is done together with Ctrl and Shift modifiers.
    A popup menu appears with a right button click or ctrl+shift+m. */
class FltkViewer : public SrViewer, public fltk::GlWindow 
 {
   public : // enumerators

    enum ViewMode { ModeExaminer,
                    ModePlanar
                  };

    enum RenderMode { ModeAsIs,
                      ModeDefault,
                      ModeSmooth,
                      ModeFlat,
                      ModeLines,
                      ModePoints
                    };
	enum CharacterMode { ModeShowGeometry,
                      ModeShowCollisionGeometry,
                      ModeShowBones,
                      ModeShowAxis
                    };

    enum MenuCmd { CmdHelp,
                   CmdViewAll,
                   CmdBackground,
                   CmdExaminer,   //!< examiner model manipulation
                   CmdPlanar,     //!< planar manipulation
                   CmdAsIs,
                   CmdDefault,
                   CmdSmooth,
                   CmdFlat,
                   CmdLines,
                   CmdPoints,
                   CmdAxis,
                   CmdBoundingBox,
                   CmdStatistics,
                   CmdSpinAnim,
				   CmdCharacterShowGeometry,
				   CmdCharacterShowCollisionGeometry,
				   CmdCharacterShowDeformableGeometry,
				   CmdCharacterShowBones,
				   CmdCharacterShowAxis,
                 };

   private : // internal data

    FltkViewerData *_data;

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
    void menu_cmd ( MenuCmd c );

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

    /*! Will make SrViewer to render the scene in the next fltk loop. If the
        current scene is spinning, then the scene is already being rendered by the
        spin animation timeout with a defined frequency, an then a call to render()
        will have no effect. */
    void render ();

    /*! Returns true if the window is iconized, false otherwise. */
    bool iconized ();

    /*! Returns true if the scene is spinning, false otherwise. When the scene is 
        spinning, then a spin animation timeout function is created in fltk that
        stays calling render() with a calculated frequency. */
    bool spinning () ;

    /*! Sets data to be considered by the spin animation. */
    void set_spin_data ( const SrQuat &delta, float interval, float activation=0.1f );

    /*! Gets the current spin animation data being used. */
    void get_spin_data ( SrQuat &delta, double &interval, double &activation );

    /*! Forces spin animation to start or stop. */
    void spinning ( bool onoff );

    void allow_spin_animation ( bool b );

    void increment_model_rotation ( const SrQuat &dq );

    float fps ();
    sruint curframe ();

    SrColor background ();
    void background ( SrColor c );

    ViewMode get_view_mode ();
    void get_camera ( SrCamera &cam );
    void set_camera ( const SrCamera &cam );

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

    /*! hande(int) controls the activation of the button menu, controls the
        spinning animation, and translates fltk events into the SrEvent class
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

    /*! Takes mouse events to rotate, scale and translate the scene in planar mode. 
        If the event is not used, it is passed to the scene by calling
        handle_scene_event(). */
    virtual int handle_planar_manipulation ( const SrEvent &e );
   
    /*! Applies an event action to the scene */
    virtual int handle_scene_event ( const SrEvent &e );

   /*! All keyboard events are passed to this method. The SrViewer
       implementation checks if crtl+shift+m is pressed to display
       the mouse menu, crtl+shift+x to exit the application, 
       crtl+shift+e to call the eps export action;
       otherwise it passes the event to the scene graph. */
    virtual int handle_keyboard ( const SrEvent &e );

    /*! Will be called each time a spin animation accured. The SrViewer
        implementation does nothing. */
    virtual void spin_animation_occured ();

	virtual void label_viewer(const char* str);
	virtual void show_viewer();
	virtual void hide_viewer();
 };


 class FltkViewerFactory : public SrViewerFactory
 {
	public:
		 FltkViewerFactory();

		virtual SrViewer* create(int x, int y, int w, int h);
 };

//================================ End of File =================================================

# endif // FLTK_VIEWER_H
