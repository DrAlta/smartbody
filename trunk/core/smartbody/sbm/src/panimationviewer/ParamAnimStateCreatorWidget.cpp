#include "ParamAnimStateCreatorWidget.h"
#include <sbm/SBAnimationState.h>
#include <sbm/SBAnimationStateManager.h>
#include <sbm/SBScene.h>

int PAStateCreator::lastNameIndex = 0;

PAStateCreator::PAStateCreator(PAStateEditor* editor, bool createMode, std::string stateName, int x, int y, int w, int h) : Fl_Window(x, y, w, h)
{
	set_modal();
	isCreateMode = createMode;
	stateEditor = editor;
	xDis = 10;
	yDis = 10;
	this->label("Edit State");
	this->begin();

	int csx = xDis;
	int csy = 4 * yDis;

	inputStateName = new Fl_Input(xDis + csx + 20, yDis, 10 * xDis, 2 * yDis, "Name");
	if (isCreateMode)
	{
		inputStateName->value(getUniqueStateName("state").c_str());
		
	}
	else
	{
		inputStateName->value(stateName.c_str());
	}

	choiceStateType = new Fl_Choice(xDis + csx, 2 * yDis + csy, 150, 25, "State Type");
	choiceStateType->add("0D");
	choiceStateType->add("1D");
	choiceStateType->add("2D");
	choiceStateType->add("3D");
	choiceStateType->value(0);
	if (!isCreateMode)
	{
		SmartBody::SBAnimationStateManager* stateManager = SmartBody::SBScene::getScene()->getStateManager();
		SmartBody::SBAnimationState* currentState = stateManager->getState(stateName);
		if (currentState)
		{
			SBAnimationState0D* state0d = dynamic_cast<SBAnimationState0D*>(currentState);
			if (state0d)
				choiceStateType->value(0);
			SBAnimationState1D* state1d = dynamic_cast<SBAnimationState1D*>(currentState);
			if (state1d)
				choiceStateType->value(1);
			SBAnimationState2D* state2d = dynamic_cast<SBAnimationState2D*>(currentState);
			if (state2d)
				choiceStateType->value(2);
			SBAnimationState3D* state3d = dynamic_cast<SBAnimationState3D*>(currentState);
			if (state3d)
				choiceStateType->value(3);
		}
		choiceStateType->deactivate();
	}

	animationList = new Fl_Multi_Browser(xDis + csx, 4 * yDis + csy, 150, 250, "All Motions");
	stateAnimationList = new Fl_Multi_Browser(xDis + csx + 250, 4 * yDis + csy, 150, 250, "Motions in State");
	animationAdd = new Fl_Button(xDis + csx + 160, 4 * yDis + csy + 50, 50, 25, ">>>");
	animationAdd->callback(addMotion, this);
	animationRemove = new Fl_Button(xDis + csx + 160, 4 * yDis + csy + 100, 50, 25,  "<<<");
	animationRemove->callback(removeMotion, this);

	if (isCreateMode)
		buttonCreateState = new Fl_Button(xDis + csx, 4 * yDis + csy + 300, 60, 25, "Create State");
	else
		buttonCreateState = new Fl_Button(xDis + csx, 4 * yDis + csy + 300, 60, 25, "Save Changes");

	buttonCreateState->callback(createState, this);
	buttonCancelState = new Fl_Button(xDis + csx + 100, 4 * yDis + csy + 300, 60, 25, "Cancel");
	buttonCancelState->callback(cancelState, this);

	this->end();

	loadMotions();
	if (!isCreateMode)
	{
		SmartBody::SBAnimationStateManager* stateManager = SmartBody::SBScene::getScene()->getStateManager();
		SmartBody::SBAnimationState* state = stateManager->getState(stateName);
		if (!state)
		{
			fl_alert("State %s does not exist.", stateName.c_str());
			cancelState(this, NULL);
		}
		std::vector<SkMotion*>& motions = state->motions;
		for (std::vector<SkMotion*>::iterator iter = motions.begin();
			 iter != motions.end();
			 iter++)
		{
			stateAnimationList->add((*iter)->getName().c_str());
		}
	}
}

PAStateCreator::~PAStateCreator()
{
}

void PAStateCreator::loadMotions()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	std::vector<std::string> motionNames = scene->getMotionNames();
	std::map<std::string, SkMotion*>::iterator iter;
	for (std::vector<std::string>::iterator iter = motionNames.begin();
		iter != motionNames.end();
		iter++)
		animationList->add(iter->c_str());
}

void PAStateCreator::addMotion(Fl_Widget* widget, void* data)
{
	PAStateCreator* creator = (PAStateCreator*) data;
	for (int i = 0; i < creator->animationList->size(); i++)
	{
		if (creator->animationList->selected(i+1))
		{
			bool shouldAdd = true;
			for (int j = 0; j < creator->stateAnimationList->size(); j++)
			{
				if (strcmp(creator->stateAnimationList->text(j+1), creator->animationList->text(i+1)) == 0)
				{
					shouldAdd = false;
					break;
				}
			}
			if (shouldAdd)
				creator->stateAnimationList->add(creator->animationList->text(i+1));
		}
	}
}

void PAStateCreator::removeMotion(Fl_Widget* widget, void* data)
{
	PAStateCreator* creator = (PAStateCreator*) data;

	std::vector<std::string> selectedMotions;
	for (int i = 0; i < creator->stateAnimationList->size(); i++)
	{
		if (creator->stateAnimationList->selected(i+1))
		{
			selectedMotions.push_back(creator->stateAnimationList->text(i + 1));
			creator->stateAnimationList->remove(i + 1);
			i--;
		}
	}
}

