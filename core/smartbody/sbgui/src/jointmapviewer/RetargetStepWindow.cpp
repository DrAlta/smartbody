#include "vhcl.h"
#include "RetargetStepWindow.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBPawn.h>
#include <sbm/sbm_deformable_mesh.h>
#include <bml/bml.hpp>
#include <sb/SBSkeleton.h>
#include <sb/SBJointMap.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBBehaviorSet.h>
#include <sb/SBBehaviorSetManager.h>
#include <sb/SBAssetManager.h>

#include <autorig/SBAutoRigManager.h>
#include <boost/filesystem.hpp>

#include <FL/fl_ask.H>
RetargetStepWindow::RetargetStepWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(w, h, name)
{	
	int yDis = 10;
	int yOffset = 20;
	int xOffset = 10;

	int tabGroupX = 10;
	int tabGroupY = 50;
	int tabGroupW = w - 20;
	int tabGroupH = h - 120;

	int childGroupX = 0;
	int childGroupY = 3 * yDis ;
	int childGroupW = tabGroupW- 10;
	int childGroupH = tabGroupH - 4 * yDis;

	int windowGroupX = 20;
	int windowGroupY = 1 * yDis ;
	int windowGroupW = tabGroupW- 30;
	int windowGroupH = tabGroupH - 5 * yDis;

	jointMapViewer = NULL;
	retargetViewer = NULL;
	_removeCharacterName = "";
	_removePawnName = "";

	_choiceCharacters = new Fl_Choice(110, yDis, 150, 20, "Character");
	_choiceCharacters->callback(CharacterCB, this);
	updateCharacterList();

	_choicePawns = new Fl_Choice(410, yDis, 150, 20, "Mesh Pawn");
	//_choicePawns->callback(CharacterCB, this);
	updatePawnList();

	_buttonAutoRig = new Fl_Button(570, yDis, 120, 25, "Apply AutoRig");
	_buttonAutoRig->callback(ApplyAutoRigCB, this);
	//_choiceCharacters->callback(CharacterCB,this);

	_buttonVoxelRigging = new Fl_Check_Button(710, yDis, 120, 25, "Voxel Rigging");

	tabGroup = new Fl_Tabs(tabGroupX, tabGroupY, tabGroupW, tabGroupH);
	//tabGroup->callback(changeTabGroup, this);
	tabGroup->begin();
		Fl_Group* firstGroup = new Fl_Group(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, "Joint Mapper");
		firstGroup->begin();	
		jointMapViewer = new JointMapViewer(windowGroupX + childGroupX + tabGroupX, windowGroupY + childGroupY + tabGroupY , windowGroupW, windowGroupH,"Joint Mapper");
		jointMapViewer->begin();
		jointMapViewer->end();
		firstGroup->end();

		Fl_Group* secondGroup = new Fl_Group(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, "Behavior Set");
		secondGroup->begin();
		retargetViewer = new RetargetViewer(windowGroupX + childGroupX + tabGroupX, windowGroupY + childGroupY + tabGroupY, windowGroupW, windowGroupH, "Behavior Set");		
		retargetViewer->begin();
		retargetViewer->end();
		secondGroup->end();
	tabGroup->end();
	this->resizable(tabGroup);
	//this->size_range(600, 740);
	
	jointMapViewer->rootWindow = this;
	retargetViewer->rootWindow = this;

	int curY = tabGroupH + 60;	
	_buttonApplyAll = new Fl_Button(100, curY, 120, 25, "Apply All");
	_buttonApplyAll->callback(ApplyCB, this);
// 	_buttonApplyMap = new Fl_Button(100, curY, 120, 25, "Apply Joint Map");
// 	_buttonApplyMap->callback(ApplyJointMapCB,this);
// 	_buttonApplyBehaviorSet = new Fl_Button(220, curY, 120, 25, "Apply Behavior Set");
// 	_buttonApplyBehaviorSet->callback(ApplyBehaviorSetCB,this);			
	_buttonCancel = new Fl_Button(260, curY, 120, 25, "Cancel");
	_buttonCancel->callback(CancelCB, this);

	_buttonRefresh = new Fl_Button(420, curY, 120, 25, "Refresh");
	_buttonRefresh->callback(RefreshCB, this);

	retargetViewer->setShowButton(false);
	jointMapViewer->setShowButton(false);
	/*
	Fl_Group* firstGroup = new Fl_Group(xOffset, yOffset, w/2 - 20, h - 20, "Joint Mapper");
	firstGroup->begin();		
	jointMapViewer = new JointMapViewer(xOffset,yOffset, w/2 - 20, h - 20, "");
	firstGroup->resizable(jointMapViewer);
	firstGroup->end();
	this->resizable(firstGroup);

	//yOffset =  h/4+30; 
	xOffset += w/2+20;	
	Fl_Group* secondGroup = new Fl_Group(xOffset, yOffset, w/2 - 40 , h - 20, "Behavior Selection");
	secondGroup->begin();
	retargetViewer = new RetargetViewer(xOffset, yOffset, w/2 - 40 , h - 20, "");		
	secondGroup->end();
	*/
	//secondGroup->resizable(retargetViewer);
	//this->resizable(secondGroup);
	//this->size_range(800, 480);
	
	for (int c = 0; c < _choiceCharacters->size(); c++)
	{
		if (_choiceCharacters->text(c))
		{
			_choiceCharacters->value(c);
			setCharacterName(_choiceCharacters->text());
			break;
		}
	}
}



