#ifndef _SBAUTORIGMANAGER_H_
#define _SBAUTORIGMANAGER_H_

#include <string>
#include <map>
#include <vector>
#include <sr/sr_event.h>
#include <sr/sr_model.h>

class Mesh;

class SBAutoRigManager // build auto rigging given a static character mesh
{
	public:
		SBAutoRigManager();
		~SBAutoRigManager();
		static SBAutoRigManager* _singleton;
	public:
		static SBAutoRigManager& singleton() 
		{			
			return *singletonPtr();			
		}

		static SBAutoRigManager* singletonPtr() 
		{			
			if (!_singleton)
				_singleton = new SBAutoRigManager();
			return _singleton;			
		}

		static void destroy_singleton() {
			if( _singleton )
				delete _singleton;
			_singleton = NULL;
		}		

		bool buildAutoRigging(SrModel& inModel, std::string outSkName, std::string outDeformableMeshName);
		bool buildAutoRiggingVoxels(SrModel& inModel, std::string outSkName, std::string outDeformableMeshName);	
		bool buildAutoRiggingVoxelsWithVoxelSkinWeights(SrModel& inModel, std::string outSkName, std::string outDeformableMeshName);		
		bool buildAutoRiggingFromPawnMesh(const std::string& pawnName, int riggingType, const std::string& outSkName, const std::string& outDeformableMeshName);	

		bool updateSkinWeightFromCharacterMesh(const std::string& charName, int weightType);

};

SBAutoRigManager* getAutoRigManager();
#endif
