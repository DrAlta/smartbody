#include "vhcl.h"
#include "AutoRigViewer.h"
#include "jointmapviewer/RetargetStepWindow.h"
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBAssetManager.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBBehaviorSetManager.h>
#include <sb/SBBehaviorSet.h>
#include <autorig/SBAutoRigManager.h>
#include <sbm/sbm_deformable_mesh.h>
#include <sk/sk_joint.h>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/glut.H>
#include <FL/fl_ask.H>
#include <sr/sr_gl.h>
#include <sr/sr_gl_render_funcs.h>
#include <sr/sr_sphere.h>
#include <sr/sr_sn_shape.h>
#include <sbm/gwiz_math.h>

#include <boost/filesystem.hpp>

#ifndef WIN32
#define _strdup strdup
#endif


AutoRigViewer::AutoRigViewer(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h, name)
{	
	begin();	
	int curY = 10;
	int startY = 10;	
	int yDis = 10;
	_choicePawns = new Fl_Choice(60, yDis, 150, 25, "Mesh Pawn");
	updatePawnList();
	yDis += 30;
	_choiceVoxelRigging = new Fl_Choice(60, yDis, 120, 25, "Type");
	_choiceVoxelRigging->add("Voxel Weight");
	_choiceVoxelRigging->add("Glow Weight");
	_choiceVoxelRigging->add("Diffusion Weight");
	_choiceVoxelRigging->value(0);

	yDis += 45;
	_buttonAutoRig = new Fl_Button(60, yDis, 120, 25, "Apply AutoRig");
	_buttonAutoRig->callback(ApplyAutoRigCB, this);
	
	end();
	_deletePawnName = "";
	retargetStepWindow = NULL;
}



AutoRigViewer::~AutoRigViewer()
{
	
}

void AutoRigViewer::draw()
{
	Fl_Double_Window::draw();
}


void AutoRigViewer::updateAutoRigViewer()
{
	updatePawnList();
}

void AutoRigViewer::updatePawnList()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	int oldValue = _choicePawns->value();
	std::string oldPawnName = "";
	if (oldValue >= 0 && oldValue < _choicePawns->size())
	{
		if (_choicePawns->text(oldValue))
			oldPawnName = _choicePawns->text(oldValue);
	}

	const std::vector<std::string>& pawns = scene->getPawnNames();
	_choicePawns->clear();
	int pawnCount = 0;
	bool hasPawn = false;
	for (size_t c = 0; c < pawns.size(); c++)
	{
		if (pawns[c] == _deletePawnName) // don't add remove character name
			continue;

		SmartBody::SBPawn* pawn = scene->getPawn(pawns[c]);
		SmartBody::SBCharacter* sbChar = dynamic_cast<SmartBody::SBCharacter*>(pawn);
		if (!sbChar && pawn->dStaticMeshInstance_p && pawn->dStaticMeshInstance_p->getDeformableMesh()) // only add pawns that has attached mesh
		{
			_choicePawns->add(pawns[c].c_str());
			if (pawns[c] == oldPawnName)
			{
				_choicePawns->value(pawnCount);
				hasPawn = true;
			}
			pawnCount++;
		}		
	}

	if (!hasPawn) // no character, set to default character
	{		
		for (int c = 0; c < _choicePawns->size(); c++)
		{
			if (_choicePawns->text(c))
			{
				_choicePawns->value(c);
				//setCharacterName(_choiceCharacters->text());
			}
		}
	}
}