void RetargetStepWindow::RefreshCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->refreshAll();
}


void RetargetStepWindow::refreshAll()
{
	updatePawnList();
	updateCharacterList();
	jointMapViewer->updateUI();
	retargetViewer->updateBehaviorSet();
	redraw();
}

void RetargetStepWindow::setApplyType( bool applyAll )
{
	if (applyAll)
	{
		_buttonApplyAll->show();
		_buttonCancel->show();
		retargetViewer->setShowButton(false);
		jointMapViewer->setShowButton(false);
		//_buttonApplyBehaviorSet->hide();
		//_buttonApplyMap->hide();

	}
	else 
	{
		_buttonApplyAll->hide();
		_buttonCancel->hide();
		retargetViewer->setShowButton(true);
		jointMapViewer->setShowButton(true);
		//_buttonApplyBehaviorSet->show();
		//_buttonApplyMap->show();
	}
}

RetargetStepWindow::~RetargetStepWindow()
{

}

void RetargetStepWindow::updateCharacterList()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	int oldValue = _choiceCharacters->value();
	std::string oldCharacterName = "";
	if (oldValue >= 0 && oldValue < _choiceCharacters->size())
	{
		if (_choiceCharacters->text(oldValue))
			oldCharacterName = _choiceCharacters->text(oldValue);
	}
	const std::vector<std::string>& characters = scene->getCharacterNames();
	_choiceCharacters->clear();		
	bool hasOldCharacter = false;
	int charCount = 0;
	for (size_t c = 0; c < characters.size(); c++)
	{
		if (characters[c] == _removeCharacterName) // don't add remove character name
			continue;

		_choiceCharacters->add(characters[c].c_str());		
		if (characters[c] == oldCharacterName)
		{
			_choiceCharacters->value(charCount);
			hasOldCharacter = true;
		}	
		charCount++;
	}
	if (!hasOldCharacter) // no character, set to default character
	{
		std::string newCharName = "";
		for (int c = 0; c < _choiceCharacters->size(); c++)
		{
			if (_choiceCharacters->text(c))
			{
				_choiceCharacters->value(c);
				newCharName = _choiceCharacters->text(c);
			}
		}
		setCharacterName(newCharName);
	}	
// 	if (oldValue < _choiceCharacters->size() && oldValue >= 0)
// 	{
// 		_choiceCharacters->value(oldValue);
// 	}
	
}

void RetargetStepWindow::updatePawnList()
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
		if (pawns[c] == _removePawnName) // don't add remove character name
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
// 	if (oldValue < _choicePawns->size() && oldValue >= 0)
// 	{
// 		_choicePawns->value(oldValue);
// 	}
}

void RetargetStepWindow::setCharacterName( std::string charName )
{
	_charName = charName;
	for (int c = 0; c < _choiceCharacters->size(); c++)
	{
		if (charName == _choiceCharacters->text(c))
		{
			_choiceCharacters->value(c);
			break;
		}
	}

	if (jointMapViewer)
		jointMapViewer->setCharacterName(_charName);
	if (retargetViewer)
		retargetViewer->setCharacterName(_charName);
}