std::string PAStateCreator::getUniqueStateName(std::string prefix)
{
	SmartBody::SBAnimationStateManager* stateManager = SmartBody::SBScene::getScene()->getStateManager();
	
	std::stringstream strstr;

	bool isUnique = false;
	while (!isUnique)
	{
		isUnique = true;

        strstr.str("");
		strstr << prefix << lastNameIndex;
		lastNameIndex++;

		SBAnimationState* state = stateManager->getState(strstr.str());
		if (state)
			lastNameIndex++;
	}

	return strstr.str();
} 

void PAStateCreator::createState(Fl_Widget* widget, void* data)
{
	PAStateCreator* creator = (PAStateCreator*) data;

	// get the animation type and name
	std::string type = creator->choiceStateType->menu()[creator->choiceStateType->value()].label();
	std::string stateName = creator->inputStateName->value();

	// make sure the name is valid
	SmartBody::SBAnimationStateManager* stateManager = SmartBody::SBScene::getScene()->getStateManager();
	SmartBody::SBAnimationState* state = stateManager->getState(stateName);

	if (creator->isCreateMode)
	{
		if (state)
		{
			fl_alert("State name %s already exists. Please choose a different name.", stateName.c_str());
			return;
		}

		if (type == "0D")
		{
			SmartBody::SBAnimationState0D* animState = stateManager->createState0D(stateName);
			
			for (int i = 0; i < creator->stateAnimationList->size(); i++)
			{
				animState->addMotion(creator->stateAnimationList->text(i + 1));
			}
		}
		else if (type == "1D")
		{
			SmartBody::SBAnimationState1D* animState = stateManager->createState1D(stateName);

			for (int i = 0; i < creator->stateAnimationList->size(); i++)
			{
				animState->addMotion(creator->stateAnimationList->text(i + 1), 0);
			}

			
		}
		else if (type == "2D")
		{
			SmartBody::SBAnimationState2D* animState = stateManager->createState2D(stateName);

			for (int i = 0; i < creator->stateAnimationList->size(); i++)
			{
				animState->addMotion(creator->stateAnimationList->text(i + 1), 0, 0);
			}
		}
		else if (type == "3D")
		{
			SmartBody::SBAnimationState3D* animState = stateManager->createState3D(stateName);

			for (int i = 0; i < creator->stateAnimationList->size(); i++)
			{
				animState->addMotion(creator->stateAnimationList->text(i + 1), 0, 0, 0);
			}
		}
	}
	else // edit mode
	{
		// compare motions in state against motions list and add or remove accordingly
		std::vector<std::string> updatedMotions;
		for (int i = 0; i < creator->stateAnimationList->size(); i++)
		{
			updatedMotions.push_back(creator->stateAnimationList->text(i + 1));
		}

		std::vector<SkMotion*>& motions = state->motions;

		std::set<std::string> updatedMap;
		for (size_t u = 0; u < updatedMotions.size(); u++)
		{
			updatedMap.insert(updatedMotions[u]);
		}

		std::set<std::string> motionMap;
		for (size_t m = 0; m < motions.size(); m++)
		{
			motionMap.insert(motions[m]->getName());
		}

		for (std::set<std::string>::iterator newMotionIter = updatedMap.begin();
			 newMotionIter != updatedMap.end();
			 newMotionIter++)
		{
			std::set<std::string>::iterator foundIter = motionMap.find((*newMotionIter));
			if (foundIter == motionMap.end())
			{
				// new motion to be added to the state
				SmartBody::SBAnimationState0D* state0D = dynamic_cast<SBAnimationState0D*>(state);	
				SmartBody::SBAnimationState1D* state1D = dynamic_cast<SBAnimationState1D*>(state);
				SmartBody::SBAnimationState2D* state2D = dynamic_cast<SBAnimationState2D*>(state);
				SmartBody::SBAnimationState3D* state3D = dynamic_cast<SBAnimationState3D*>(state);
				if (state0D)
				{
					state0D->addMotion((*newMotionIter));
				}
				else if (state1D)
				{
					state1D->addMotion((*newMotionIter), 0);
				}
				else if (state2D)
				{
					state2D->addMotion((*newMotionIter), 0, 0);
				}
				else if (state3D)
				{
					state3D->addMotion((*newMotionIter), 0, 0, 0);
				}
			}
		}

		for (std::set<std::string>::iterator toDeleteIter = motionMap.begin();
			 toDeleteIter != motionMap.end();
			 toDeleteIter++)
		{
			std::set<std::string>::iterator foundIter = updatedMap.find((*toDeleteIter));
			if (foundIter == updatedMap.end())
			{
				// motion to be removed from the state
				SmartBody::SBAnimationState0D* state0D = dynamic_cast<SBAnimationState0D*>(state);	
				SmartBody::SBAnimationState1D* state1D = dynamic_cast<SBAnimationState1D*>(state);
				SmartBody::SBAnimationState2D* state2D = dynamic_cast<SBAnimationState2D*>(state);
				SmartBody::SBAnimationState3D* state3D = dynamic_cast<SBAnimationState3D*>(state);
				if (state0D)
				{
					state0D->removeMotion((*toDeleteIter));
				}
				else if (state1D)
				{
					state1D->removeMotion((*toDeleteIter));
				}
				else if (state2D)
				{
					state2D->removeMotion((*toDeleteIter));
				}
				else if (state3D)
				{
					state3D->removeMotion((*toDeleteIter));
				}

			}
		}
	}

	creator->stateEditor->refresh();
	creator->hide();
	
}

void PAStateCreator::cancelState(Fl_Widget* widget, void* data)
{
	PAStateCreator* creator = (PAStateCreator*) data;
	creator->stateEditor->refresh();
	creator->hide();
}
