#include "vhcl.h"
#include "JointMapViewer.h"
#include <sb/SBJointMapManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <sstream>
#include <cstring>

#ifndef WIN32
#define _strdup strdup
#endif

const std::string spineJointNames[7] = {"base","spine1", "spine2", "spine3", "spine4", "spine5", "skullbase" };
const std::string leftArmJointNames[5] = {"l_sternoclavicular","l_shoulder", "l_elbow", "l_forearm", "l_wrist" };
const std::string rightArmJointNames[5] = {"r_sternoclavicular","r_shoulder", "r_elbow", "r_forearm", "r_wrist" };
const std::string leftHandJointNames[20] = {"l_thumb1","l_thumb2", "l_thumb3", "l_thumb4", "l_index1", "l_index2","l_index3","l_index4","l_middle1", "l_middle2", "l_middle3", "l_middle4", "l_ring1", "l_ring2", "l_ring3","l_ring4", "l_pinky1", "l_pinky2", "l_pinky3","l_pinky4"};
const std::string rightHandJointNames[20] = {"r_thumb1","r_thumb2", "r_thumb3", "r_thumb4", "r_index1", "r_index2","r_index3","r_index4","r_middle1", "r_middle2", "r_middle3", "r_middle4", "r_ring1", "r_ring2", "r_ring3","r_ring4","r_pinky1", "r_pinky2", "r_pinky3","r_pinky4"};

const std::string leftLegJointNames[5] = { "l_hip", "l_knee", "l_ankle", "l_forefoot", "l_toe" };
const std::string rightLegJointNames[5] = { "r_hip", "r_knee", "r_ankle", "r_forefoot", "r_toe" };

