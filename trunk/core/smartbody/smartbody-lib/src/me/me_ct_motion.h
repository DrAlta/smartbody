/*
 *  me_ct_motion.h - part of Motion Engine and SmartBody-lib
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
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently at UC Merced)
 *      Andrew n marshall, USC
 */


# ifndef ME_CT_MOTION_H
# define ME_CT_MOTION_H

//=================================== MeCtMotion =====================================

# include <SR/sr_hash_table.h>
# include <SR/sr_buffer.h>

# include <SK/sk_motion.h>

# include <ME/me_controller.h>
#include <queue>

/*! This motion controller provides a controller interface to play
    an attached SkMotion. Besides few extra functionality such as
    time warping, loop, etc; it also efficiently supports the
    creation of several MeCtMotions sharing a same SkMotion. */
class MeCtMotion : public MeController
 { private :
    SkMotion*            _motion;    // the motion,
    SkMotion::InterpType _play_mode; // its play mode
//    double               _duration;  // the time-warped duration
	double               _offset;    // time offset of the animation
    float                _maxtwarp;  // max time warping factor allowed to increase the motion speed
    float                _mintwarp;  // min time warping factor allowed to reduce the motion speed
    float                _twarp;     // current used warping time
    bool                 _loop;      // if the motion is to be played in loop
    int                  _last_apply_frame; // to optimize shared motion evaluation
	SrBuffer<int>        _mChan_to_buff; // motion's channels to context's buffer index
	std::queue<MotionEvent*> _events;
	int					 _lastCycle;

   public :
    static const char* type_name;

   public :
    /*! Constructor */
    MeCtMotion ();

    /*! Destructor is public but pay attention to the use of ref()/unref() */
   virtual ~MeCtMotion ();

    /*! Set the motion to be used. A valid motion must be set using
        this method before calling any other method.
        The old motion is unreferenced, and the new one is referenced.
        (SkMotion derives SrSharedClass and has ref/unref methods)
        The keytimes of m are translated to ensure start from zero. 
        MeController::init() is automatically called. */
    void init ( SkMotion* m_p );
    void init ( SkMotion* m_p, double time_offset, double time_scale );

	/*! Initialize a controller by cloning another */
//	void init ( MeCtMotion* other );

    /*! Returns a pointer to the current motion of this controller */
    SkMotion* motion () { return _motion; }

    /*! Set the play mode, default is linear */
    void play_mode ( SkMotion::InterpType it ) { _play_mode=it; }

    /*! Returns the current play mode */
    SkMotion::InterpType play_mode () const { return _play_mode; }

    /*! Defines the maximum and minimum time scales acceptable for this motion. 
        These parameters are used to determine the feasibility of warping the motion
        in order to reach imposed timing constraints. */
//    void warp_limits ( float wmin, float wmax );
    
    /*! Returns the maximum time warping factor */
//    float maxtwarp () const { return _maxtwarp; }
    
    /*! Returns the minimum time warping factor */
//    float mintwarp () const { return _mintwarp; }
    
    /*! Set a desired time warping ratio, that will be clamped to the min/max values */
    void twarp ( float tw );

    /*! Returns the current time warping ratio, default is 1 (not warping) */
    float twarp () const { return _twarp; }

    /*! Returns true if to be played in loop, and false otherwise */
    bool loop () const { return _loop; }

    /*! Change the loop state. If in loop, the duration of the controller will 
        be undetermined, ie -1 */
    void loop ( bool b ) { _loop=b; }

    /*! This method will return the fixed total duration time that the controller
        will take, or will return -1 if the controller is looped */
    virtual double controller_duration ();

	/*! Gets the offset from the start of the motion. */
	virtual double offset () { return _offset; }

	/*! Sets the offset from the start of the motion. */
	void offset ( double amount );

    /*! Returns the duration of the warped motion, which corresponds to the phase in
        a loop controller with undetermined duration */
//    double phase_duration () { return _duration; }
    double phase_duration () { return synch_points.get_duration(); }

    /*! Output data */
    void output ( SrOutput& out );

    /*! Reads the data. Returns false if the read motion name is not in the
        given hash table of motions */
    bool input ( SrInput& inp, const SrHashTable<SkMotion*>& motions );

	virtual void print_state( int tabCount );
	
	SrBuffer<int>& get_context_map();

	void checkMotionEvents(double time);
	void loadMotionEvents();

   private:
	void map_floats();

	// callbacks for the base class
	virtual void controller_map_updated();
	virtual bool controller_evaluate ( double t, MeFrameData& frame );
    virtual SkChannelArray& controller_channels ();
    virtual const char* controller_type () const;
};

//======================================= EOF =====================================

# endif // ME_CT_MOTION_H

