/*
 *  ParamAnimScriptEditor.cpp - part of SmartBody-lib's Test Suite
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

#include "ParamAnimScriptEditor.h"
#include <sbm/mcontrol_util.h>

#define transitionTrace 0

PAScriptEditor::PAScriptEditor(int x, int y, int w, int h, PanimationWindow* window) : fltk::Group(x, y, w, h), paWindow(window)
{
	this->label("Script Editor");
	this->begin();
		availableStateList = new Browser(2 * xDis, 3 * yDis, w / 2 - 6 * xDis, 2 * h / 3 - 5 * yDis, "Available States"); 
		currentStateList = new Browser(w / 2 + 4 * xDis, 3 * yDis, w / 2 - 6 * xDis, 2 * h / 3 - 5 * yDis, "Scheduled States");
		currentStateList->when(fltk::WHEN_ENTER_KEY_ALWAYS);
		currentStateList->callback(updateStateInfo, this);
		addStateButton = new Button(w / 2 - 3 * xDis, h / 2, 6 * xDis, 2 * yDis, ">>>");
		addStateButton->callback(addState, this);
		removeStateButton = new Button(w / 2 - 3 * xDis, h / 2 + 3 * yDis, 6 * xDis, 2 * yDis, "<<<");
		removeStateButton->callback(removeState, this);
		runStateList = new Button(2 * xDis, 2 * h / 3, 100, 20, "Run");
		runStateList->callback(run, this);
		currentStatePanel = new Output(10 * xDis, h - 6 * yDis, 200, 2 * yDis, "Current State:");
		currentStateWeight = new ValueSlider(11 * xDis + 200, h - 6 * yDis, 200, 2 * yDis, "Weight");
		currentStateWeight->callback(changeCurrentStateWeight, this);
		currentStateWeight->minimum(0);
		currentStateWeight->maximum(1);
		currentStateWeight->deactivate();
		nextStatePanel = new Output(10 * xDis, h - 3 * yDis, 200, 2 * yDis, "Next State:");
	this->end();
	initialAvailableStates();
}

PAScriptEditor::~PAScriptEditor()
{
}

void PAScriptEditor::addState(fltk::Widget* widget, void* data)
{
	PAScriptEditor* editor = (PAScriptEditor*) data;
	for (int i = 0; i < editor->availableStateList->size(); i++)
	{
		if (editor->availableStateList->goto_index(i)->selected())
		{
			std::string selectedState = editor->availableStateList->goto_index(i)->label();
			editor->currentStateList->add(selectedState.c_str());
#if transitionTrace
			editor->updateAvailableStates(selectedState);
#endif
			std::map<std::string, double>::iterator iter = editor->stateTimeOffset.find(selectedState);
			if (iter != editor->stateTimeOffset.end())
				editor->stateTimeOffset.erase(iter);
			editor->stateTimeOffset.insert(std::make_pair(selectedState, 0.1));
		
			int loopValue = fltk::ask("Is this loop mode");
			bool loop;
			if (loopValue == 1)	loop = true;
			else				loop = false;
			std::map<std::string, bool>::iterator iter1 = editor->stateLoopMode.find(selectedState);
			if (iter1 != editor->stateLoopMode.end())
				editor->stateLoopMode.erase(iter1);
			editor->stateLoopMode.insert(std::make_pair(selectedState, loop));
		}
	}
}

void PAScriptEditor::removeState(fltk::Widget* widget, void* data)
{
	PAScriptEditor* editor = (PAScriptEditor*) data;
	int size = editor->currentStateList->size();
	if (size > 0)
	{
		std::string selectedState = editor->currentStateList->goto_index(size - 1)->label();
		editor->currentStateList->remove(size - 1);

		bool deleteMap = true;
		for (int i = 0; i < editor->currentStateList->size(); i++)
		{
			if (selectedState == editor->currentStateList->goto_index(i)->label())
				deleteMap = false;
		}
		if (deleteMap)
		{
			std::map<std::string, double>::iterator iter = editor->stateTimeOffset.find(selectedState);
			editor->stateTimeOffset.erase(iter);

			std::map<std::string, bool>::iterator iter1 = editor->stateLoopMode.find(selectedState);
			editor->stateLoopMode.erase(iter1);
		}
	}

	if (editor->currentStateList->size() == 0)
		editor->initialAvailableStates();
#if transitionTrace
	else
	{
		editor->currentStateList->goto_index(editor->currentStateList->size() - 1)->set_selected();
		editor->updateAvailableStates(editor->currentStateList->goto_index(editor->currentStateList->size() - 1)->label());
	}
	for (int i = 0; i < editor->currentStateList->size(); i++)
	{
		if (editor->currentStateList->goto_index(i)->selected())
			editor->updateAvailableStates(editor->currentStateList->goto_index(i)->label());
	}
#endif
}

void PAScriptEditor::updateStateInfo(fltk::Widget* widget, void* data)
{
	PAScriptEditor* editor = (PAScriptEditor*) data;
	for (int i = 0; i < editor->currentStateList->size(); i++)
	{
		if (editor->currentStateList->goto_index(i)->selected())
		{
			std::string selectedState = editor->currentStateList->goto_index(i)->label();
			bool shouldAddTimeOffset;
			if (i == 0)	shouldAddTimeOffset = false;
			else
			{
				std::string previousState = editor->currentStateList->goto_index(i - 1)->label();
				std::map<std::string, bool>::iterator iter1 = editor->stateLoopMode.find(previousState);
				shouldAddTimeOffset = iter1->second;
				}
			if (shouldAddTimeOffset)
			{
				const char* offsetString = fltk::input("time offset from previous state", "0.0");
				if (offsetString != NULL)
				{
					double offset = atof(offsetString);
					std::map<std::string, double>::iterator iter = editor->stateTimeOffset.find(selectedState);
					iter->second = offset;
				}
			}
			PAStateData* state = mcuCBHandle::singleton().lookUpPAState(selectedState);
			if (state->getNumMotions() > 1)
			{
				const char* ws = fltk::input("weights (separate by white space)", "");
				if (ws == NULL)	return;
				std::string weights = ws;
				std::vector<std::string> weight = editor->paWindow->tokenize(weights, " ");
				if (state->getNumMotions() == weight.size())
					for (int i = 0; i < state->getNumMotions(); i++)
						state->weights[i] = (float)atof(weight[i].c_str());	
			}
		}
	}	
}

void PAScriptEditor::run(fltk::Widget* widget, void* data)
{
	PAScriptEditor* editor = (PAScriptEditor*) data;
	std::string charName = editor->paWindow->characterList->child(editor->paWindow->characterList->value())->label();
	double offset = 0.0;
	for (int i = 0; i < editor->currentStateList->size(); i++)
	{
		std::string stateName = editor->currentStateList->goto_index(i)->label();
		std::map<std::string, bool>::iterator iter = editor->stateLoopMode.find(stateName);
		bool loop = iter->second;
		std::string loopString;
		if (loop)	loopString = "true";
		else		loopString = "false";
		std::map<std::string, double>::iterator iter1 = editor->stateTimeOffset.find(stateName);
		if (i != 0)
			offset += iter1->second;
		std::stringstream command;
		command << "panim schedule char " << charName << " state " << stateName << " loop " << loopString;
		PAStateData* state = mcuCBHandle::singleton().lookUpPAState(stateName);
		int wNumber = state->getNumMotions();
		for (int j = 0; j < wNumber; j++)
			command << " " << state->weights[j];
		editor->paWindow->execCmd(editor->paWindow, command.str(), offset);
	}
}

void PAScriptEditor::changeCurrentStateWeight(fltk::Widget* widget, void* data)
{
	PAScriptEditor* editor = (PAScriptEditor*) data;
	double weight = editor->currentStateWeight->value();
	std::string charName = editor->paWindow->characterList->child(editor->paWindow->characterList->value())->label();
	std::string stateName = editor->currentStatePanel->value();
	std::stringstream command;
	command << "panim update char " << charName;
	PAStateData* state = mcuCBHandle::singleton().lookUpPAState(stateName);
	if (!state)
		return;
	int wNumber = state->getNumMotions();
	int id = int(weight);
	for (int j = 0; j < wNumber; j++)
	{
		if (j == id)
			command << " " << 1 - (weight - id);
		else if (j == (id + 1))
			command << " " << (weight - id);
		else
			command << " 0.0";
	}
	editor->paWindow->execCmd(editor->paWindow, command.str());
}

void PAScriptEditor::initialAvailableStates()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	availableStateList->clear();
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
		availableStateList->add(mcu.param_anim_states[i]->stateName.c_str());
}

void PAScriptEditor::updateAvailableStates(std::string currentState)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	availableStateList->clear();
	for (size_t i = 0; i < mcu.param_anim_transitions.size(); i++)
	{
		if (mcu.param_anim_transitions[i]->fromState->stateName == currentState)
			availableStateList->add(mcu.param_anim_transitions[i]->toState->stateName.c_str());
	}
}

void PAScriptEditor::refresh()
{
	initialAvailableStates();
	currentStateList->clear();
	currentStatePanel->value("");
	nextStatePanel->value("");
	currentStateWeight->deactivate();
	stateTimeOffset.clear();
	stateLoopMode.clear();
}

void PAScriptEditor::update()
{
	std::string charName = paWindow->characterList->child(paWindow->characterList->value())->label();
	SbmCharacter* character = mcuCBHandle::singleton().character_map.lookup(charName.c_str());
	if (!character)
		return;
	if (!character->param_animation_ct)
		return;
	std::string curStateName = character->param_animation_ct->getCurrentStateName();
	std::string nextStateName = character->param_animation_ct->getNextStateName();
	currentStatePanel->value(curStateName.c_str());
	nextStatePanel->value(nextStateName.c_str());

	if (curStateName != "" && nextStateName != "")
	{
		currentStateWeight->deactivate();
	}
	else
	{
		int numWeights = character->param_animation_ct->getNumWeights();
		if (numWeights > 1)
		{
			currentStateWeight->activate();
			currentStateWeight->maximum(numWeights - 1);
		}
	}	
}
