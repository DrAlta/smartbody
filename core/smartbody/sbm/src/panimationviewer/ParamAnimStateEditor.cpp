/*
 *  ParamAnimStateEditor.cpp - part of SmartBody-lib's Test Suite
 *  Copyright (C) 2009  University of Southern California
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
 *      Yuyu Xu, USC
 */

#include "ParamAnimStateEditor.h"
#include <sbm/mcontrol_util.h>
#include "ParamAnimBlock.h"

PAStateEditor::PAStateEditor(int x, int y, int w, int h, PanimationWindow* window) : Fl_Group(x, y, w, h), paWindow(window)
{
	this->label("State Editor");
	this->begin();
		stateEditorMode = new Fl_Check_Button(xDis + x, yDis + y, 200, 2 * yDis, "Create State Mode");
		stateEditorMode->callback(changeStateEditorMode, this);

		int csx = xDis + x;
		int csy = 4 * yDis + y;
		createStateGroup = new Fl_Group(csx, csy, w - 2 * xDis, h /2 - 4 * yDis, "new state");
		int createStateGroupW = w - 2 * xDis;
		int createStateGroupH = h /2 - 4 * yDis;
		createStateGroup->begin();
			createStateButton = new Fl_Button(xDis + csx, yDis + csy, 10 * xDis, 2 * yDis, "Create State");
			createStateButton->callback(createNewState, this);
			newStateName = new Fl_Input(createStateGroupW -  11 * xDis + csx, yDis + csy, 10 * xDis, 2 * yDis, "New State Name:");
			animationList = new Fl_Multi_Browser(xDis + csx, 4 * yDis + csy, createStateGroupW / 2 - 5 * xDis, createStateGroupH - 5 * yDis);
			stateAnimationList = new Fl_Multi_Browser(createStateGroupW / 2 + 4 * xDis + csx, 4 * yDis + csy, createStateGroupW / 2 - 5 * xDis, createStateGroupH - 5 * yDis);
			animationAdd = new Fl_Button(createStateGroupW / 2 - 3 * xDis + csx, createStateGroupH / 2 + csy, 6 * xDis, 2 * yDis, ">>>");
			animationAdd->callback(addMotion, this);
			animationRemove = new Fl_Button(createStateGroupW / 2 - 3 * xDis + csx, createStateGroupH / 2 + 5 * yDis + csy, 6 * xDis, 2 * yDis, "<<<");
			animationRemove->callback(removeMotion, this);
		createStateGroup->end();
		createStateGroup->box(FL_BORDER_BOX);

		int esx = xDis + x;
		int esy = h / 2 + yDis + y;
		Fl_Group* buttonGroup = new Fl_Group(esx, esy, w - 2 * xDis, 3 * yDis);
		buttonGroup->begin();
			stateList = new Fl_Choice(10 * xDis + esx, yDis + esy, 100, 2 * yDis, "State List");
//			stateList->when(FL_WHEN_ENTER_KEY);
			stateList->callback(changeStateList, this);
			addMark = new Fl_Button(12 * xDis + 100 + esx, yDis + esy, 100, 2 * yDis, "Add Mark");
			addMark->callback(addStateTimeMark, this);
			removeMark = new Fl_Button(13 * xDis + 200+ esx, yDis + esy, 100, 2 * yDis, "Delete Mark");
			removeMark->callback(removeStateTimeMark, this);
			updateMark = new Fl_Button(14 * xDis + 300+ esx, yDis + esy, 100, 2 * yDis, "Update Mark");
			updateMark->callback(updateStateTimeMark, this);
			maxTimeInput = new Fl_Float_Input(14 * xDis + 500+ esx, yDis + esy, 100, 2 * yDis, "Max Time");
			maxTimeInput->callback(updateMaxTime, this);
#ifdef AUTO_FOOTSTEP_MARK
			autoFootStepMarks = new Fl_Button(15 * xDis + 400+ esx, yDis + esy, 100, 2 * yDis, "Auto Footsteps");
			autoFootStepMarks->callback(addFootStepMark, this);
#endif
		buttonGroup->end();

		editStateTimeMarkGroup = new Fl_Scroll(esx, esy + 3 * yDis + 10, w - 2 * xDis, h / 2 - 5 * yDis - 10);
		editStateTimeMarkGroup->type(Fl_Scroll::VERTICAL_ALWAYS);
		editStateTimeMarkGroup->begin();
			stateTimeMarkWidget = new ParamAnimEditorWidget(2 * xDis + esx, 5 * yDis + esy, w - 5 * xDis, h / 2 - 6 * yDis, (char*) "");
			stateTimeMarkWidget->begin();
			stateTimeMarkWidget->end();
		editStateTimeMarkGroup->end();
		editStateTimeMarkGroup->resizable(stateTimeMarkWidget);
		editStateTimeMarkGroup->box(FL_BORDER_BOX);
		this->resizable(editStateTimeMarkGroup);
	this->end();

	stateEditorNleModel = new nle::NonLinearEditorModel();
	stateTimeMarkWidget->setModel(stateEditorNleModel);
	stateTimeMarkWidget->setup();

	stateEditorMode->value(true);
	createStateGroup->activate();
	stateList->deactivate();
	loadMotions();
	loadStates();
}