void AutoRigViewer::applyAutoRig( int riggingType /*= 0*/ )
{
	if (!_choicePawns->text())
		return;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string pawnName = _choicePawns->text();	
	SBAutoRigManager& autoRigManager = SBAutoRigManager::singleton();



	SmartBody::SBPawn* sbPawn = scene->getPawn(pawnName);	
	DeformableMeshInstance* meshInstance = sbPawn->dStaticMeshInstance_p;
	if (!sbPawn || !meshInstance || meshInstance->getDeformableMesh() == NULL)
	{

		LOG("AutoRigging Fail : No pawn is selected, or the selected pawn does not contain 3D mesh for rigging.");
		return;
	}
	DeformableMesh* mesh = meshInstance->getDeformableMesh();	

	std::string modelName = mesh->getName();//(const char*) model.name;
	std::string filebasename = boost::filesystem::basename(modelName);
	std::string fileextension = boost::filesystem::extension(modelName);
	std::string skelName = filebasename+".sk";
	std::string deformMeshName = filebasename+"AutoRig.dae"; 
	LOG("Start Build Auto Rigging");
	bool autoRigSuccess = autoRigManager.buildAutoRiggingFromPawnMesh(pawnName, riggingType, skelName, deformMeshName);
	LOG("Auto Rigging Donw");
#if 0

	SrModel& model = mesh->dMeshStatic_p[0]->shape();		
	SrModel scaleModel = SrModel(model);
	SmartBody::SBAssetManager* assetManager = SmartBody::SBScene::getScene()->getAssetManager();
	bool autoRigSuccess = false;
	SrMat worldRotation = sbPawn->get_world_offset().get_rotation(); 
	if (!assetManager->getDeformableMesh(deformMeshName))
	{			
		//model.scale(meshInstance->getMeshScale()); // resize the vertices
		float meshScale = meshInstance->getMeshScale();
		for (int i=0;i<scaleModel.V.size();i++)
			scaleModel.V[i] *= meshScale;
		for (int i=0;i<scaleModel.V.size();i++)
			scaleModel.V[i] = scaleModel.V[i]*worldRotation;

		if (riggingType == 0)
			autoRigSuccess = autoRigManager.buildAutoRiggingVoxels(scaleModel,skelName,deformMeshName);
		//autoRigSuccess = autoRigManager.buildAutoRiggingVoxelsWithVoxelSkinWeights(scaleModel,skelName,deformMeshName);
		else if (riggingType == 1)
			autoRigSuccess = autoRigManager.buildAutoRiggingVoxelsWithVoxelSkinWeights(scaleModel,skelName,deformMeshName);
		else if (riggingType == 2)
			autoRigSuccess = autoRigManager.buildAutoRigging(scaleModel, skelName, deformMeshName);		
	}
	else
	{
		LOG("Deformable mesh %s already exists. Skip auto-rigging and create the character directly.");
	}
#endif

	if (!autoRigSuccess && riggingType == 2)		
	{
		std::string errorMsg = "AutoRigging Fail : The input mesh must be a single component and water tight mesh. Try to enable 'voxelRigging'.";
		LOG(errorMsg.c_str());
		fl_alert(errorMsg.c_str());
		return;
	}		

	std::string charName = sbPawn->getName()+"autoRig";

	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	SmartBody::SBJointMap* jointMap = jointMapManager->getJointMap(skelName);
	if (!jointMap)
	{
		jointMap = jointMapManager->createJointMap(skelName);
		jointMap->guessMapping(scene->getSkeleton(skelName), false);
	}

	SmartBody::SBSkeleton* skel = scene->createSkeleton(skelName);

	SmartBody::SBCharacter* character = scene->createCharacter(charName, "");
	character->setSkeleton(skel);
	character->createStandardControllers();
	character->setStringAttribute("deformableMesh",deformMeshName);	

	SrVec dest = sbPawn->getPosition();
	float yOffset = -skel->getBoundingBox().a.y;
	dest.y = yOffset;		
	character->setPosition(SrVec(dest.x,dest.y,dest.z));
	character->setStringAttribute("displayType","GPUmesh");


	// setup behavior set
	SmartBody::SBBehaviorSetManager* manager = scene->getBehaviorSetManager();
	if (manager->getNumBehaviorSets() == 0)
	{
		// look for the behavior set directory under the media path
		scene->addAssetPath("script", "behaviorsets");
		scene->runScript("default-behavior-sets.py");

		if (manager->getNumBehaviorSets() == 0)
		{
			LOG("Can not find any behavior sets under path %s/behaviorsets.", scene->getMediaPath().c_str());
		}
		else
		{
			LOG("Found %d behavior sets under path %s/behaviorsets", manager->getNumBehaviorSets(), scene->getMediaPath().c_str());
		}
	}
#define TEST_ROCKETBOX 1
#if TEST_ROCKETBOX
	scene->addAssetPath("script", "scripts");
	scene->run("scene.run('characterUnitTest.py')");

	character->createActionAttribute("_1testHead", true, "TestHead", 300, false, false, false, "Test Head");
	character->createActionAttribute("_2testGaze", true, "TestHead", 300, false, false, false, "Test Head");
	character->createActionAttribute("_3testGesture", true, "TestHead", 300, false, false, false, "Test Head");
	character->createActionAttribute("_4testReach", true, "TestHead", 300, false, false, false, "Test Head");
	character->createActionAttribute("_5testLocomotion", true, "TestHead", 300, false, false, false, "Test Head");
#endif	
	//updateCharacterList();	
	scene->removePawn(pawnName);	

	
	if (retargetStepWindow)
	{
		retargetStepWindow->refreshAll();		
		retargetStepWindow->setApplyType(true);

		retargetStepWindow->setCharacterName(charName);
		retargetStepWindow->setJointMapName(skelName);	
	}
	
}

void AutoRigViewer::ApplyAutoRigCB( Fl_Widget* widget, void* data )
{
	AutoRigViewer* viewer = (AutoRigViewer*) data;
	int riggingType = viewer->_choiceVoxelRigging->value();
	viewer->applyAutoRig(riggingType);
}
