/*
 *  ParamAnimRunTimeEditor.cpp - part of SmartBody-lib's Test Suite
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

#include "ParamAnimRunTimeEditor.h"
#include <FL/gl.h>
#include <GL/glu.h>
#include <FL/fl_draw.H>
#include <sbm/mcontrol_util.h>
#include <sbm/me_ct_param_animation_data.h>
#include "ParameterGroup.h"
#include "ParameterVisualization.h"
#include "Parameter3DVisualization.h"

PARunTimeEditor::PARunTimeEditor(int x, int y, int w, int h, PanimationWindow* window) : Fl_Group(x, y, w, h), paWindow(window)
{
	this->label("Run Time Editor");
	this->begin();
		currentCycleState = new Fl_Output(2 * xDis + 100 + x, yDis + y, 100, 2 * yDis, "Current State");
		nextCycleStates = new Fl_Hold_Browser(2 * xDis + x, 5 * yDis + y, w / 2 - 4 * xDis, h / 4, "Next State");
		nextCycleStates->callback(updateTransitionStates, this);
		
	
		availableTransitions = new Fl_Hold_Browser(w / 2 + 2 * xDis + x, 5 * yDis + y, w / 2 - 4 * xDis, h / 4, "Available Transitions");
		availableTransitions->callback(updateNonCycleState, this);
		availableTransitions->when(FL_WHEN_ENTER_KEY_ALWAYS);
		runNextState = new Fl_Button(2 * xDis + x, h / 4 + 6 * yDis + y, 100, 2 * yDis, "Run");
		runNextState->callback(run, this);
		parameterGroup = new Fl_Group(2 * xDis + x , h / 4 + 9 * yDis + y, w - 2 * xDis, 3 * h / 4 - 10 * yDis);
		parameterGroup->box(FL_UP_BOX);
	this->end();
	this->resizable(parameterGroup);
	paramGroup = NULL;
	initializeRunTimeEditor();
}

PARunTimeEditor::~PARunTimeEditor()
{
}

void PARunTimeEditor::update()
{
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	SbmCharacter* character = mcuCBHandle::singleton().getCharacter(charName);
	if (!character)
		return;
	if (!character->param_animation_ct)
		return;

	std::string currentState = "";
	if (character->param_animation_ct->getCurrentPAStateData())
	{
		currentState = character->param_animation_ct->getCurrentPAStateData()->state->stateName;
	}
	if (prevCycleState != currentState)
	{
		updateRunTimeStates(currentState);
		prevCycleState = currentState;
		currentCycleState->value(currentState.c_str());
		paWindow->redraw();
	}

	if (paramGroup)
	{
		PAStateData* curStateData = character->param_animation_ct->getCurrentPAStateData();
		if (!curStateData)
			return;
		if (curStateData)
		{
	//		if (curState->cycle)
			{
				if (paramGroup->paramVisualization)
				{
					float x = 0.0f, y = 0.0f;
					curStateData->state->getParametersFromWeights(x, y, curStateData->weights);
					int actualPixelX = 0;
					int actualPixelY = 0;
					paramGroup->paramVisualization->getActualPixel(x, y, actualPixelX, actualPixelY);
					paramGroup->paramVisualization->setPoint(actualPixelX, actualPixelY);	
					paramGroup->paramVisualization->redraw();
				}
				if (paramGroup->param3DVisualization)
				{
					float x = 0.0f, y = 0.0f, z = 0.0f;
					curStateData->state->getParametersFromWeights(x, y, z, curStateData->weights);
					paramGroup->xAxis->value(x);
					paramGroup->yAxis->value(y);
					paramGroup->zAxis->value(z);
					paramGroup->param3DVisualization->redraw();
				}
			}
		}
	}
}

void PARunTimeEditor::updateRunTimeStates(std::string currentState)
{
	nextCycleStates->clear();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	PAState* state = mcu.lookUpPAState(currentState);

//	if (stateData)
//		if (!stateData->cycle)
//			return;

	if (currentState == "")
		return;

	if (currentState == PseudoIdleState)
	{
		for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
		{
//			if (mcu.param_anim_states[i]->cycle)
				addItem(nextCycleStates, mcu.param_anim_states[i]->stateName);
		}
	}
	else
	{
//		if (stateData->toStates.size() == 0)
			addItem(nextCycleStates, PseudoIdleState);
		for (size_t i = 0; i < state->toStates.size(); i++)
			for (size_t j = 0; j < state->toStates[i]->toStates.size(); j++)
				addItem(nextCycleStates, state->toStates[i]->toStates[j]->stateName.c_str());
	}
	for (int i = 0; i < nextCycleStates->size(); i++)
		nextCycleStates->select(i+1, false);
	availableTransitions->clear();

	if (paramGroup)
	{
		parameterGroup->remove(paramGroup);
		delete paramGroup;
		paramGroup = NULL;
	}
	if (state)
	{
		std::vector<double> weights;
		PAStateData* stateData = new PAStateData(state, weights);
		// memory leak!
		paramGroup = new ParameterGroup(this->parameterGroup->x(), this->parameterGroup->y(), parameterGroup->w(), parameterGroup->h(), (char*)"", stateData, paWindow);
		parameterGroup->add(paramGroup);
		paramGroup->show();
		paramGroup->redraw();
		if (paramGroup->param3DVisualization)
			paramGroup->param3DVisualization->show();
	}
}

void PARunTimeEditor::addItem(Fl_Browser* browser, std::string item)
{
	for (int i = 0; i < browser->size(); i++)
	{
		const char* text = browser->text(i+1);		
		if (item == text)
			return;
	}
	browser->add(item.c_str());
	const char* newText = browser->text(1);
}

void PARunTimeEditor::initializeRunTimeEditor()
{
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	SbmCharacter* character = mcuCBHandle::singleton().getCharacter(charName);
	if (character)
	{
		if (character->param_animation_ct == NULL)
			return;
		currentCycleState->value(character->param_animation_ct->getCurrentPAStateData()->state->stateName.c_str());

		nextCycleStates->clear();
		availableTransitions->clear();
		updateRunTimeStates(currentCycleState->value());
	}
	prevCycleState = "";
}

void PARunTimeEditor::updateNonCycleState(Fl_Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;

	std::string nonCycleState;
	for (int i = 0; i < editor->availableTransitions->size(); i++)
	{
		if (editor->availableTransitions->selected(i+1))
			nonCycleState = editor->availableTransitions->text(i+1);
	}
	PAState* state = mcuCBHandle::singleton().lookUpPAState(nonCycleState);
	if (state && state->getNumParameters() > 0)
	{
		if (editor->paramGroup)
		{
			editor->parameterGroup->remove(editor->paramGroup);
			delete editor->paramGroup;
			editor->paramGroup = NULL;
		}
		
		std::vector<double> weights;
		PAStateData* stateData = new PAStateData(state, weights);
		// memory leak here!
		editor->paramGroup = new ParameterGroup(editor->parameterGroup->x(), editor->parameterGroup->y(), editor->parameterGroup->w(), editor->parameterGroup->h(), (char*)"", stateData, editor->paWindow);
		editor->parameterGroup->add(editor->paramGroup);
		editor->paramGroup->show();
		editor->paramGroup->redraw();		
	}
}

void PARunTimeEditor::updateTransitionStates(Fl_Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	editor->availableTransitions->clear();
	std::string currentState = editor->currentCycleState->value();
	std::string nextState = "";
	for (int i = 0; i < editor->nextCycleStates->size(); i++)
	{
		if (editor->nextCycleStates->selected(i+1))
			nextState = editor->nextCycleStates->text(i+1);
	}
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
	{
		bool fromHit = false;
		bool toHit = false;
		if (currentState == PseudoIdleState)
		{
			if (mcu.param_anim_states[i]->fromStates.size() == 0)
			{
				for (size_t j = 0; j < mcu.param_anim_states[i]->toStates.size(); j++)
				{
					if (mcu.param_anim_states[i]->toStates[j]->stateName == nextState)
					{
						editor->availableTransitions->add(mcu.param_anim_states[i]->stateName.c_str());	
						break;
					}
				}
			}
		}
		else if (nextState == PseudoIdleState)
		{
			if (mcu.param_anim_states[i]->toStates.size() == 0)
			{
				for (size_t j = 0; j < mcu.param_anim_states[i]->fromStates.size(); j++)
				{
					if (mcu.param_anim_states[i]->fromStates[j]->stateName == currentState)
					{
						editor->availableTransitions->add(mcu.param_anim_states[i]->stateName.c_str());	
						break;
					}
				}
			}
		}
		else
		{
			for (size_t j = 0; j < mcu.param_anim_states[i]->fromStates.size(); j++)
				if (mcu.param_anim_states[i]->fromStates[j]->stateName == currentState)
				{
					fromHit = true;
					break;
				}
			for (size_t j = 0; j < mcu.param_anim_states[i]->toStates.size(); j++)
				if (mcu.param_anim_states[i]->toStates[j]->stateName == nextState)
				{
					toHit = true;
					break;
				}

			if (fromHit && toHit)
				editor->availableTransitions->add(mcu.param_anim_states[i]->stateName.c_str());	
		}
	}
}

void PARunTimeEditor::run(Fl_Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;
	std::string charName = editor->paWindow->characterList->menu()[editor->paWindow->characterList->value()].label();
	std::string nextCycleState = "";
	for (int i = 0; i < editor->nextCycleStates->size(); i++)
		if (editor->nextCycleStates->selected(i+1))
		{	
			nextCycleState = editor->nextCycleStates->text(i+1);
			break;
		}
	std::string transitionState = "";
	for (int i = 0; i < editor->availableTransitions->size(); i++)
		if (editor->availableTransitions->selected(i+1))
		{	
			transitionState = editor->availableTransitions->text(i+1);
			break;
		}

	double timeoffset = 0.0;
	if (transitionState != "")
	{
		std::stringstream command1;
		command1 << "panim schedule char " << charName << " state " << transitionState << " loop false playnow false additive false joint null";
		editor->paWindow->execCmd(editor->paWindow, command1.str(), timeoffset);
		timeoffset += 0.1;
	}
	if (nextCycleState != PseudoIdleState && nextCycleState != "")
	{
		std::stringstream command2;
		command2 << "panim schedule char " << charName << " state " << nextCycleState << " loop true playnow false additive false joint null";
		editor->paWindow->execCmd(editor->paWindow, command2.str(), timeoffset);
	}
	
	if (nextCycleState == "") return;
	if (nextCycleState == PseudoIdleState)
	{
		std::stringstream command3;
		command3 << "panim schedule char " << charName << " state " << "PseudoIdle" << " loop true playnow true additive false joint null";
		editor->paWindow->execCmd(editor->paWindow, command3.str(), timeoffset);
	}
}
