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
#include <sbm/SBAnimationState.h>
#include <sbm/SBAnimationStateManager.h>
#include <FL/fl_file_chooser.H>
#include "ParamAnimStateCreatorWidget.h"

PAStateEditor::PAStateEditor(int x, int y, int w, int h, PanimationWindow* window) : Fl_Group(x, y, w, h), paWindow(window)
{
	lastNameIndex = 0;

	this->label("State Editor");
	this->begin();

		int csx = xDis + x;
		int csy = 4 * yDis + y;
		stateSelectionGroup = new Fl_Group(csx, csy, w - 2 * xDis, h /2 - 4 * yDis, "Create States");
		int stateSelectionGroupW = w - 2 * xDis;
		int stateSelectionGroupH = h /2 - 3 * yDis;
		stateSelectionGroup->begin();
			stateList = new Fl_Choice(xDis + 50 + csx, yDis + csy, 150, 2 * yDis, "State");
//			stateList->when(FL_WHEN_ENTER_KEY);
			stateList->callback(changeStateList, this);
			choiceStateType = new Fl_Choice(xDis + 300 + csx, yDis + csy, 10 * xDis, 2 * yDis, "Type");
			choiceStateType->add("---");
			choiceStateType->add("0D");
			choiceStateType->add("1D");
			choiceStateType->add("2D");
			choiceStateType->add("3D");
			choiceStateType->value(0);
			choiceStateType->deactivate();

			createStateButton = new Fl_Button(xDis + 400 + csx, yDis + csy, 10 * xDis, 2 * yDis, "Create");
			createStateButton->callback(editStateMotions, this);
			stateAnimationList = new Fl_Multi_Browser(xDis + csx, 4 * yDis + csy, stateSelectionGroupW / 2 - 5 * xDis, stateSelectionGroupH - 5 * yDis - 30, "State Motions");
			stateAnimationList->align(FL_ALIGN_TOP);
			stateAnimationList->when(FL_WHEN_CHANGED);
			stateAnimationList->callback(selectStateAnimations, this);
			animationAdd = new Fl_Button(stateSelectionGroupW / 2 - 3 * xDis + csx, stateSelectionGroupH / 2 + csy, 6 * xDis, 2 * yDis, ">>>");
//			animationAdd->callback(addMotion, this);
			animationRemove = new Fl_Button(stateSelectionGroupW / 2 - 3 * xDis + csx, stateSelectionGroupH / 2 + 5 * yDis + csy, 6 * xDis, 2 * yDis, "<<<");
//			animationRemove->callback(removeMotion, this);
			shapeList = new Fl_Multi_Browser(stateSelectionGroupW / 2 + 4 * xDis + csx, 4 * yDis + csy, stateSelectionGroupW / 2 - 5 * xDis, stateSelectionGroupH - 5 * yDis, "Parameterization");
			shapeList->align(FL_ALIGN_TOP);
		//	shapeList->when(FL_WHEN_CHANGED);
		//	shapeList->callback(selectStateAnimations, this);

			inputParameterX = new Fl_Float_Input(xDis + 80 + csx, 4 *yDis + csy + stateSelectionGroupH - 5 * yDis - 30 + 10, 60, 2* yDis - 5, "Parameters");
			
			inputParameterY = new Fl_Float_Input(xDis + 150 + csx, 4 *yDis + csy + stateSelectionGroupH - 5 * yDis - 30 + 10, 60, 2* yDis - 5, "");
			inputParameterZ = new Fl_Float_Input(xDis + 220 + csx, 4 *yDis + csy + stateSelectionGroupH - 5 * yDis - 30 + 10, 60, 2* yDis - 5, "");
			inputParameterX->callback(updateParameters, this);
			inputParameterY->callback(updateParameters, this);
			inputParameterZ->callback(updateParameters, this);
			inputParameterX->deactivate();
			inputParameterY->deactivate();
			inputParameterZ->deactivate();

			checkPlay = new Fl_Check_Button(xDis + 360 + csx,  4 *yDis + csy + stateSelectionGroupH - 5 * yDis - 30 + 10, 30, 2* yDis - 5, "Play");
			checkPlay->callback(playmotion, this);
			sliderScrub = new Fl_Value_Slider(xDis + 400 + csx,  4 *yDis + csy + stateSelectionGroupH - 5 * yDis - 30 + 10, 300, 2* yDis - 5, "");
			sliderScrub->type(FL_HORIZONTAL);
			sliderScrub->range(0, 1);
			sliderScrub->callback(scrub, this);
			sliderScrub->deactivate();

		stateSelectionGroup->end();
		stateSelectionGroup->box(FL_BORDER_BOX);

		int esx = xDis;
		int esy = h / 2 + yDis + y;
		Fl_Group* buttonGroup = new Fl_Group(esx, esy, w - 2 * xDis, 3 * yDis);
		buttonGroup->begin();
			addMark = new Fl_Button(xDis + esx, yDis + esy, 50, 2 * yDis, "+");
			addMark->callback(addStateTimeMark, this);
			removeMark = new Fl_Button(xDis + 50 + esx, yDis + esy, 50, 2 * yDis, "-");
			removeMark->callback(removeStateTimeMark, this);
			updateMark = new Fl_Button(xDis + 100 + esx, yDis + esy, 100, 2 * yDis, "Update");
			updateMark->callback(updateStateTimeMark, this);
			buttonSave = new Fl_Button(xDis + 300 + esx, yDis + esy, 100, 2 * yDis, "Save");
			buttonSave->callback(save, this);
			maxTimeInput = new Fl_Float_Input(xDis + 600 + esx, yDis + esy, 100, 2 * yDis, "Max Time");
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

	stateList->activate();
	loadStates();

	creator = NULL;
}

