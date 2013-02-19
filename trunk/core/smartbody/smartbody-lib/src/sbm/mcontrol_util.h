#ifndef _MCONTROL_UTIL_H_
#define _MCONTROL_UTIL_H_


class mcuCBHandle;

#include <map>
#include <vhcl.h>



#if LINK_VHMSG_CLIENT
#include "vhmsg-tt.h"
#endif

#include <sbm/sbm_constants.h>

#include <sbm/GenericViewer.h>
#include <sb/SBVHMsgManager.h>
#include "sbm_constants.h"

#include "sr_hash_map.h"
#include "sr_cmd_map.h"
#include "sr_cmd_seq.h"
#include "sr_path_list.h"

#include <sbm/action_unit.hpp>
#include <sbm/general_param_setting.h>
#include <bml/bml_processor.hpp>



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


namespace SmartBody
{
    class SBScene;
    class SBCommandManager;
};


class KinectProcessor;

//////////////////////////////////////////////////////////////////




class VHMsgLog;

class mcuCBHandle {
	protected:
		// Data
	

	public:
	
		//double      physicsTime;

		KinectProcessor*							kinectProcessor;
		SrViewerFactory *viewer_factory;
		SrViewerFactory *ogreViewerFactory;
		SrViewer	*viewer_p;
		SrViewer    *ogreViewer_p;
		
		SrCamera	*camera_p;
		

		std::map<std::string, DeformableMesh*> deformableMeshMap;

		GeneralParamMap				param_map;			// map that contains the information of shader parameters
	
public:

		BML_PROCESSOR				bml_processor;

		

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

		void render();
			
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


#endif
