#ifndef _BMLDEFS_H_
#define _BMLDEFS_H_

#include "bml.hpp"

namespace BML {

class BMLDefs
{
	public:
		BMLDefs();
		~BMLDefs();

		static XMLCh* DTYPE_SBM;

		static XMLCh* TAG_DESCRIPTION;
		static XMLCh* ATTR_ID;
		static XMLCh* ATTR_TYPE;
		static XMLCh* ATTR_NAME;
		static XMLCh* ATTR_LEVEL;
		static XMLCh* ATTR_HANDLE;

		static XMLCh* ATTR_START;
		static XMLCh* ATTR_READY;
		static XMLCh* ATTR_STROKE_START;
		static XMLCh* ATTR_STROKE;
		static XMLCh* ATTR_STROKE_END;
		static XMLCh* ATTR_RELAX;
		static XMLCh* ATTR_END;

		static XMLCh* TM_START;
		static XMLCh* TM_READY;
		static XMLCh* TM_STROKE_START;
		static XMLCh* TM_STROKE;
		static XMLCh* TM_STROKE_END;
		static XMLCh* TM_RELAX;
		static XMLCh* TM_END;

		static XMLCh* TAG_SBM_ANIMATION;
		static XMLCh* TAG_SBM_PANIMATION;
		static XMLCh* ATTR_SPEED;
		static XMLCh* ATTR_PVALUE;

		static XMLCh* ATTR_SHAKE;
		static XMLCh* ATTR_TOSS;
		static XMLCh* ATTR_ORIENT;
		static XMLCh* ATTR_NOD;

// todo
		static XMLCh* TAG_REF;
		static XMLCh* TAG_CURVE;
		static XMLCh* TAG_NUM_KEYS;
		static XMLCh* TAG_LIPS;
		static XMLCh* TAG_ARTICULATION;
		static XMLCh* TAG_SYNC;
		static XMLCh* TAG_TIME;
//

		static XMLCh* TAG_SACCADE;
		static XMLCh* TAG_REACH;
		static XMLCh* TAG_BODYREACH;
		static XMLCh* TAG_CONSTRAINT;

		static XMLCh* TAG_SBM_EVENT;
		static XMLCh* ATTR_MESSAGE;

		static XMLCh* TAG_FACE;
		static XMLCh* TAG_GAZE;

		static XMLCh* TAG_PARAM;
		static XMLCh* TAG_INTERRUPT;

		static XMLCh* TAG_LOCOTMOTION;
		static XMLCh* TAG_QUICKDRAW;

		static XMLCh* TAG_SPEECH;
		static XMLCh* TAG_SBM_SPEECH;
		
		static XMLCh* ATTR_TARGET;

		// XMLStrings (utf-16 character arrays) for parsing vrSpeak's XML
		static XMLCh* TAG_ACT;
		static XMLCh* TAG_BML;
		static XMLCh* TAG_BODY;
		static XMLCh* TAG_TORSO;
		static XMLCh* TAG_REQUIRED;
		#ifdef BMLR_BML2ANIM
		static XMLCh* TAG_POSTURE;
		#endif
		static XMLCh* TAG_HEAD;
		static XMLCh* TAG_TM;
		static XMLCh* TAG_MARK;


		static XMLCh* TAG_SBM_COMMAND;

		// Deprecated behavior tags
		static XMLCh* TAG_ANIMATION;
		static XMLCh* TAG_EVENT;

		static XMLCh* TAG_PANIMATION;

		// XMLStrings (utf-16 character arrays) for parsing vrSpeak's XML
		static XMLCh* ATTR_SPEAKER;
		static XMLCh* ATTR_ADDRESSEE;
		static XMLCh* ATTR_CONTENTTYPE;
		static XMLCh* ATTR_LANG;
		static XMLCh* ATTR_TID;
		static XMLCh* ATTR_POSTURE;
		static XMLCh* ATTR_REPEATS;
		static XMLCh* ATTR_AMOUNT;
		static XMLCh* ATTR_VELOCITY;
		static XMLCh* ATTR_ANGLE;
		static XMLCh* ATTR_DIRECTION;
		static XMLCh* ATTR_ROLL;
		static XMLCh* ATTR_SMOOTH;

