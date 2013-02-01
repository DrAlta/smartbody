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

#ifdef __ANDROID__
#define LINK_VHMSG_CLIENT		(1)
#define USE_WSP 1
#elif defined(__native_client__)
#define USE_WSP 0
#else
#define LINK_VHMSG_CLIENT		(1)
#define USE_WSP 1
#endif


#if LINK_VHMSG_CLIENT
#include "vhmsg-tt.h"
#endif

#include <sbm/GenericViewer.h>

#include "sbm_constants.h"

#include "sr_hash_map.h"
#include "sr_cmd_map.h"
#include "sr_cmd_seq.h"
#include "sr_path_list.h"
#include "remote_speech.h"
#include "local_speech.h"
#include "text_speech.h" // [BMLR]
#include "sbm_speech_audiofile.hpp"

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

class SequenceManager
{
	public:
		SequenceManager();
		~SequenceManager();

		bool addSequence(const std::string& seqName, srCmdSeq* seq);
		bool removeSequence(const std::string& seqName, bool deleteSequence);
		srCmdSeq* getSequence(const std::string& name);
		srCmdSeq* getSequence(int num, std::string& name);
		int getNumSequences();

		void clear();

	protected:
		std::set<std::string> _sequenceSet;
		std::vector<std::pair<std::string, srCmdSeq*> > _sequences;
};

class VHMsgLog;

class mcuCBHandle {
	protected:
		// Data
	

	public:
		// Data
		unsigned int				queued_cmds;
		
		bool		loop;
		bool		vhmsg_enabled;
		vhcl::Log::Listener* logListener;
		bool		net_bone_updates;
		bool		net_world_offset_updates;
		bool        resourceDataChanged;
		int testBMLId;

		
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

		srCmdMap <mcuCBHandle>		cmd_map;
		srCmdMap <mcuCBHandle>		set_cmd_map;
		srCmdMap <mcuCBHandle>		print_cmd_map;
		srCmdMap <mcuCBHandle>		test_cmd_map;

		SequenceManager	pendingSequences;
		SequenceManager activeSequences;

		std::map<std::string, DeformableMesh*> deformableMeshMap;

		GeneralParamMap				param_map;			// map that contains the information of shader parameters

		std::string getValidName(const std::string& name);
		int registerCharacter(SbmCharacter* character);
		int unregisterCharacter(SbmCharacter* character);
		int registerPawn(SbmPawn* pawn);
		int unregisterPawn(SbmPawn* pawn);

		std::map<std::string, SbmPawn*>& getPawnMap();
		bool addPawn(SbmPawn* pawn);
		void removePawn(const std::string& name);
		SbmPawn* getPawn(const std::string& name);
		int getNumPawns();

		std::map<std::string, DeformableMesh*>& getDeformableMeshMap();
		DeformableMesh* getDeformableMesh(const std::string& name);

		
		std::map<std::string, SbmCharacter*>& getCharacterMap();


		std::map<std::string, SmartBody::Nvbg*> nvbgMap;
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

		int vhmsg_send( const char *op, const char* message );

		int vhmsg_send( const char* message );

		void NetworkSendSkeleton( bonebus::BoneBusCharacter * character, SkSkeleton * skeleton, GeneralParamMap * param_map );
		// ----------------------------------------------
		// END vhmsg and network management
		// ----------------------------------------------

		// ----------------------------------------------
		// asset management
		// ----------------------------------------------
		
		int map_skeleton( const char * mapName, const char * skeletonName );
		int map_motion( const char * mapName, const char * motionName );

		// ----------------------------------------------
		// END asset management
		// ----------------------------------------------

		srCmdSeq* lookup_seq( const char* );

		// ----------------------------------------------
		// command management
		// ----------------------------------------------
		int execute( const char *key, srArgBuffer& args ) { 
			std::stringstream strstr;
			strstr << key << " " << args.peek_string();
			

			int ret = ( cmd_map.execute( key, args, this ) );
			if (ret == CMD_SUCCESS)
				resourceDataChanged = true;

			return ret; 
		}

		int execute( const char *key, char* strArgs ) {
			std::stringstream strstr;
			strstr << key << " " << strArgs;

            srArgBuffer args( strArgs );
			
			int ret = ( cmd_map.execute( key, args, this ) );
			if (ret == CMD_SUCCESS)
				resourceDataChanged = true;

			return ret; 
		}

		int execute( char *cmd ) { 

			//LOG("execute cmd = %s\n",cmd);
			// check to see if this is a sequence command
			// if so, save the command id
			std::string checkCmd = cmd;
			size_t startpos = checkCmd.find_first_not_of(" \t");
			if( std::string::npos != startpos )
				checkCmd = checkCmd.substr( startpos );

			int ret = ( cmd_map.execute( cmd, this ) ); 
			if (ret == CMD_SUCCESS)
				resourceDataChanged = true;

			return ret;
		}

		int execute_seq( srCmdSeq *seq );
		int execute_seq( srCmdSeq *seq, const char* seq_name );
		int execute_seq_chain( const std::vector<std::string>& seq_names, const char* error_prefix = NULL );

		//  Schedule command in some seconds
		int execute_later( const char* command, float seconds );

		//  Queue command for next frame
		int execute_later( const char* command ) { 
			return( execute_later( command, 0 ) ); 
		}

		int executePython(const char* command);
		int executePythonFile(const char* filename);


		int abortSequence( const char* command );
		int deleteSequence( const char* command );

		// ----------------------------------------------
		// END command management
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


		// ----------------------------------------------
		// command setup management
		// ----------------------------------------------
		int insert_set_cmd( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( set_cmd_map.insert( key, fp ) );
		}

		int insert_print_cmd( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( print_cmd_map.insert( key, fp ) );
		}

		int insert_test_cmd( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp )	{
			return( test_cmd_map.insert( key, fp ) );
		}
		int insert( const char *key, srCmdMap<mcuCBHandle>::sr_cmd_callback_fp fp, char* description = NULL )
		{
			//if (cmd_map.is_command(key))
				return( cmd_map.insert( key, fp ) );
			//else
			//	return CMD_SUCCESS;
		}

		void registerCallbacks();
		// ----------------------------------------------
		// END command setup management
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
			 mcuCBHandle& mcu = mcuCBHandle::singleton();
			 mcu.vhmsg_send("sbmlog", message.c_str());
		}
};
//////////////////////////////////////////////////////////////////

#endif
