/*
 *  ParamAnimTransitionEditor.cpp - part of SmartBody-lib's Test Suite
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

#include "ParamAnimTransitionEditor.h"
#include <sbm/mcontrol_util.h>
#include "ParamAnimBlock.h"


PATransitionEditor::PATransitionEditor(int x, int y, int w, int h, PanimationWindow* window) : fltk::Group(x, y, w, h), paWindow(window)
{
	this->label("Transition Editor");
	this->begin();
		transitionEditorMode = new CheckButton(xDis, yDis, 200, 2 * yDis, "Create Transition Mode");
		transitionEditorMode->callback(changeTransitionEditorMode, this);
		
		createTransitionGroup = new Group(xDis, 4 * yDis, w - 2 * xDis, h /2 - 4 * yDis, "new transition");
		int createTransitionGroupW = w - 2 * xDis;
		int createTransitionGroupH = h /2 - 4 * yDis;
		createTransitionGroup->begin();
			createTransitionButton = new Button(xDis, yDis, 10 * xDis, 2 * yDis, "Create Transition");
			createTransitionButton->callback(createNewTransition, this);
			stateList1 = new Choice(xDis + 100, 5 * yDis, 100, 2 * yDis, "State1");
			stateList1->callback(changeStateList1, this);
			stateList1->when(fltk::WHEN_CHANGED);
			animForTransition1 = new Browser(xDis, 8 * yDis, createTransitionGroupW / 2 - 5 * xDis, createTransitionGroupH - 9 * yDis);
			animForTransition1->callback(changeAnimForTransition, this);
			stateList2 = new Choice(createTransitionGroupW / 2 + 4 * xDis + 100, 5 * yDis, 100, 2 * yDis, "State2");
			stateList2->callback(changeStateList2, this);
			stateList2->when(fltk::WHEN_CHANGED);
			animForTransition2 = new Browser(createTransitionGroupW / 2 + 4 * xDis, 8 * yDis, createTransitionGroupW / 2 - 5 * xDis, createTransitionGroupH - 9 * yDis);
			animForTransition2->callback(changeAnimForTransition, this);
		createTransitionGroup->end();
		createTransitionGroup->box(fltk::BORDER_BOX);
		this->add(createTransitionGroup);
		this->resizable(createTransitionGroup);

		editTransitionTimeMarkGroup = new Group(xDis, h / 2 + yDis, w - 2 * xDis, h / 2 - 2 * yDis, "time mark editor");
		editTransitionTimeMarkGroup->begin();
			transitionList = new Choice(10 * xDis, yDis, 100, 2 * yDis, "Transition List");
			transitionList->callback(changeTransitionList, this);
			transitionList->when(fltk::WHEN_CHANGED);
			addMark1 = new Button(12 * xDis + 100, yDis, 100, 2 * yDis, "Add Mark");
			addMark1->callback(addTransitionTimeMark, this);
			removeMark1 = new Button(13 * xDis + 200, yDis, 100, 2 * yDis, "Delete Mark");
			removeMark1->callback(removeTransitionTimeMark, this);
			updateMark1 = new Button(14 * xDis + 300, yDis, 100, 2 * yDis, "Update Mark");
			updateMark1->callback(updateTransitionTimeMark, this);
			transitionTimeMarkWidget = new ParamAnimEditorWidget(2 * xDis, 5 * yDis, w - 5 * xDis, h / 2 - 6 * yDis, "Transition Time Mark Editor");
		editTransitionTimeMarkGroup->end();
		editTransitionTimeMarkGroup->resizable(transitionTimeMarkWidget);
		editTransitionTimeMarkGroup->box(fltk::BORDER_BOX);
		this->add(editTransitionTimeMarkGroup);
		this->resizable(editTransitionTimeMarkGroup);
	this->end();

	transitionEditorNleModel = new nle::NonLinearEditorModel();
	transitionTimeMarkWidget->setModel(transitionEditorNleModel);

	ParamAnimTrack* track1 = new ParamAnimTrack();
	track1->setName("transition1");
	ParamAnimBlock* block1 = new ParamAnimBlock();
	track1->addBlock(block1);
	transitionEditorNleModel->addTrack(track1);

	ParamAnimTrack* track2 = new ParamAnimTrack();
	track2->setName("transition2");
	ParamAnimBlock* block2 = new ParamAnimBlock();
	track2->addBlock(block2);
	transitionEditorNleModel->addTrack(track2);
	transitionTimeMarkWidget->setup();

	transitionEditorMode->state(true);
	createTransitionGroup->activate();
	transitionList->deactivate();

	loadStates();
	loadTransitions();
}

PATransitionEditor::~PATransitionEditor()
{
}

void PATransitionEditor::loadStates()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	stateList1->remove_all();
	stateList2->remove_all();
	stateList1->add("---");
	stateList2->add("---");
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
	{
		stateList1->add(mcu.param_anim_states[i]->stateName.c_str());
		stateList2->add(mcu.param_anim_states[i]->stateName.c_str());
	}
}

void PATransitionEditor::loadTransitions()
{
	transitionList->remove_all();
	transitionList->add("---");
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (size_t i = 0; i < mcu.param_anim_transitions.size(); i++)
	{
		std::string transitionName = mcu.param_anim_transitions[i]->fromState->stateName + std::string(" | ") + mcu.param_anim_transitions[i]->toState->stateName;
		transitionList->add(transitionName.c_str());
	}
}

void PATransitionEditor::changeTransitionEditorMode(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	if (editor->transitionEditorMode->state())		// create mode
	{
		editor->createTransitionGroup->activate();
		editor->transitionList->deactivate();
	}
	else										// edit mode
	{
		editor->createTransitionGroup->deactivate();
		editor->transitionList->activate();
	}	
}

void PATransitionEditor::changeStateList1(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	editor->loadStates();
	PAStateData* state1 = mcuCBHandle::singleton().lookUpPAState(editor->stateList1->child(editor->stateList1->value())->label());
	editor->animForTransition1->clear();
	if (state1)
	{
		for (int i = 0; i < state1->getNumMotions(); i++)
			editor->animForTransition1->add(state1->motions[i]->name());
	}
	for (int i = 0; i < editor->animForTransition1->size(); i++)
		editor->animForTransition1->select(i, false);
}

void PATransitionEditor::changeStateList2(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	editor->loadStates();
	PAStateData* state2 = mcuCBHandle::singleton().lookUpPAState(editor->stateList2->child(editor->stateList2->value())->label());
	editor->animForTransition2->clear();
	if (state2)
	{
		for (int i = 0; i < state2->getNumMotions(); i++)
			editor->animForTransition2->add(state2->motions[i]->name());
	}
	for (int i = 0; i < editor->animForTransition2->size(); i++)
		editor->animForTransition2->select(i, false);
}

const double precisionCompensate = 0.0001;
void PATransitionEditor::changeAnimForTransition(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string motionName1 = "";
	for (int i = 0; i < editor->animForTransition1->size(); i++)
	{
		if (editor->animForTransition1->selected(i))
		{
			motionName1 = editor->animForTransition1->goto_index(i)->label();
			break;
		}
	}
	std::string motionName2 = "";
	for (int i = 0; i < editor->animForTransition2->size(); i++)
	{
		if (editor->animForTransition2->selected(i))
		{
			motionName2 = editor->animForTransition2->goto_index(i)->label();
			break;
		}
	}
	if (motionName1 != "")
	{
		std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(motionName1);
		nle::Block* block = editor->transitionEditorNleModel->getTrack(0)->getBlock(0);
		block->removeAllMarks();
		block->setName(motionName1);
		block->setStartTime(0);
		block->setEndTime(iter->second->duration() - precisionCompensate);
	}
	if (motionName2 != "")
	{
		std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(motionName2);
		nle::Block* block = editor->transitionEditorNleModel->getTrack(1)->getBlock(0);
		block->removeAllMarks();
		block->setName(motionName2);
		block->setStartTime(0);
		block->setEndTime(iter->second->duration() - precisionCompensate);		
	}

	if (motionName1 != "" && motionName2 != "")
	{
		nle::Block* block1 = editor->transitionEditorNleModel->getTrack(0)->getBlock(0);
		nle::Block* block2 = editor->transitionEditorNleModel->getTrack(1)->getBlock(0);
		std::string stateName1 = editor->stateList1->child(editor->stateList1->value())->label();
		std::string stateName2 = editor->stateList2->child(editor->stateList2->value())->label();
		PAStateData* fromState = mcu.lookUpPAState(stateName1);
		PAStateData* toState = mcu.lookUpPAState(stateName2);
		for (int i = 0; i < fromState->getNumMotions(); i++)
		{
			if (motionName1 == fromState->motions[i]->name())
			{
				int numKeys = fromState->getNumKeys();
				block1->setStartTime(fromState->keys[i][0]);
				block1->setEndTime(fromState->keys[i][numKeys - 1] - precisionCompensate);
			}
		}
		for (int i = 0; i < toState->getNumMotions(); i++)
		{
			if (motionName2 == toState->motions[i]->name())
			{
				int numKeys = toState->getNumKeys();
				block2->setStartTime(toState->keys[i][0]);
				block2->setEndTime(toState->keys[i][numKeys - 1] - precisionCompensate);
			}
		}
	}
	editor->transitionTimeMarkWidget->setup();
	editor->paWindow->redraw();
}

void PATransitionEditor::addTransitionTimeMark(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	editor->paWindow->addTimeMark(editor->transitionEditorNleModel, true);
	editor->transitionTimeMarkWidget->setup();
	editor->paWindow->redraw();
}

void PATransitionEditor::removeTransitionTimeMark(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	editor->paWindow->removeTimeMark(editor->transitionEditorNleModel);
	editor->transitionTimeMarkWidget->setup();
	editor->paWindow->redraw();
}

void PATransitionEditor::updateTransitionTimeMark(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	PATransitionData* transition = NULL;
	std::string fromStateName = "";
	std::string toStateName = "";
	if (!editor->transitionList->active())
	{
		fromStateName = editor->stateList1->child(editor->stateList1->value())->label();
		toStateName = editor->stateList2->child(editor->stateList2->value())->label();
		transition = mcuCBHandle::singleton().lookUpPATransition(fromStateName, toStateName);
	}
	else
	{
		std::string fullName = editor->transitionList->child(editor->transitionList->value())->label();
		if (fullName == "---")
		{
			editor->transitionEditorNleModel->getTrack(0)->getBlock(0)->setName("");
			editor->transitionEditorNleModel->getTrack(1)->getBlock(0)->setName("");
			return;
		}
		size_t seperateMarkPos = fullName.find("|");
		fromStateName = fullName.substr(0, seperateMarkPos);
		toStateName = fullName.substr(seperateMarkPos + 1, fullName.size() - 1);
		transition = mcuCBHandle::singleton().lookUpPATransition(fromStateName, toStateName);
	}
	if (transition)
	{
		nle::Block* block1 = editor->transitionEditorNleModel->getTrack(0)->getBlock(0);
		nle::Block* block2 = editor->transitionEditorNleModel->getTrack(1)->getBlock(0);		
		PAStateData* fromState = mcuCBHandle::singleton().lookUpPAState(fromStateName);
		PAStateData* toState = mcuCBHandle::singleton().lookUpPAState(toStateName);
		transition->fromState = fromState;
		transition->toState = toState;
		transition->fromMotionName = block1->getName();
		if (block1->getNumMarks() == 0)	
		{
			transition->easeOutStart.push_back(mcuCBHandle::singleton().lookUpMotion(transition->fromMotionName.c_str())->duration() - defaultTransition);
			transition->easeOutEnd.push_back(mcuCBHandle::singleton().lookUpMotion(transition->fromMotionName.c_str())->duration());
		}
		else
		{
			transition->easeOutStart.clear();
			transition->easeOutEnd.clear();
			for (int i = 0; i < block1->getNumMarks() / 2; i++)
			{
				transition->easeOutStart.push_back(block1->getMark(i * 2 + 0)->getStartTime());
				transition->easeOutEnd.push_back(block1->getMark(i * 2 + 1)->getStartTime());	
			}
		}
		transition->toMotionName = block2->getName();
		if (block2->getNumMarks() == 0)
		{
			transition->easeInStart = 0.0;
			transition->easeInEnd = defaultTransition;
		}
		else
		{
			transition->easeInStart = block2->getMark(0)->getStartTime();
			transition->easeInEnd = block2->getMark(1)->getStartTime();
		}
		for (int i = 0; i < block1->getNumMarks(); i++)
			block1->getMark(i)->setSelected(false);
		for (int i = 0; i < block2->getNumMarks(); i++)
			block2->getMark(i)->setSelected(false);
	}
	editor->paWindow->redraw();
}


void PATransitionEditor::createNewTransition(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string fromStateName = editor->stateList1->child(editor->stateList1->value())->label();
	std::string toStateName = editor->stateList2->child(editor->stateList2->value())->label();

	PATransitionData* transition = mcu.lookUpPATransition(fromStateName, toStateName);
	if (transition != NULL)
	{
		LOG("Transition %s to %s already exist.", fromStateName.c_str(), toStateName.c_str());
		return;
	}
	PAStateData* fromState = mcuCBHandle::singleton().lookUpPAState(fromStateName);
	PAStateData* toState = mcuCBHandle::singleton().lookUpPAState(toStateName);
	if (fromState != NULL && toState != NULL)
	{
		transition = new PATransitionData();
		transition->fromState = fromState;
		transition->toState = toState;
		mcu.addPATransition(transition);
		updateTransitionTimeMark(widget, data);
	}

	if (transition)
	{
		std::stringstream createTransitionCommand;
		createTransitionCommand << "panim transition fromstate " << transition->fromState->stateName << " tostate " << transition->toState->stateName << " ";
		for (int i = 0; i < editor->animForTransition1->size(); i++)
			if (editor->animForTransition1->selected(i))
				createTransitionCommand << editor->animForTransition1->goto_index(i)->label() << " ";
		createTransitionCommand << transition->getNumEaseOut() << " ";
		for (int i = 0; i < transition->getNumEaseOut(); i++)
			createTransitionCommand << transition->easeOutStart[i] << " " << transition->easeOutEnd[i] << " ";
		
		for (int i = 0; i < editor->animForTransition2->size(); i++)
			if (editor->animForTransition2->selected(i))
				createTransitionCommand << editor->animForTransition2->goto_index(i)->label() << " ";
		createTransitionCommand << transition->easeInStart << " " << transition->easeInEnd << " ";

		editor->paWindow->textDisplay->buffer()->append(createTransitionCommand.str().c_str());	
		editor->paWindow->textDisplay->buffer()->append("\n");
		editor->paWindow->textDisplay->relayout();
		editor->paWindow->textDisplay->redraw();
	}
}

void PATransitionEditor::changeTransitionList(fltk::Widget* widget, void* data)
{
	PATransitionEditor* editor = (PATransitionEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	editor->loadTransitions();

	nle::Block* block1 = editor->transitionEditorNleModel->getTrack(0)->getBlock(0);
	block1->removeAllMarks();
	nle::Block* block2 = editor->transitionEditorNleModel->getTrack(1)->getBlock(0);		
	block2->removeAllMarks();

	std::string fullName = editor->transitionList->child(editor->transitionList->value())->label();
	if (fullName == "---")
	{
		editor->paWindow->redraw();
		return;
	}

	size_t seperateMarkPos = fullName.find("|");
	std::string fromStateName = fullName.substr(0, seperateMarkPos - 1);
	std::string toStateName = fullName.substr(seperateMarkPos + 2, fullName.size() - 1);
	PATransitionData* transition = mcu.lookUpPATransition(fromStateName, toStateName);

	block1->removeAllMarks();
	block1->setName(transition->fromMotionName);
	std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(transition->fromMotionName);
	block1->setStartTime(0);
	block1->setEndTime(iter->second->duration());
	for (int i = 0; i < transition->getNumEaseOut(); i++)
	{
		editor->paWindow->addTimeMarkToBlock(block1, transition->easeOutStart[i]);
		editor->paWindow->addTimeMarkToBlock(block1, transition->easeOutEnd[i]);
	}
	block2->removeAllMarks();
	block2->setName(transition->toMotionName);
	iter = mcu.motion_map.find(transition->toMotionName);
	block2->setStartTime(0);
	block2->setEndTime(iter->second->duration());
	editor->paWindow->addTimeMarkToBlock(block2, transition->easeInStart);
	editor->paWindow->addTimeMarkToBlock(block2, transition->easeInEnd);

	editor->paWindow->redraw();
}