		////// XML Direction staticants
		// Angular (gaze) and orienting (head)
		static XMLCh* DIR_RIGHT;
		static XMLCh* DIR_LEFT;
		static XMLCh* DIR_UP;
		static XMLCh* DIR_DOWN;
		// Angular only
		static XMLCh* DIR_UPRIGHT;
		static XMLCh* DIR_UPLEFT;
		static XMLCh* DIR_DOWNRIGHT;
		static XMLCh* DIR_DOWNLEFT;
		static XMLCh* DIR_POLAR;
		// Orienting only
		static XMLCh* DIR_ROLLRIGHT;
		static XMLCh* DIR_ROLLLEFT;

		static XMLCh* ATTR_ROOT;
		static XMLCh* ATTR_ROOTJOINT;
		static XMLCh* ATTR_EFFECTOR;
		static XMLCh* ATTR_CONSTRAINT_TYPE;
		static XMLCh* ATTR_EFFECTOR_ROOT;
		static XMLCh* ATTR_FADE_OUT;
		static XMLCh* ATTR_FADE_IN;

		static XMLCh* ATTR_RAMPUP;
		static XMLCh* ATTR_RAMPDOWN;
		static XMLCh* ATTR_DURATION;

		static XMLCh* ATTR_OFFSET_ROTX;
		static XMLCh* ATTR_OFFSET_ROTY;
		static XMLCh* ATTR_OFFSET_ROTZ;
		//static XMLCh* ATTR_OFFSET_POS[]         = L"offset-pos";

		static XMLCh* ATTR_X;
		static XMLCh* ATTR_Y;
		static XMLCh* ATTR_Z;

		static XMLCh* ATTR_POSX;
		static XMLCh* ATTR_POSY;
		static XMLCh* ATTR_POSZ;

		static XMLCh* ATTR_ROTX;
		static XMLCh* ATTR_ROTY;
		static XMLCh* ATTR_ROTZ;

		static XMLCh* ATTR_TARGET_POS;
		static XMLCh* ATTR_REACH_VELOCITY;
		static XMLCh* ATTR_REACH_FINISH;
		static XMLCh* ATTR_REACH_TYPE;

		static XMLCh* ATTR_ROTATION;
		static XMLCh* ATTR_ENABLE;

		static XMLCh* TAG_VISEME;
		static XMLCh* TAG_SOUND;
		static XMLCh* ATTR_AU;
		static XMLCh* ATTR_SIDE;

		static XMLCh* ATTR_SBM_ROLL;
		static XMLCh* ATTR_JOINT_RANGE;
		static XMLCh* ATTR_JOINT_SPEED;
		static XMLCh* ATTR_TIME_HINT;
		static XMLCh* ATTR_JOINT_SMOOTH;
		static XMLCh* ATTR_PITCH;
		static XMLCh* ATTR_HEADING;
		static XMLCh* ATTR_BLEND;
		static XMLCh* ATTR_INTERPOLATE_BIAS;

		static XMLCh* ATTR_PRIORITY_JOINT;
		static XMLCh* ATTR_PITCH_MIN;
		static XMLCh* ATTR_PITCH_MAX;

		static XMLCh* VALUE_TEXT_PLAIN;
		static XMLCh* VALUE_SSML;

		static XMLCh* start_id;
		static XMLCh* end_id;

		static XMLCh* ATTR_REACH_ARM;
		static XMLCh* ATTR_USE_EXAMPLE;
		static XMLCh* ATTR_BUILD_EXAMPLE;
		static XMLCh* ATTR_EXAMPLE_DIST;
		static XMLCh* ATTR_RESAMPLE_SIZE;

		static XMLCh* ATTR_ANIM;
		static XMLCh* ATTR_TRACK_DUR;
		static XMLCh* ATTR_GUNDRAW_DUR;
		static XMLCh* ATTR_HOLSTER_DUR;
		static XMLCh* ATTR_AIM_OFFSET;