JointMapViewer::JointMapViewer(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h, name)
{

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	begin();	
	int curY = 10;
	_choiceJointMaps = new Fl_Choice(110, curY, 150, 20, "JointMaps");
	//choiceCharacters->callback(CharacterCB, this);
	//std::vector<std::string> characters = scene->getCharacterNames();
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	std::vector<std::string> jointMapNames = jointMapManager->getJointMapNames();	
	for (size_t c = 0; c < jointMapNames.size(); c++)
	{
		_choiceJointMaps->add(jointMapNames[c].c_str());
	}
	_choiceJointMaps->callback(SelectMapCB,this);

	curY += 25;

	_choiceCharacters = new Fl_Choice(110, curY, 150, 20, "Character");
	//choiceCharacters->callback(CharacterCB, this);
	std::vector<std::string> characters = scene->getCharacterNames();
	for (size_t c = 0; c < characters.size(); c++)
	{
		_choiceCharacters->add(characters[c].c_str());
	}	
	_choiceCharacters->callback(SelectCharacterCB,this);
	curY += 25;
// 
// 
// 	SmartBody::SBBehaviorSetManager* behavMgr = SmartBody::SBScene::getScene()->getBehaviorSetManager();
// 	std::map<std::string, SmartBody::SBBehaviorSet*>& behavSets = behavMgr->getBehaviorSets();
// 
	
	for (int i=0;i<7;i++) standardJointNames.push_back(spineJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(leftArmJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(rightArmJointNames[i]);
	for (int i=0;i<20;i++) standardJointNames.push_back(leftHandJointNames[i]);
	for (int i=0;i<20;i++) standardJointNames.push_back(rightHandJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(leftLegJointNames[i]);
	for (int i=0;i<5;i++) standardJointNames.push_back(rightLegJointNames[i]);

	curY += 25;
	
 	_scrollGroup = new Fl_Scroll(10, curY, this->w() - 20, 450, "");
 	_scrollGroup->type(Fl_Scroll::VERTICAL);
 	_scrollGroup->begin();
 
	int scrollY = curY;
 	for (unsigned int i=0;i<standardJointNames.size();i++)
	{
 		std::string name = standardJointNames[i];
		//LOG("joint name = %s",name.c_str());
 		//Fl_Check_Button* check = new Fl_Check_Button(20, curY, 100, 20, _strdup(name.c_str()));
		//Fl_Group* jointMapGroup = new Fl_Group(20, curY , 200, 20, _strdup(name.c_str()));
		//Fl_Input* input = new Fl_Input(100 , scrollY, 150, 20, _strdup(name.c_str()));
		Fl_Input_Choice* choice = new Fl_Input_Choice(100, scrollY, 150, 20, _strdup(name.c_str()));
		_jointChoiceList.push_back(choice);
 		choice->input()->when(FL_WHEN_CHANGED);
 		choice->input()->callback(JointNameChange,this);
 		choice->menubutton()->when(FL_WHEN_CHANGED);
 		choice->menubutton()->callback(JointNameChange,this);
 		scrollY += 25;
 	}
	
	_scrollGroup->end();
	curY += 450  + 25;
	Fl_Button* buttonApply = new Fl_Button(100, curY, 60, 20, "Apply Map");
	buttonApply->callback(ApplyMapCB, this);
	Fl_Button* buttonCancel = new Fl_Button(180, curY, 60, 20, "Cancel");
	buttonCancel->callback(CancelCB, this);
	end();

	if (jointMapNames.size() > 0) 
	{
		_choiceJointMaps->value(0);
		updateSelectMap();
	}

	if (characters.size() > 0)
	{		
		_choiceCharacters->value(0);	
		updateCharacter();
	}	
}


JointMapViewer::~JointMapViewer()
{
}

void JointMapViewer::JointNameChange( Fl_Widget* widget, void* data )
{
	Fl_Input* input = dynamic_cast<Fl_Input*>(widget);
	Fl_Menu_Button* menuButton = dynamic_cast<Fl_Menu_Button*>(widget);

	Fl_Input_Choice* inputChoice = NULL;
	if (input)
		inputChoice = dynamic_cast<Fl_Input_Choice*>(input->parent());
	if (menuButton)
		inputChoice = dynamic_cast<Fl_Input_Choice*>(menuButton->parent());
	if (inputChoice)
	{
		JointMapViewer* viewer = (JointMapViewer*) data;
		viewer->updateJointName(inputChoice);
	}	
}


void JointMapViewer::updateJointName( Fl_Input_Choice* jointChoice )
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string charName = _choiceCharacters->text();
	SmartBody::SBCharacter* curChar = scene->getCharacter(charName);
	SmartBody::SBSkeleton* charSk = curChar->getSkeleton();		
	int valueIndex = jointChoice->menubutton()->value();	
	std::string choiceStr = "";
	if (valueIndex >= 0)
	{
		jointChoice->value(valueIndex);
		choiceStr = jointChoice->value();
		if (choiceStr == "--empty--")
			jointChoice->value("");
	}
	jointChoice->clear();
	jointChoice->add("--empty--"); // add empty string as the first choice
	std::string filterLabel = jointChoice->value();
	for (unsigned int i=0;i<skeletonJointNames.size();i++)
	{
		std::string& jname = skeletonJointNames[i];
		if (jname.find(filterLabel) != std::string::npos)
		{
			jointChoice->add(jname.c_str());			
		}
	}		
	if (valueIndex >= 0)
	{
		if (choiceStr == "--empty--")
			jointChoice->value("");
		else
			jointChoice->value(choiceStr.c_str());
	}	
}

void JointMapViewer::updateCharacter()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string charName = _choiceCharacters->text();
	SmartBody::SBCharacter* curChar = scene->getCharacter(charName);
	SmartBody::SBSkeleton* charSk = curChar->getSkeleton();
	skeletonJointNames = charSk->getJointNames();

	int numChildren = _scrollGroup->children();
	for (int i=0;i<numChildren;i++)
	{
		Fl_Input_Choice* input = dynamic_cast<Fl_Input_Choice*>(_scrollGroup->child(i));
		if (!input)
		{
			continue;
		}	
		updateJointName(input);
// 		input->clear();
// 		for (unsigned int k=0;k<jointNames.size();k++)
// 		{
// 			input->add(jointNames[k].c_str());
// 		}		
	}
}

void JointMapViewer::updateSelectMap()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string jointMapName = _choiceJointMaps->text();
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	SmartBody::SBJointMap* jointMap = jointMapManager->getJointMap(jointMapName);
	if (!jointMap) return;

	int numChildren = _scrollGroup->children();
	for (int i=0;i<numChildren;i++)
	{
		Fl_Input_Choice* input = dynamic_cast<Fl_Input_Choice*>(_scrollGroup->child(i));
		if (input)
		{
			std::string targetName = input->label();
			std::string sourceName = jointMap->getMapSource(targetName);
			if (sourceName == "")
			{
				input->value("");
			}
			else
			{
				input->value(sourceName.c_str());
			}
		}
	}
}