// void RetargetStepWindow::setSkeletonName( std::string skName )
// {
// 	retargetViewer->setSkeletonName(skName);
// }

void RetargetStepWindow::setJointMapName( std::string jointMapName )
{
	jointMapViewer->setJointMapName(jointMapName);
}


void RetargetStepWindow::applyAutoRig(bool voxelRigging)
{
	if (!_choicePawns->text())
		return;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string pawnName = _choicePawns->text();
	SmartBody::SBPawn* sbPawn = scene->getPawn(pawnName);	
	DeformableMeshInstance* meshInstance = sbPawn->dStaticMeshInstance_p;
	if (!sbPawn || !meshInstance || meshInstance->getDeformableMesh() == NULL)
	{
		
		LOG("AutoRigging Fail : No pawn is selected, or the selected pawn does not contain 3D mesh for rigging.");
		return;
	}
	
	SBAutoRigManager& autoRigManager = SBAutoRigManager::singleton();
	

	DeformableMesh* mesh = meshInstance->getDeformableMesh();	
	SrModel& model = mesh->dMeshStatic_p[0]->shape();	
	
	SrModel scaleModel = SrModel(model);

	std::string modelName = (const char*) model.name;
	std::string filebasename = boost::filesystem::basename(modelName);
	std::string fileextension = boost::filesystem::extension(modelName);
	std::string skelName = filebasename+".sk";
	std::string deformMeshName = filebasename+"AutoRig.dae"; 

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

		if (voxelRigging)
			autoRigSuccess = autoRigManager.buildAutoRiggingVoxels(scaleModel,skelName,deformMeshName);
		else
			autoRigSuccess = autoRigManager.buildAutoRigging(scaleModel, skelName, deformMeshName);

		if (!autoRigSuccess && !voxelRigging)		
		{
			std::string errorMsg = "AutoRigging Fail : The input mesh must be a single component and water tight mesh. Try to enable 'voxelRigging'.";
			LOG(errorMsg.c_str());
			fl_alert(errorMsg.c_str());
			return;
		}		
	}
	else
	{
		LOG("Deformable mesh %s already exists. Skip auto-rigging and create the character directly.");
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
	this->refreshAll();		
	this->setApplyType(true);
	setCharacterName(charName);
	setJointMapName(skelName);	
}


void RetargetStepWindow::applyRetargetSteps()
{
	jointMapViewer->applyJointMap();
	retargetViewer->RetargetCB(NULL,retargetViewer);
}


void RetargetStepWindow::CharacterCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;	
	Fl_Choice* charChoice = dynamic_cast<Fl_Choice*>(widget);	
	viewer->setCharacterName(charChoice->text());
	viewer->redraw();
}


void RetargetStepWindow::ApplyCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->applyRetargetSteps();
	viewer->hide();	
}

void RetargetStepWindow::CancelCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->hide();
}

void RetargetStepWindow::draw()
{
	if (jointMapViewer)
		jointMapViewer->redraw();
	Fl_Double_Window::draw();
}

void RetargetStepWindow::ApplyJointMapCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->jointMapViewer->applyJointMap();
	viewer->hide();
}

void RetargetStepWindow::ApplyBehaviorSetCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->retargetViewer->RetargetCB(NULL,viewer->retargetViewer);
	viewer->hide();
}

void RetargetStepWindow::ApplyAutoRigCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	bool useVoxelRigging = viewer->_buttonVoxelRigging->value();
	viewer->applyAutoRig(useVoxelRigging);
}

void RetargetStepWindow::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	refreshAll();
}

void RetargetStepWindow::OnCharacterDelete( const std::string & name )
{
	_removeCharacterName = name;
	refreshAll();
	_removeCharacterName = "";
}

void RetargetStepWindow::OnCharacterUpdate( const std::string & name )
{

}

void RetargetStepWindow::OnPawnCreate( const std::string & name )
{
	refreshAll();
}

void RetargetStepWindow::OnPawnDelete( const std::string & name )
{
	_removePawnName = name;
	refreshAll();
	_removePawnName = "";
}

void RetargetStepWindow::hide()
{
	SBWindowListener::windowHide();
	Fl_Double_Window::hide();	
}

void RetargetStepWindow::show()
{
	SBWindowListener::windowShow();
	Fl_Double_Window::show();
	refreshAll();
}