PAStateEditor::~PAStateEditor()
{
}

void PAStateEditor::loadMotions()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkMotion*>::iterator iter;
	for (iter = mcu.motion_map.begin(); iter != mcu.motion_map.end(); iter++)
		animationList->add(iter->first.c_str());
}

void PAStateEditor::loadStates()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	stateList->clear();
	stateList->add("---");
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
		stateList->add(mcu.param_anim_states[i]->stateName.c_str());
	stateList->value(0);
}

void PAStateEditor::changeStateEditorMode(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	editor->refresh();
	if (editor->stateEditorMode->value())		// create mode
	{
		editor->createStateGroup->activate();
		editor->stateList->deactivate();
	}
	else										// edit mode
	{
		editor->createStateGroup->deactivate();
		editor->stateList->activate();
	}
}

void PAStateEditor::updateStateTimeMarkEditor(Fl_Widget* widget, void* data, bool toAdd)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (editor->stateAnimationList->size() == 0)
		editor->stateEditorNleModel->removeAllTracks();

	if (toAdd)
	{
		for (int i = 0; i < editor->stateAnimationList->size(); i++)
		{
			std::string motionName = editor->stateAnimationList->text(i+1);//text(i+1);
			nle::Track* track = editor->stateEditorNleModel->getTrack(motionName);
			if (!track && toAdd)
			{
				ParamAnimTrack* newTrack = new ParamAnimTrack();
				newTrack->setName(motionName.c_str());
				std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(motionName);
				ParamAnimBlock* block = new ParamAnimBlock();
				block->setName(motionName.c_str());
				block->setStartTime(0);
				block->setEndTime(iter->second->duration());
				editor->stateEditorNleModel->addTrack(newTrack);
				newTrack->addBlock(block);		
				editor->stateEditorNleModel->update();
				editor->stateTimeMarkWidget->setViewableTimeEnd(editor->stateEditorNleModel->getEndTime());
			}
		}
	}
	else
	{
		std::string motionName;
		for (int i = 0; i < editor->stateEditorNleModel->getNumTracks(); i++)
		{
			bool del = true;
			for (int j = 0; j < editor->stateAnimationList->size(); j++)
			{
				std::string motionName = editor->stateAnimationList->text(j + 1);
				if (editor->stateEditorNleModel->getTrack(i)->getName() == motionName)
				{
					del = false;
					break;
				}
			}
			if (del)
				editor->stateEditorNleModel->removeTrack(editor->stateEditorNleModel->getTrack(i)->getName());
		}
	}
	editor->paWindow->redraw();
}