void JointMapViewer::applyJointMap()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::string jointMapName = _choiceJointMaps->text();
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	SmartBody::SBJointMap* jointMap = jointMapManager->getJointMap(jointMapName);

	SmartBody::SBCharacter* curChar = scene->getCharacter(_choiceCharacters->text());
	if (!curChar || !jointMap) return;
	int numChildren = _scrollGroup->children();
	jointMap->clearMapping();
	for (int i=0;i<numChildren;i++)
	{
		Fl_Input_Choice* input = dynamic_cast<Fl_Input_Choice*>(_scrollGroup->child(i));
		if (input)
		{
			std::string targetName = input->label();
			std::string sourceName = input->value();		
			jointMap->setMapping(sourceName, targetName);
		}
	}	
	SmartBody::SBSkeleton* sceneSk = scene->getSkeleton(curChar->getSkeleton()->getName());
	jointMap->applySkeleton(sceneSk);
	jointMap->applySkeleton(curChar->getSkeleton());	
}

void JointMapViewer::SelectMapCB( Fl_Widget* widget, void* data )
{
	JointMapViewer* viewer = (JointMapViewer*) data;
	viewer->updateSelectMap();
	viewer->updateCharacter();	
}


void JointMapViewer::SelectCharacterCB( Fl_Widget* widget, void* data )
{
	JointMapViewer* viewer = (JointMapViewer*) data;	
	viewer->updateCharacter();	
}


void JointMapViewer::ApplyMapCB(Fl_Widget* widget, void* data)
{
	JointMapViewer* viewer = (JointMapViewer*) data;
	viewer->applyJointMap();
	viewer->updateCharacter();

// 	SmartBody::SBBehaviorSetManager* behavMgr = SmartBody::SBScene::getScene()->getBehaviorSetManager();
// 
// 	// run the script associated with any checked behavior sets
// 	int numChildren = viewer->_scrollGroup->children();
// 	for (int c = 0; c < numChildren; c++)
// 	{
// 		Fl_Check_Button* check = dynamic_cast<Fl_Check_Button*>(viewer->_scrollGroup->child(c));
// 		if (check)
// 		{
// 			if (check->value())
// 			{
// 				SmartBody::SBBehaviorSet* behavSet = behavMgr->getBehaviorSet(check->label());
// 				if (behavSet)
// 				{
// 					LOG("Retargetting %s...", check->label());
// 					const std::string& script = behavSet->getScript();
// 					SmartBody::SBScene::getScene()->runScript(script.c_str());
// 					std::stringstream strstr;
// 					strstr << "setupBehaviorSet()";
// 					SmartBody::SBScene::getScene()->run(strstr.str());
// 					std::stringstream strstr2;
// 					strstr2 << "retargetBehaviorSet('" << viewer->getCharacterName() << "', '" << viewer->getSkeletonName() << "')";
// 					SmartBody::SBScene::getScene()->run(strstr2.str());
// 				}
// 			}
// 		}
// 	}
	viewer->hide();
}

void JointMapViewer::CancelCB(Fl_Widget* widget, void* data)
{
	JointMapViewer* viewer = (JointMapViewer*) data;

	viewer->hide();
}