PAStateEditor::~PAStateEditor()
{
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
			{
				editor->stateEditorNleModel->removeTrack(editor->stateEditorNleModel->getTrack(i)->getName());
				i--;
			}
		}
	}
	editor->paWindow->redraw();
}

void PAStateEditor::editStateMotions(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;

	if (editor->creator)
		delete editor->creator;
	bool isCreateMode = true;
	std::string stateName = "";
	if (std::string("Edit State") == editor->createStateButton->label())
	{
		isCreateMode = false;
		stateName = editor->stateList->menu()[editor->stateList->value()].label();
	}
	editor->creator = new PAStateCreator(editor, isCreateMode, stateName, editor->x() + 50, editor->y() + 50, 600, 600);
	editor->creator->show();
	/*
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string stateName = editor->newStateName->value();
	if (stateName == "")
	{
		fl_message("Please input new state name");
		return;
	}
	if (mcu.lookUpPAState(stateName))
	{
		LOG("State %s already exists. Please choose another name, or remove the other state first.", stateName.c_str());
		return;
	}
	std::stringstream createStateCommand;
	int numMotion = editor->stateEditorNleModel->getNumTracks();
	//int cycleWindow = fl_choice((char*) "This is a Cycle State?", "yes", "no", NULL);
	//std::string cycle = "true";
	//if (cycleWindow == 1) cycle = "false";
	//createStateCommand << "panim state " << stateName << " cycle " << cycle << " " << numMotion << " ";
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

	PAState* newStateData = new PAState(stateName);
	editor->updateCorrespondenceMarks(newStateData);
	editor->stateTimeMarkWidget->setup();
	editor->paWindow->redraw();
	*/
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
	PAState* currentState = mcu.lookUpPAState(currentStateName);
	if (currentState)
	{
		editor->createStateButton->label("Edit State");
		// determine the state type
		SBAnimationState0D* state0d = dynamic_cast<SBAnimationState0D*>(currentState);
		if (state0d)
			editor->choiceStateType->value(1);
		SBAnimationState1D* state1d = dynamic_cast<SBAnimationState1D*>(currentState);
		if (state1d)
			editor->choiceStateType->value(2);
		SBAnimationState2D* state2d = dynamic_cast<SBAnimationState2D*>(currentState);
		if (state2d)
			editor->choiceStateType->value(3);
		SBAnimationState3D* state3d = dynamic_cast<SBAnimationState3D*>(currentState);
		if (state3d)
			editor->choiceStateType->value(4);

		editor->inputParameterX->deactivate();
		editor->inputParameterY->deactivate();
		editor->inputParameterZ->deactivate();

		editor->sliderScrub->deactivate();


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

		// add the motions to the motion list
		editor->stateAnimationList->clear();
		for (int i = 0; i < currentState->getNumMotions(); i++)
		{
			SkMotion* motion = currentState->motions[i];
			editor->stateAnimationList->add(motion->getName().c_str());
		}
	}
	else
	{
		editor->createStateButton->label("Create State");
		editor->choiceStateType->value(0);
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
	PAState* currentState = NULL;
	if (editor->stateList->active())
		currentState = mcuCBHandle::singleton().lookUpPAState(currentStateName);
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

void PAStateEditor::updateCorrespondenceMarks(PAState* state)
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
		if (block1->getNumMarks() != block2->getNumMarks())
			continue;

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
	int origStateValue = stateList->value();
	stateEditorNleModel->removeAllTracks();
	stateTimeMarkWidget->setup();
	stateAnimationList->clear();
	shapeList->clear();
	loadStates();
	stateList->value(origStateValue);
	changeStateList(stateList, this);
}

void PAStateEditor::changeStateType(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;

}

void PAStateEditor::save(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;

	//std::string mediaPath = SmartBody::SBScene::getScene()->getMediaPath();




	const char* stateFileName = fl_file_chooser("State file:", "*.py", NULL);
	if (!stateFileName)
		return;

	std::string stateName = editor->stateList->text();

	SmartBody::SBAnimationState* state = SmartBody::SBScene::getScene()->getStateManager()->getState(stateName);
	SmartBody::SBAnimationState0D* state0D = dynamic_cast<SmartBody::SBAnimationState0D*>(state);
	SmartBody::SBAnimationState1D* state1D = dynamic_cast<SmartBody::SBAnimationState1D*>(state);
	SmartBody::SBAnimationState2D* state2D = dynamic_cast<SmartBody::SBAnimationState2D*>(state);
	SmartBody::SBAnimationState3D* state3D = dynamic_cast<SmartBody::SBAnimationState3D*>(state);

	std::string stateNameVariable = "state" + stateName;
	std::stringstream strstr;
	strstr << "# state " << stateName << "\n";
	strstr << "# autogenerated by SmartBody\n";
	strstr << "\n";
	strstr << "stateManager = scene.getStateManager()\n";
	// create the state
	if (state0D)
	{
		strstr << "\n";
		strstr << stateNameVariable << " = stateManager.createState0D(\"" << stateName << "\")\n";
	}
	if (state1D || state2D || state3D)
	{
		strstr << "\n";
		// add the motions
		if (state1D)
		{
			strstr << stateNameVariable << " = stateManager.createState1D(\"" << stateName << "\")\n";
		}
		else if (state2D)
		{
			strstr << stateNameVariable << " = stateManager.createState2D(\"" << stateName << "\")\n";
		}
		else if (state3D)
		{
			strstr << stateNameVariable << " = stateManager.createState3D(\"" << stateName << "\")\n";
		}

		strstr << "\n";
		strstr << "motions = StringVec()\n";
		for (int x = 0; x < state->getNumMotions(); x++)
		{
			strstr << "motions.append(\"" << state->getMotion(x) << "\")\n";
		}
		// add the parameters
		strstr << "\n";
		if (state1D || state2D || state3D)
		{
			strstr << "paramsX = DoubleVec()\n";
		}
		if (state2D || state3D)
		{
			strstr << "paramsY = DoubleVec()\n";
		}
		if (state3D)
		{
			strstr << "paramsZ = DoubleVec()\n";
		}
		
		for (int x = 0; x < state->getNumMotions(); x++)
		{
			double p1, p2, p3;
			if (state1D)
			{
				state->getParameter(state->getMotion(x), p1);
				strstr << "paramsX.append(" << p1 << ")\n";
			}
			else if (state2D)
			{
				state->getParameter(state->getMotion(x), p1, p2);
				strstr << "paramsX.append(" << p1 << ")\n";
				strstr << "paramsY.append(" << p2 << ")\n";
			}
			else if (state3D)
			{
				state->getParameter(state->getMotion(x), p1, p2, p3);
				strstr << "paramsX.append(" << p1 << ")\n";
				strstr << "paramsY.append(" << p2 << ")\n";
				strstr << "paramsZ.append(" << p3 << ")\n";
			}
			
		}
		strstr << "for i in range(0, len(motions)):\n";
		if (state1D)
		{
			strstr << "\t" << stateNameVariable << ".addMotion(motions[i], paramsX[i])\n";
		}
		else if (state2D)
		{
			strstr << "\t" << stateNameVariable << ".addMotion(motions[i], paramsX[i], paramsY[i])\n";
		}
		else if (state3D)
		{
			strstr << "\t" << stateNameVariable << ".addMotion(motions[i], paramsX[i], paramsY[i], paramsZ[i])\n";
		}
		// add the correspondance points
		strstr << "\n";
		for (int c = 0; c < state->getNumKeys(); c++)
		{
			strstr << "points" << c << " = DoubleVec()\n";
			for (int m = 0; m < state->getNumMotions(); m++)
			{
				strstr << "points" << c << ".append(" << state->keys[m][c] << ")\n";
			}
			strstr << stateNameVariable << ".addCorrespondancePoints(motions, points" << c << ")\n";
		}

	}
	if (state2D)
	{
		// create the triangles
		strstr << "\n";
		std::vector<TriangleInfo>& triangleInfo = state->getTriangles();
		for (size_t t = 0; t < triangleInfo.size(); t++)
		{
			strstr << stateNameVariable << ".addTriangle(\"" << triangleInfo[t].motion1 << "\", \"" <<  triangleInfo[t].motion2 << "\", \"" <<  triangleInfo[t].motion3 << "\")\n"; 
		}
	}
	if (state3D)
	{
		// create the tetrahedrons
		strstr << "\n";
		std::vector<TetrahedronInfo>& tetrahedronInfo = state->getTetrahedrons();
		for (size_t t = 0; t < tetrahedronInfo.size(); t++)
		{
			strstr << stateNameVariable << ".addTetrahedron(\"" << tetrahedronInfo[t].motion1 << "\", \"" <<  tetrahedronInfo[t].motion2 << "\", \"" <<  tetrahedronInfo[t].motion3 << "\", \"" <<  tetrahedronInfo[t].motion4 << "\")\n"; 
		}
	}

	// save to the file
	std::ofstream stateFile(stateFileName);
	if (stateFile.is_open() != true)
	{
		fl_alert("Problem writing to file %s, state was not saved.", stateFileName);
		return;
	}
	stateFile << strstr.str();
	stateFile.close();

	/*
	// output to the window
	Fl_Text_Display* display = editor->paWindow->textDisplay;	
	display->insert(strstr.str().c_str());
	display->insert("\n");
	display->redraw();
	display->show_insert_position();
	*/
}

void PAStateEditor::selectStateAnimations(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;

	std::vector<std::string> selectedMotions;

	for (int i = 0; i < editor->stateAnimationList->size(); i++)
	{
		if (editor->stateAnimationList->selected(i+1))
		{
			selectedMotions.push_back(editor->stateAnimationList->text(i + 1));
		}
	}

	if (selectedMotions.size() == 1)
	{
		SmartBody::SBMotion* motion = SmartBody::SBScene::getScene()->getMotion(selectedMotions[0]);
		if (motion)
		{
			if (editor->checkPlay->value())
			{
				editor->sliderScrub->activate();
			}
			else
			{
				editor->sliderScrub->activate();
			}
			editor->sliderScrub->range(0, motion->duration());
		}
		
		const char* stateText = editor->stateList->text();
		PAState* currentState = NULL;
		if (editor->stateList->active())
			currentState = mcuCBHandle::singleton().lookUpPAState(stateText);
		if (!currentState)
			return;

		SmartBody::SBAnimationState0D* state0D = dynamic_cast<SmartBody::SBAnimationState0D*>(currentState);
		SmartBody::SBAnimationState1D* state1D = dynamic_cast<SmartBody::SBAnimationState1D*>(currentState);
		SmartBody::SBAnimationState2D* state2D = dynamic_cast<SmartBody::SBAnimationState2D*>(currentState);
		SmartBody::SBAnimationState3D* state3D = dynamic_cast<SmartBody::SBAnimationState3D*>(currentState);
		
		char buff[32];
		if (state0D)
		{
			editor->inputParameterX->deactivate();
			editor->inputParameterY->deactivate();
			editor->inputParameterZ->deactivate();
		}
		else if (state1D)
		{
			double x;
			state1D->getParameter(selectedMotions[0], x); 
			sprintf(buff, "%.2f", x);
			editor->inputParameterX->value(buff);

			editor->inputParameterX->activate();
			editor->inputParameterY->deactivate();
			editor->inputParameterZ->deactivate();
		}
		else if (state2D)
		{
			double x, y;
			state2D->getParameter(selectedMotions[0], x, y); 
			sprintf(buff, "%.2f", x);
			editor->inputParameterX->value(buff);
			sprintf(buff, "%.2f", y);
			editor->inputParameterY->value(buff);

			editor->inputParameterX->activate();
			editor->inputParameterY->activate();
			editor->inputParameterZ->deactivate();
		}
		else if (state3D)
		{
			double x, y, z;
			state3D->getParameter(selectedMotions[0], x, y, z); 
			sprintf(buff, "%.2f", x);
			editor->inputParameterX->value(buff);
			sprintf(buff, "%.2f", y);
			editor->inputParameterY->value(buff);
			sprintf(buff, "%.2f", z);
			editor->inputParameterZ->value(buff);

			editor->inputParameterX->activate();
			editor->inputParameterY->activate();
			editor->inputParameterZ->activate();
		}
	}
	else
	{
		editor->sliderScrub->deactivate();
		
		editor->inputParameterX->value(0);
		editor->inputParameterY->value(0);
		editor->inputParameterZ->value(0);
		editor->inputParameterX->deactivate();
		editor->inputParameterY->deactivate();
		editor->inputParameterZ->deactivate();
	}

}

void PAStateEditor::updateParameters(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;

	std::vector<std::string> selectedMotions;

	for (int i = 0; i < editor->stateAnimationList->size(); i++)
	{
		if (editor->stateAnimationList->selected(i+1))
		{
			selectedMotions.push_back(editor->stateAnimationList->text(i + 1));
		}
	}

	const char* stateText = editor->stateList->text();
	PAState* currentState = NULL;
	if (editor->stateList->active())
		currentState = mcuCBHandle::singleton().lookUpPAState(stateText);
	if (!currentState)
		return;


	SmartBody::SBAnimationState0D* state0D = dynamic_cast<SmartBody::SBAnimationState0D*>(currentState);
	SmartBody::SBAnimationState1D* state1D = dynamic_cast<SmartBody::SBAnimationState1D*>(currentState);
	SmartBody::SBAnimationState2D* state2D = dynamic_cast<SmartBody::SBAnimationState2D*>(currentState);
	SmartBody::SBAnimationState3D* state3D = dynamic_cast<SmartBody::SBAnimationState3D*>(currentState);
	
	if (state0D)
	{
		editor->inputParameterX->deactivate();
		editor->inputParameterY->deactivate();
		editor->inputParameterZ->deactivate();
	}
	else if (state1D)
	{
		state1D->setParameter(selectedMotions[0], (float) atof(editor->inputParameterX->value())); 
	}
	else if (state2D)
	{
		state2D->setParameter(selectedMotions[0], (float) atof(editor->inputParameterX->value()), (float) atof(editor->inputParameterY->value())); 
	}
	else if (state3D)
	{
		state3D->setParameter(selectedMotions[0], (float) atof(editor->inputParameterX->value()), (float) atof(editor->inputParameterY->value()), (float) atof(editor->inputParameterZ->value())); 
	}
}

void PAStateEditor::scrub(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;

	std::vector<std::string> selectedMotions;

	for (int i = 0; i < editor->stateAnimationList->size(); i++)
	{
		if (editor->stateAnimationList->selected(i+1))
		{
			selectedMotions.push_back(editor->stateAnimationList->text(i + 1));
		}
	}

	if (selectedMotions.size() == 1)
	{
		SmartBody::SBMotion* motion = SmartBody::SBScene::getScene()->getMotion(selectedMotions[0]);
		double time = editor->sliderScrub->value();
		double delta = motion->duration() / double(motion->frames() - 1);
		int frameNumber = int(time / delta);
		std::string charName = editor->paWindow->characterList->menu()[editor->paWindow->characterList->value()].label();
		std::stringstream command;
		command << "motionplayer " << charName << " " << selectedMotions[0] << " " << frameNumber;
		SmartBody::SBScene::getScene()->command(command.str());
	}

}

void PAStateEditor::playmotion(Fl_Widget* widget, void* data)
{
	PAStateEditor* editor = (PAStateEditor*) data;

	PAStateEditor::selectStateAnimations(widget, data);

	std::string charName = editor->paWindow->characterList->menu()[editor->paWindow->characterList->value()].label();

	std::stringstream command;
	if (editor->checkPlay->value())
		command << "motionplayer " << charName << " on";
	else
		command << "motionplayer " << charName << " off";
	SmartBody::SBScene::getScene()->command(command.str());
}




