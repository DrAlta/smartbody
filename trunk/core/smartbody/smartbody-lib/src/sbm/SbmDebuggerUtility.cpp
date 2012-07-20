#include "vhcl.h"
#include "vhcl_socket.h"

#include "NetRequest.h"
#include "SbmDebuggerUtility.h"
#include "SbmDebuggerClient.h"

#include <sb/SBScene.h>
#include <sb/SBMotion.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sbm/mcontrol_util.h>
#include <sb/SBPythonClass.h>

bool QueryResourcesCB(void* caller, NetRequest* req);

SbmDebuggerUtility::SbmDebuggerUtility()
{
}

SbmDebuggerUtility::~SbmDebuggerUtility()
{
}


void SbmDebuggerUtility::initScene()
{
	
}

void SbmDebuggerUtility::queryResources()
{
	SmartBody::SBScene* sbScene = SmartBody::SBScene::getScene();
	SbmDebuggerClient* client = sbScene->getDebuggerClient();
	if (!client)
		return;

	// get assets
	std::string assetTypes[4] = {"script", "motion", "audio", "mesh"};
	NetRequest::RequestId correspondingIds[4] = {NetRequest::Get_Seq_Asset_Paths, NetRequest::Get_ME_Asset_Paths, NetRequest::Get_Audio_Asset_Paths, NetRequest::Get_Mesh_Asset_Paths};
	for (int i = 0; i < 4; i++)
	{
		std::string command = vhcl::Format("scene.getAssetPaths(\'%s\')", assetTypes[i].c_str());
		client->SendSBMCommand(correspondingIds[i], "string-array", command, QueryResourcesCB, this);
	}

	// get scripts, service, motion
	client->SendSBMCommand(NetRequest::Get_Script_Names, "string-array", "scene.getScriptNames()", QueryResourcesCB, this);
	client->SendSBMCommand(NetRequest::Get_Service_Names, "string-array", "scene.getServiceManager().getServiceNames()", QueryResourcesCB, this);
	client->SendSBMCommand(NetRequest::Get_Motion_Names, "string-array", "scene.getMotionNames()", QueryResourcesCB, this);
	//	client->SendSBMCommand(NetRequest::Get_Skeleton_Names, "string-array", "scene.getSkeletonNames()", QueryResourcesCB, this);
	//	client->SendSBMCommand(NetRequest::Get_BoneMap_Names, "string-array", "scene.getBoneMapNames()", QueryResourcesCB, this);
	//	client->SendSBMCommand(NetRequest::Get_EventHandler_Names, "string-array", "scene.getEventHandlerNames()", QueryResourcesCB, this);   
	client->SendSBMCommand(NetRequest::Get_Scene_Scale, "float", "scene.getScale()", QueryResourcesCB, this);
}

/*
	Init character given serialized skeleton information
*/
void SbmDebuggerUtility::initCharacter(const std::string& name, const std::string& skelName)
{
	if (name == "")
	{
		LOG("Character has no name - will not be created.");
		return;
	}
	SBCharacter* sbCharacter = SmartBody::SBScene::getScene()->createCharacter(name, "");
	if (!sbCharacter)
	{
		LOG("Problem creating character %s, will not be created in remote session...", name.c_str());
		return;
	}
	SBSkeleton* sbSkeleton = SmartBody::SBScene::getScene()->getSkeleton(skelName);
	if (!sbSkeleton)
	{
		LOG("Problem creating skeleton %s, character %s will not be created in remote session...", name.c_str(), skelName.c_str());
		return;
	}
	SBSkeleton* copySbSkeleton = new SBSkeleton(sbSkeleton);
	if (!copySbSkeleton)
	{
		LOG("Problem creating copy of skeleton %s, character %s will not be created in remote session...", name.c_str(), skelName.c_str());
		return;
	}
	sbCharacter->setSkeleton(copySbSkeleton);	
}

void SbmDebuggerUtility::initCharacterFaceDefinition(const std::string& characterName, const std::string& faceDefName, const std::string& message)
{
	SmartBody::SBScene* sbScene = SmartBody::SBScene::getScene();
	SBCharacter* sbCharacter = sbScene->getCharacter(characterName);
	if (!sbCharacter)
		return;

	SBFaceDefinition* faceDef = sbScene->getFaceDefinition(faceDefName);
	if (!faceDef)
		return;

	sbCharacter->setFaceDefinition(faceDef);
}

void SbmDebuggerUtility::initPawn(const std::string& name)
{
	SBPawn* sbPawn = SmartBody::SBScene::getScene()->createPawn(name);
}

/*
	Run python command block sent from server. Current usages are listed as following:
	- Init blend given serialized blend information
	- Init transition
	- Init face definition
*/
void SbmDebuggerUtility::runPythonCommand(const std::string& info)
{
	mcuCBHandle::singleton().executePython(info.c_str());
}

