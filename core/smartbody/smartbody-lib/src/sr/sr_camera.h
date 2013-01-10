/*
 *  sr_camera.h - part of Motion Engine and SmartBody-lib
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

# ifndef SR_CAMERA_H
# define SR_CAMERA_H

/** \file sr_camera.h 
 * Keeps camera parameters
 */

# include <sr/sr_vec.h>
# include <sr/sr_quat.h>
#include <string>
#include <sb/SBPawn.h>

class SrMat;
class SrBox;

namespace SmartBody {
	class SBSubject;
}

/*! \class SrCamera sr_camera.h
    \brief Keeps camera parameters

    SrCamera contains the parameters to define a camera.
    Attention: if znear is too small inconsistencies in the rendering may appear;
    a minimal value of 0.1 should be considered. */
class SrCamera : public SmartBody::SBPawn
 {

   public :
    
    /*! Initialize the camera with the default parameters, see init(). */
    SrCamera ();

    /*! Copy constructor. */
    SrCamera ( const SrCamera* c );

    /*! Initialize the camera with the main parameters eye, center and up. */
    SrCamera ( const SrPnt& e, const SrPnt& c, const SrVec& u );

	void copyCamera(const SrCamera* c);
	void setScale(float scale);
	float getScale();
	void setEye(float x, float y, float z);
	SrVec getEye();
	void setCenter(float x, float y, float z);
	SrVec getCenter();
	void setUpVector(SrVec up);
	SrVec getUpVector();
	void setFov(float fov);
	float getFov();
	void setNearPlane(float n);
	float getNearPlane();
	void setFarPlane(float f);
	float getFarPlane();
	void setAspectRatio(float aspect);
	float getAspectRatio();

	void setTrack(const std::string& characterName, const std::string& jointName);
	void removeTrack();

	void print();
	void reset();
	
    /*! Set the parameters to their default values, which are :
        eye=(0,0,2), center=(0,0,0), up=(0,1,0), fovy=60, znear=0.1, zfar=1000, aspect=1. */
    void init ();

    /*! Set m to be the transformation matrix generated by the parameters
        eye, center, and up. A reference to m is also returned.
        Note: the scale factor is not included in this matrix. */
    SrMat& get_view_mat ( SrMat &m ) const;

    /*! Set m to be the transformation projection matrix generated by the parameters
        fovy, znear, zfar, aspect. A reference to m is also returned. */
    SrMat& get_perspective_mat ( SrMat &m ) const;

    /*! Gets the 3d ray (p1,p2) which projects exactly in the given window point
        according to the camera current parameters. Points p1 and p2 lye in the 
        near and far planes respectively. Window points are considered to be
        in normalized coordinates, ranging between [-1,1]. */
    void get_ray ( float winx, float winy, SrVec& p1, SrVec& p2 ) const;

    /*! Sets center at the center of the box, and put the eye in the semi-line
        rooted at center and with direction z, in a distance from the center
        that is sufficient to visualize all the box with the given fov_y parm.
        After this call, variable SrCamera::fovy will have the same value as fov_y.
        Note: the scale factor is set to one in this method. */
    void view_all ( const SrBox& box, float fov_y );

    /*! Apply a trackball translation induced from the mouse motion. First, the ray
        passing through each window position and the intersection point with the
        projection plane is determined. Then, the two intersection points determine the
        displacement to be applied to the trackball, after a multiplication with the
        current spin rotation. Mouse coordinates must be normalized in [-1,1]x[-1,1]. */
    void apply_translation_from_mouse_motion ( float lwinx, float lwiny, float winx, float winy );

    /*! Transforms the camera position with the given rotation. The rotation is
        applied to the up vector, and to the eye vector in the following way by
        going: x-=center; x=x*q; x+=center (x represents eye or up vector). */
    void operator*= ( const SrQuat& q );

    /*! Adds the vector v to the eye and center points. */
    void operator+= ( const SrVec& v );

    /*! Subtracts the vector v to the eye and center points. */
    void operator-= ( const SrVec& v );

    /*! Returns a camera that is the same as the given camera c, but with the
        rotation q applied. See the operator *= for a description of how the
        rotation is applied to the camera. */
    friend SrCamera operator* ( const SrCamera& c, const SrQuat& q );

    /*! Returns a camera that is the same as the given camera c, but with the
        the translation vector v added to the eye and center points. */
    friend SrCamera operator+ ( const SrCamera& c, const SrVec& v );

    /*! Output camera data values in format keyword1 value \n keyword2 value ...
        (keywords are: eye, center, up, etc*/
    friend SrOutput& operator<< ( SrOutput& out, const SrCamera& c );

    /*! Input camera data. Not all keywords are required to exist. The routine
        returns when a non-keyword entry is found (which is 'ungetted' in inp). */
    friend SrInput& operator>> ( SrInput& inp, SrCamera& c );

	virtual void notify(SmartBody::SBSubject* subject);

 protected:
    SrPnt  eye;    //!< position of the eye, default is (0,0,2).
    SrPnt  center; //!< position where the eye is looking to, default is (0,0,0).
    SrVec  up;     //!< the up vector orients the camera around the eye-center vector, default is (0,1,0)
    float  fovy;   //!< the y field of view in radians. Default is pi/3 (60deg), range is [0.01,pi].
    float  znear;  //!< must be >0, default is 0.1.
    float  zfar;   //!< must be >0, default is 1000.
    float  aspect; //!< normally is set to the screen width/heigh, default is 1.
    float  scale;  //!< a scale factor to be applied between the view matrix and the scene

 };

//================================ End of File =================================================

# endif // SR_CAMERA_H

