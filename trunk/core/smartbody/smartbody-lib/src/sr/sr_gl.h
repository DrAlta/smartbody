/*
 *  sr_gl.h - part of Motion Engine and SmartBody-lib
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

# ifndef SR_GL_H
# define SR_GL_H

/** \file sr_gl.h 
 * Sr wrapper and extensions for OpenGL
 *
 * Overload of most used OpenGL functions to directly work with SR types,
 * and some extra utilities.
 */

# include <sr/sr.h>

# ifdef SR_TARGET_WINDOWS // defined in sr.h
# include <Windows.h>
# endif

# ifdef __APPLE__
# include <OpenGL/gl.h>
# else
# include <GL/gl.h>
# endif

class SrVec;
class SrMat;
class SrQuat;
class SrColor;
class SrLight;
class SrOutput;
class SrMaterial;

//======================================= geometry ================================

void glNormal ( const SrVec &v );

void glVertex ( const SrVec &v );
void glVertex ( const SrVec &v1, const SrVec &v2 );
void glVertex ( const SrVec &v1, const SrVec &v2, const SrVec &v3 );
void glVertex ( const SrVec &v1, const SrVec &v2, const SrVec &v3, const SrVec &v4 );
void glVertex ( float x, float y, float z );
void glVertex ( float x, float y, float z, float a, float b, float c );
void glVertex ( float x, float y );
void glDrawBox ( const SrVec& a, const SrVec& b ); //!< Send quads with normals forming the box

//====================================== appearance ================================

void glClearColor ( const SrColor& c );
void glColor ( const SrColor& c );
void glLight ( int id, const SrLight& l, bool bind_pos = true ); //!< id = x E {0,...,7}, from GL_LIGHTx
void glLightPos( int id, const SrLight& l );
void glMaterial ( const SrMaterial &m ); //!< Sets material for GL_FRONT_AND_BACK
void glMaterialFront ( const SrMaterial &m );
void glMaterialBack ( const SrMaterial &m );

//==================================== matrices ==============================

void glMultMatrix ( const SrMat &m );
void glLoadMatrix ( const SrMat &m );
void glTranslate ( const SrVec &v );
void glScale ( float s );
void glRotate ( const SrQuat &q );
void glLookAt ( const SrVec &eye, const SrVec &center, const SrVec &up );
void glPerspective ( float fovy, float aspect, float znear, float zfar );
void glGetViewMatrix ( SrMat &m );
void glGetProjectionMatrix ( SrMat &m );

//==================================== info ==============================

void glPrintInfo ( SrOutput &o );

//================================ End of File ==================================

# endif // SR_GL_H