		static XMLCh* ATTR_ANIM1;
		static XMLCh* ATTR_ANIM2;
		static XMLCh* ATTR_LOOP;

		static XMLCh* ATTR_VALUE;

		static XMLCh* ATTR_TWARP;

		static XMLCh* ATTR_TRUE;
		static XMLCh* ATTR_FALSE;

		static XMLCh* ATTR_ARMLEFT;
		static XMLCh* ATTR_ARMRIGHT;

		static XMLCh* ATTR_FACS;

		static XMLCh* ATTR_RPS;
		static XMLCh* ATTR_GRPS;
		static XMLCh* ATTR_LRPS;

		static XMLCh* ATTR_EYEBROWS;
		static XMLCh* ATTR_EYELIDS;
		static XMLCh* ATTR_MOUTH;
		static XMLCh* ATTR_APEX_DURATION;
		static XMLCh* TAG_GRAB;
		static XMLCh* ATTR_CONS_JOINT;
		static XMLCh* ATTR_CONS_TARGET;
		static XMLCh* ATTR_WRIST;
		static XMLCh* ATTR_GRAB_TYPE;
		static XMLCh* ATTR_GRAB_VELOCITY;
		static XMLCh* ATTR_GRAB_FINISH;
		static XMLCh* ATTR_OBSTACLE;
		static XMLCh* ATTR_GRAB_STATE;
		static XMLCh* ATTR_REACH_ACTION;
		static XMLCh* ATTR_REACH_DURATION;
		static XMLCh* TAG_EXAMPLE_LOCOMOTION;
		static XMLCh* ATTR_PROXIMITY;
		static XMLCh* ATTR_MANNER;
		static XMLCh* ATTR_FACING;
		static XMLCh* ATTR_FOLLOW;
		static XMLCh* ATTR_NUM_STEPS;
		static XMLCh* ATTR_SPD;
		static XMLCh* ATTR_SOURCE_JOINT;
		static XMLCh* ATTR_ATTACH_PAWN;
		static XMLCh* ATTR_RELEASE_PAWN;
		static XMLCh* ATTR_FOOT_IK;
		static XMLCh* ATTR_MAGNITUDE;
		static XMLCh* ATTR_MODE;
		static XMLCh* ATTR_ANGLELIMIT;
		static XMLCh* ATTR_FINISH;

		static XMLCh* ATTR_STEERACCEL;
		static XMLCh* ATTR_STEERANGLEACCEL;
		static XMLCh* ATTR_STEERSCOOTACCEL;
		static XMLCh* ATTR_NODAXIS;
		static XMLCh* ATTR_NODACCEL;
		static XMLCh* ATTR_NODPERIOD;
		static XMLCh* ATTR_NODWARP;
		static XMLCh* ATTR_NODPITCH;
		static XMLCh* ATTR_NODDECAY;
		static XMLCh* ATTR_WIGGLE;
		static XMLCh* ATTR_WAGGLE;

		static XMLCh* TAG_STATES;
		static XMLCh* ATTR_STARTINGNOW;

		static XMLCh* ATTR_PERCENTAGE_BIN0;
		static XMLCh* ATTR_PERCENTAGE_BIN45;
		static XMLCh* ATTR_PERCENTAGE_BIN90;
		static XMLCh* ATTR_PERCENTAGE_BIN135;
		static XMLCh* ATTR_PERCENTAGE_BIN180;
		static XMLCh* ATTR_PERCENTAGE_BIN225;
		static XMLCh* ATTR_PERCENTAGE_BIN270;
		static XMLCh* ATTR_PERCENTAGE_BIN315;
		static XMLCh* ATTR_SACCADE_INTERVAL_MEAN;
		static XMLCh* ATTR_SACCADE_INTERVAL_VARIANT;

		// OpenCollada
		static XMLCh* ATTR_SID;
		static XMLCh* ATTR_COUNT;
		static XMLCh* ATTR_INPUT;
		static XMLCh* ATTR_SEMANTIC;
		static XMLCh* ATTR_VCOUNT;
};



}

#endif