void PAStateEditor::createNewState(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string stateName = editor->newStateName->value();
	if (stateName == "")
	{
		fl_message("Please input new state name");
		return;
	}
	if (mcu.lookUpPAState(stateName))
	{
		LOG("mcu_panim_test_cmd_func ERR: State %s exist.", stateName.c_str());
		return;
	}
	std::stringstream createStateCommand;
	int numMotion = editor->stateEditorNleModel->getNumTracks();
	int cycleWindow = fl_choice((char*) "This is a Cycle State?", "yes", "no", NULL);
	std::string cycle = "true";
	if (cycleWindow == 1) cycle = "false";
	createStateCommand << "panim state " << stateName << " cycle " << cycle << " " << numMotion << " ";
	for (int i = 0; i < numMotion; i++)
		createStateCommand << editor->stateEditorNleModel->getTrack(i)->getName() << " ";
	if (numMotion == 0)
		return;
	int numKeys = editor->stateEditorNleModel->getTrack(0)->getBlock(0)->getNumMarks();
	createStateCommand << numKeys << " ";
	for (int i = 0; i < numMotion; i++)
		for (int j = 0; j < numKeys; j++)
			createStateCommand << editor->stateEditorNleModel->getTrack(i)->getBlock(0)->getMark(j)->getStartTime() << " ";
	editor->paWindow->execCmd(editor->paWindow, createStateCommand.str());

	PAStateData* newStateData = new PAStateData(stateName);
	editor->updateCorrespondenceMarks(newStateData);
	editor->stateTimeMarkWidget->setup();
	editor->paWindow->redraw();
}

void PAStateEditor::addMotion(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	for (int i = 0; i < editor->animationList->size(); i++)
	{
		if (editor->animationList->selected(i+1))
		{
			bool shouldAdd = true;
			for (int j = 0; j < editor->stateAnimationList->size(); j++)
			{
				if (strcmp(editor->stateAnimationList->text(j+1), editor->animationList->text(i+1)) == 0)
				{
					shouldAdd = false;
					break;
				}
			}
			if (shouldAdd)
				editor->stateAnimationList->add(editor->animationList->text(i+1));
		}
	}
	updateStateTimeMarkEditor(widget, data, true);
}

void PAStateEditor::removeMotion(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	bool cycle = true;
	while (cycle)
	{
		cycle = false;
		for (int i = 0; i < editor->stateAnimationList->size(); i++)
		{
			if (editor->stateAnimationList->selected(i+1))
			{
				editor->stateAnimationList->remove(i+1);
				cycle = true;
				break;
			}
		}
	}
	updateStateTimeMarkEditor(widget, data, false);
}

void PAStateEditor::changeStateList(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	int curValue = editor->stateList->value();
	editor->loadStates();
	editor->stateList->value(curValue);
	editor->stateEditorNleModel->removeAllTracks();
	std::string currentStateName = editor->stateList->menu()[editor->stateList->value()].label();
	PAStateData* currentState = mcu.lookUpPAState(currentStateName);
	if (currentState)
	{
		for (int i = 0; i < currentState->getNumMotions(); i++)
		{
			SkMotion* motion = currentState->motions[i];
			std::string motionName = motion->getName();
			ParamAnimTrack* track = new ParamAnimTrack();
			track->setName(motionName.c_str());
			ParamAnimBlock* block = new ParamAnimBlock();
			block->setName(motionName.c_str());
			block->setStartTime(0);
			block->setEndTime(motion->duration());

			editor->stateEditorNleModel->addTrack(track);
			track->addBlock(block);
		}
		editor->updateCorrespondenceMarks(currentState);
	}
	editor->stateTimeMarkWidget->setup();
	editor->paWindow->redraw();
}

void PAStateEditor::addFootStepMark( Fl_Widget* widget, void* data )
{
	LOG("Automatically add foot step marks");
	PAStateEditor* editor = (PAStateEditor*) data;

}

void PAStateEditor::updateMaxTime(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	editor->stateTimeMarkWidget->setViewableTimeEnd(atof(editor->maxTimeInput->value()));
	editor->paWindow->redraw();
}