void SbmDebuggerUtility::initSkeleton(const std::string& skFileName, const std::string& info)
{
	SrInput input(info.c_str());
	SBSkeleton* sbSkel = new SBSkeleton();
	SkSkeleton* skSkel = sbSkel;
	skSkel->load(input);
	skSkel->skfilename(skFileName.c_str());
	mcuCBHandle::singleton().skeleton_map.insert(std::pair<std::string, SkSkeleton*>(sbSkel->name(), skSkel));
}


void SbmDebuggerUtility::updateCharacter(const std::string& cName, const std::string& jName, 
										 float& posX, float& posY, float& posZ, 
										 float& rotX, float& rotY, float& rotZ, float& rotW)
{
	SBCharacter* sbCharacter = SmartBody::SBScene::getScene()->getCharacter(cName);
	if (!sbCharacter)
		return;

	SBJoint* sbJoint = sbCharacter->getSkeleton()->getJointByName(jName);
	if (sbJoint)
	{
		sbJoint->pos()->value(0, (float)posX);
		sbJoint->pos()->value(1, (float)posY);
		sbJoint->pos()->value(2, (float)posZ);
		sbJoint->quat()->value(SrQuat((float)rotW, (float)rotX, (float)rotY, (float)rotZ));
	}
}

void SbmDebuggerUtility::updatePawn(const std::string& pName, float& posX, float& posY, float& posZ, 
									float& rotX, float& rotY, float& rotZ, float& rotW)
{
	SBPawn* sbPawn  = SmartBody::SBScene::getScene()->getPawn(pName);
	if (!sbPawn)
		return;

	// TODO: const_cast? really?
	SkJoint* skJoint = const_cast<SkJoint*> (sbPawn->get_world_offset_joint());
	if (skJoint)
	{
		skJoint->pos()->value(0, (float)posX);
		skJoint->pos()->value(1, (float)posY);
		skJoint->pos()->value(2, (float)posZ);
		skJoint->quat()->value(SrQuat((float)rotW, (float)rotX, (float)rotY, (float)rotZ));
	}
}

void SbmDebuggerUtility::updateCamera(float& eyePosX, float& eyePosY, float& eyePosZ, 
									  float& lookAtPosX, float& lookAtPosY, float& lookAtPosZ, 
									  float& fovY, float& aspect, float& zNear, float zFar)
{
	SrCamera* camera = mcuCBHandle::singleton().viewer_p->get_camera();

	camera->eye.x = eyePosX;
	camera->eye.y = eyePosY;
	camera->eye.z = eyePosZ;
	camera->center.x = lookAtPosX;
	camera->center.y = lookAtPosY;
	camera->center.z = lookAtPosZ;
	camera->fovy = sr_torad(fovY);
	camera->aspect = aspect;
	camera->znear = zNear;
	camera->zfar = zFar;
}


bool QueryResourcesCB(void* caller, NetRequest* req)
{
	SmartBody::SBScene* sbScene = SmartBody::SBScene::getScene();
	std::vector<std::string> args = req->Args();
	switch (req->Rid())
	{
	case NetRequest::Get_Seq_Asset_Paths:
		for (size_t i = 0; i < args.size(); i++)
			sbScene->addAssetPath("script", args[i]);
		break;

	case NetRequest::Get_ME_Asset_Paths:
		for (size_t i = 0; i < args.size(); i++)
			sbScene->addAssetPath("motion", args[i]);
		break;

	case NetRequest::Get_Audio_Asset_Paths:
		for (size_t i = 0; i < args.size(); i++)
			sbScene->addAssetPath("audio", args[i]);
		break;

	case NetRequest::Get_Mesh_Asset_Paths:
		for (size_t i = 0; i < args.size(); i++)
			sbScene->addAssetPath("mesh", args[i]);
		break;

	case NetRequest::Get_Script_Names:
		break;

	case NetRequest::Get_Service_Names:
		break;

	case NetRequest::Get_Motion_Names:
		for (size_t i = 0; i < args.size(); i++)
		{
			std::map<std::string, SkMotion*>& motionMap = mcuCBHandle::singleton().motion_map;
			SkMotion* dummyMotion = new SkMotion();
			motionMap.insert(std::make_pair(args[i], dummyMotion));
			dummyMotion->setName(args[i]);
		}
		break;

	case NetRequest::Get_Skeleton_Names:
		break;

	case NetRequest::Get_BoneMap_Names:
		break;

	case NetRequest::Get_EventHandler_Names:
		break;

	case NetRequest::Get_Scene_Scale:
		float scale = (float)vhcl::ToDouble(req->ArgsAsString());
		sbScene->setScale(scale);
		break;
	}
	return true;
}