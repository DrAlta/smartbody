/*  sr_sa_gl_render.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmarBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently UC Merced)
 */

# include <sr/sr_gl.h>
# include <sr/sr_sn.h>
# include <sr/sr_gl_render_funcs.h>
# include <sr/sr_sa_render_mode.h>
# include <sr/sr_sa_gl_render.h>

//# define SR_USE_TRACE1 // constructor / destructor
//# define SR_USE_TRACE2 // render
//# define SR_USE_TRACE3 // matrix
//# include <sr/sr_trace.h>

//=============================== SrOGLData ====================================

struct SrOGLData : public SrSnShapeBase::RenderLibData
 { public :
    unsigned list;
    SrSaGlRender::render_function rfunc;
   public :
    SrOGLData ( SrSnShapeBase* s );
   ~SrOGLData () { glDeleteLists(list,1); }
 };

SrOGLData::SrOGLData ( SrSnShapeBase* s )
 {
   int i;
   SrArray<SrSaGlRender::RegData>& rfuncs = SrSaGlRender::_rfuncs;

   //SR_TRACE2 ( "Initializing render data for "<<s->inst_class_name() );
   s->changed ( true );
   s->set_renderlib_data ( this );

   // we could use here a sorted array, but as the number of registered classes
   // is normally low (<10), the overhead is not worth.
   for ( i=0; i<rfuncs.size(); i++ )
     if ( sr_compare(rfuncs[i].class_name,s->inst_class_name())==0 ) break;
   if ( i==rfuncs.size() )
     sr_out.fatal_error("SrSaGlRender: shape [%s] not registered!",s->inst_class_name());

   rfunc = rfuncs[i].rfunc;
   list = glGenLists ( 1 ); // creates one list
// sr_out<<"is list: "<<glIsList(list)<<srnl;
 }

//=============================== SrSaGlRender ====================================

SrArray<SrSaGlRender::RegData> SrSaGlRender::_rfuncs;

void register_render_function ( const char* class_name, SrSaGlRender::render_function rfunc ) // friend
 {
   SrArray<SrSaGlRender::RegData> &rf = SrSaGlRender::_rfuncs;

   int i;
   for ( i=0; i<rf.size(); i++ ) 
    { if ( sr_compare(rf[i].class_name,class_name)==0 ) break;
    }
   if ( i==rf.size() ) rf.push(); // if not found, push a new position

   rf[i].class_name = class_name;
   rf[i].rfunc = rfunc;
 }

SrSaGlRender::SrSaGlRender () 
 { 
   //SR_TRACE1 ( "Constructor" );
   if ( _rfuncs.size()==0 ) // no functions registered
    { register_render_function ( "model",    SrGlRenderFuncs::render_model );
      register_render_function ( "lines",    SrGlRenderFuncs::render_lines );
      register_render_function ( "points",   SrGlRenderFuncs::render_points );
      register_render_function ( "box",      SrGlRenderFuncs::render_box );
      register_render_function ( "sphere",   SrGlRenderFuncs::render_sphere );
      register_render_function ( "cylinder", SrGlRenderFuncs::render_cylinder );
      register_render_function ( "polygon",  SrGlRenderFuncs::render_polygon );
      register_render_function ( "polygons", SrGlRenderFuncs::render_polygons );
    }
 }

SrSaGlRender::~SrSaGlRender ()
 {
   //SR_TRACE1 ( "Destructor" );
 }

void SrSaGlRender::restore_render_mode ( SrSn* n )
 {
   SrSaRenderMode a;
   a.apply ( n );
 }

void SrSaGlRender::override_render_mode ( SrSn* n, srRenderMode m )
 {
   SrSaRenderMode a(m);
   a.apply ( n );
 }

//==================================== virtuals ====================================

void SrSaGlRender::get_top_matrix ( SrMat& mat )
 {
   glGetFloatv ( GL_MODELVIEW_MATRIX, (float*)mat );
 }

int SrSaGlRender::matrix_stack_size ()
 {
   int value;
   glGetIntegerv ( GL_MODELVIEW_STACK_DEPTH, &value );
   return value;
 }

void SrSaGlRender::init_matrix ()
 {
   // we do nothing here as SrViewer has already defined
   // the camera/visualization matrix in the stack
 }

void SrSaGlRender::mult_matrix ( const SrMat& mat )
 {
   //SR_TRACE3("Mult Matrix");
   glMultMatrixf ( (const float*)mat );
 }

void SrSaGlRender::push_matrix ()
 {
   //SR_TRACE3("Push Matrix");
   glPushMatrix ();
 }

void SrSaGlRender::pop_matrix ()
 {
   //SR_TRACE3("Pop Matrix");
   glPopMatrix ();
 }

bool SrSaGlRender::shape_apply ( SrSnShapeBase* s )
 {
   // 1. Ensures that render data is initialized
   SrOGLData* ogl = (SrOGLData*) s->get_renderlib_data();
   if ( !ogl ) {	   
	   ogl = new SrOGLData ( s );
   }
   
   // 2. Render only if needed
   if ( !s->visible() ) return true;

  
   bool isSrModel = sr_compare(s->inst_class_name(),"model") == 0;
   // 3. Check if lists are up to date
   if ( !s->haschanged() && !isSrModel)
   { //SR_TRACE2 ( "Calling list..." );	  
      glCallList ( ogl->list ); 
      return true;
   }

   // 4. If the list is not up to date, regenerate it calling the render function
   //SR_TRACE2 ( "Creating and executing list of SrSn"<<s->inst_class_name()<<"..." );
  
   if (!isSrModel)
   {
	   glNewList ( ogl->list, GL_COMPILE_AND_EXECUTE );
	   ogl->rfunc ( s );
	   glEndList ();
	   s->changed(false);
   }
   else
   {
	   ogl->rfunc(s);
   }   

   return true;
 }

//======================================= EOF ====================================