void PAStateEditor::addStateTimeMark(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	editor->paWindow->addTimeMark(editor->stateEditorNleModel);
	editor->stateTimeMarkWidget->setup();
	editor->paWindow->redraw();
}

void PAStateEditor::removeStateTimeMark(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	editor->paWindow->removeTimeMark(editor->stateEditorNleModel);
	editor->stateTimeMarkWidget->setup();
	editor->paWindow->redraw();
}

void PAStateEditor::updateStateTimeMark(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;
	
	// check validation first
	int numOfMarks = -1;
	for (int i = 0; i < editor->stateEditorNleModel->getNumTracks(); i++)
	{
		nle::Block* block = editor->stateEditorNleModel->getTrack(i)->getBlock(0);
		if (i == 0)	numOfMarks = block->getNumMarks();
		else		
		{
			if (numOfMarks != block->getNumMarks())
			{
				LOG("PanimationWindow::updateStateTimeMark ERR: mark number for each block is not equal!");
				return;
			}
		}
	}

	int stateIdx = editor->stateList->value();

	if (stateIdx < 0)
	{
		LOG("PanimationWindow::updateStateTimeMark WARNING: no state selected.");
		return;
	}

	const char* stateText = editor->stateList->text();
	std::string currentStateName = editor->stateList->text(stateIdx);
	PAStateData* currentState = NULL;
	if (editor->stateList->active())
		currentState = mcuCBHandle::singleton().lookUpPAState(currentStateName);
	else
		currentState = mcuCBHandle::singleton().lookUpPAState(editor->newStateName->value());
	if (currentState)
	{
		for (int i = 0; i < editor->stateEditorNleModel->getNumTracks(); i++)
		{
			nle::Block* block = editor->stateEditorNleModel->getTrack(i)->getBlock(0);
			currentState->keys[i].clear();
			for (int j = 0; j < numOfMarks; j++)
			{
				double keyTime = block->getMark(j)->getStartTime();
				currentState->keys[i].push_back(keyTime);
			}
		}
		editor->updateCorrespondenceMarks(currentState);
		editor->paWindow->redraw();
	}
}

void PAStateEditor::updateCorrespondenceMarks(PAStateData* state)
{
	for (int i = 0; i < stateEditorNleModel->getNumTracks(); i++)
	{
		nle::Track* track = stateEditorNleModel->getTrack(i);
		nle::Block* block = track->getBlock(0);
		block->removeAllMarks();
	}

	for (int i = 0; i < state->getNumMotions(); i++)
	{
		nle::Track* track = stateEditorNleModel->getTrack(i);
		nle::Block* block = track->getBlock(0);

		for (int j = 0; j < state->getNumKeys(); j++)
		{
			CorrespondenceMark* mark = new CorrespondenceMark();
			mark->setStartTime(state->keys[i][j]);
			mark->setEndTime(mark->getStartTime());
			mark->setColor(FL_RED);
			char buff[256];
			sprintf(buff, "%6.2f", mark->getStartTime());
			mark->setName(buff);
			mark->setShowName(true);
			block->addMark(mark);
			mark->setSelected(false);
		}
	}

	for (int i = 0; i < stateEditorNleModel->getNumTracks() - 1; i++)
	{
		nle::Track* track1 = stateEditorNleModel->getTrack(i);
		nle::Track* track2 = stateEditorNleModel->getTrack(i + 1);
		nle::Block* block1 = track1->getBlock(0);
		nle::Block* block2 = track2->getBlock(0);
		for (int j = 0; j < block1->getNumMarks(); j++)
		{
			CorrespondenceMark* mark1 = dynamic_cast<CorrespondenceMark*> (block1->getMark(j));
			CorrespondenceMark* mark2 = dynamic_cast<CorrespondenceMark*> (block2->getMark(j));
			mark1->attach(mark2);
			mark2->attach(mark1);
		}
	}
}

void PAStateEditor::refresh()
{
	stateEditorNleModel->removeAllTracks();
	stateTimeMarkWidget->setup();
	stateAnimationList->clear();
	animationList->clear();
	loadMotions();
	loadStates();
}

