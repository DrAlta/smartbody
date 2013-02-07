#ifndef MCONTROL_UTIL_H
#define MCONTROL_UTIL_H
/*
 *  mcontrol_util.h - part of SmartBody-lib
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
 *      Marcus Thiebaux, USC
 *      Ed Fast, USC
 *      Andrew n marshall, USC
 *      Ashok Basawapatna, USC (no longer)
 *      Eric Forbell, USC
 *      Thomas Amundsen, USC
 */
//  Declare classes defined by this file
//  (prevents include recursion)
class mcuCBHandle;

#include <map>
#include <vhcl.h>



#if LINK_VHMSG_CLIENT
#include "vhmsg-tt.h"
#endif

#include <sbm/GenericViewer.h>
#include <sb/SBVHMsgManager.h>
#include <sb/SBScene.h>
#include "sbm_constants.h"

#include "sr_hash_map.h"
#include "sr_cmd_map.h"
#include "sr_cmd_seq.h"
#include "sr_path_list.h"

#include <sbm/action_unit.hpp>
#include <sbm/general_param_setting.h>



#ifndef __native_client__
#include <sb/SBPythonClass.h>
#endif







#ifndef USE_PYTHON
#define USE_PYTHON
#endif

#ifndef SB_NO_PYTHON
#ifndef __native_client__
#include <boost/python.hpp>
#endif
#endif

#include BML_PROCESSOR_INCLUDE

namespace SmartBody
{
    class SBScene;
};


#if USE_WSP
namespace WSP
{
    class Manager;
};
#endif

class PABlend;
class PATransition;
class Heightfield;
class SbmPawn;
class SbmCharacter;
class Nvbg;
class KinectProcessor;
class SrViewer;
class SrCamera;

//////////////////////////////////////////////////////////////////

class CameraTrack
{
	public:
		SkJoint* joint;
		SrVec jointToCamera;
		SrVec targetToCamera;
		double yPos;
		double threshold;
};


class VHMsgLog;

class mcuCBHandle {
	protected:
		// Data
	

	public:
		// Data
		vhcl::Log::Listener* logListener;
		bool		net_bone_updates;
		bool		net_world_offset_updates;
		
		//double      physicsTime;

		KinectProcessor*							kinectProcessor;
		SrViewerFactory *viewer_factory;
		SrViewerFactory *ogreViewerFactory;
		SrViewer	*viewer_p;
		SrViewer    *ogreViewer_p;
		
		SrCamera	*camera_p;
		SrSnGroup	*root_group_p;
		
		Heightfield *height_field_p;

		std::string initPythonLibPath;

	


		std::map<std::string, DeformableMesh*> deformableMeshMap;

		GeneralParamMap				param_map;			// map that contains the information of shader parameters

		std::string getValidName(const std::string& name);

		std::map<std::string, SbmPawn*>& getPawnMap();
		bool addPawn(SbmPawn* pawn);
		void removePawn(const std::string& name);
		SbmPawn* getPawn(const std::string& name);
		int getNumPawns();
		
		std::map<std::string, SbmCharacter*>& getCharacterMap();


		std::map<std::string, SbmPawn*>	pawn_map;
		std::map<std::string, SbmCharacter*> character_map;


#ifndef SB_NO_PYTHON
#ifndef __native_client__
		boost::python::object mainModule;
		boost::python::object mainDict;
#endif		
#endif
	
public:

		BML_PROCESSOR				bml_processor;

#if USE_WSP
		WSP::Manager*				theWSP;
#endif

		std::vector<CameraTrack*>	cameraTracking;

		//SbmPhysicsSim*              physicsEngine;

	private:
		// Constant
		static mcuCBHandle* _singleton;

		//  Constructor
		mcuCBHandle( void );
		virtual ~mcuCBHandle( void );
		void clear();

		// Private access prevents calls to the following
		mcuCBHandle( mcuCBHandle& );
		mcuCBHandle& operator= (const mcuCBHandle&);

	public:
		static mcuCBHandle& singleton() {
			//LOG("Begin Singleton\n");
			//if (!_singleton)
			//	LOG("Singleton is NULL\n");
			if( !_singleton )
				_singleton = new mcuCBHandle();
			//LOG("End Singleton\n");
			return *_singleton;
		}
		static void destroy_singleton() {
			if( _singleton )
				delete _singleton;
			_singleton = NULL;
		}		


		void reset();

		// ----------------------------------------------
		// scene management
		// ----------------------------------------------
		int add_scene( SrSnGroup *scene_p );
		int remove_scene( SrSnGroup *scene_p );
		void render();
		// ----------------------------------------------
		// END scene management
		// ----------------------------------------------
		
		// ----------------------------------------------
		// terrain management
		// ----------------------------------------------
		void render_terrain( int renderMode ) ;
		float query_terrain( float x, float z, float *normal_p );
		// ----------------------------------------------
		// END terrain management
		// ----------------------------------------------
		

		// ----------------------------------------------
		// vhmsg and network management
		// ----------------------------------------------
		void set_net_host( const char * net_host );

		void NetworkSendSkeleton( bonebus::BoneBusCharacter * character, SkSkeleton * skeleton, GeneralParamMap * param_map );
		// ----------------------------------------------
		// END vhmsg and network management
		// ----------------------------------------------
	
		// ----------------------------------------------
		// viewer management
		// ----------------------------------------------

		
		int open_viewer( int width, int height, int px, int py );
		void close_viewer( void );

		int openOgreViewer( int width, int height, int px, int py );
		void closeOgreViewer( void );


		void register_viewer_factory(SrViewerFactory* factory) { 
				if (viewer_factory != NULL) delete viewer_factory;
				viewer_factory = factory;
		}
		
		void register_OgreViewer_factory(SrViewerFactory* factory) { 
			if (ogreViewerFactory != NULL) delete ogreViewerFactory;
			ogreViewerFactory = factory;
		}	
		// ----------------------------------------------
		// END viewer management
		// ----------------------------------------------


	public:
		
};

class VHMsgLogger : public vhcl::Log::Listener
{
	public:
		VHMsgLogger() : vhcl::Log::Listener()
		{
		}
        
		virtual ~VHMsgLogger()
		{
		}

        virtual void OnMessage( const std::string & message )
		{
			SmartBody::SBScene::getScene()->getVHMsgManager()->send("sbmlog", message.c_str());
		}
};
//////////////////////////////////////////////////////////////////

#endif
