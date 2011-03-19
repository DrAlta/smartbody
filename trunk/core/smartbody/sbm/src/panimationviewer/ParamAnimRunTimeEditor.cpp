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
#include <sbm/mcontrol_util.h>

ParameterVisualization::ParameterVisualization(int x, int y, int w, int h, char* name, PAStateData* s, ParameterWindow* window) : fltk::Group(x, y, w, h, name), state(s), paramWindow(window)
{
	paramX = -9999;
	paramY = -9999;
	setup();
}

ParameterVisualization::~ParameterVisualization()
{
}

void ParameterVisualization::draw()
{
	fltk::Group::draw();

	// draw axis
	fltk::setcolor(fltk::BLACK);
	fltk::drawline(0, centerY, width, centerY);
	fltk::drawline(centerX, 0, centerX, height);
	int recX, recY, recW, recH;
	getBound(centerX, centerY, recX, recY, recW, recH);	
	fltk::fillrect(recX, recY, recW, recH);

	// draw grid
	fltk::setcolor(fltk::WHITE);
	int numLinesX = width / gridSizeX;
	for (int i = -numLinesX / 2; i <= numLinesX / 2; i++)
		fltk::drawline(centerX + i * gridSizeX, 0, centerX + i * gridSizeX, height);
	int numLinesY = height / gridSizeY;
	for (int i = -numLinesY / 2; i <=  numLinesY / 2; i++)
		fltk::drawline(0, centerY + i * gridSizeY, width, centerY + i * gridSizeY);

	// draw parameters
	fltk::setcolor(fltk::GREEN);
	for (int i = 0; i < state->paramManager->getNumParameters(); i++)
	{
		int recX, recY, recW, recH;
		SrVec vec = state->paramManager->getVec(i);
		int x = 0;
		if (fabs(scaleX) > 0.0001)
			x = int(vec.x / scaleX);
		int y = 0;
		if (fabs(scaleY) > 0.0001)
			y = int(vec.y / scaleY);
		getBound(centerX + x, centerY - y, recX, recY, recW, recH);
		fltk::fillrect(recX, recY, recW, recH);
	}

	// draw lines connecting parameters
	for (int i = 0; i < state->paramManager->getNumParameters(); i++)
	{
		SrVec vec = state->paramManager->getVec(i);
		int x = 0;
		if (fabs(scaleX) > 0.0001)
			x = int(vec.x / scaleX);
		int y = 0;
		if (fabs(scaleY) > 0.0001)
			y = int(vec.y / scaleY);
		for (int j = i; j < state->paramManager->getNumParameters(); j++)
		{
			SrVec vec1 = state->paramManager->getVec(j);
			int x1 = 0;
			if (fabs(scaleX) > 0.0001)
				x1 = int(vec1.x / scaleX);
			int y1 = 0;
			if (fabs(scaleY) > 0.0001)
				y1 = int(vec1.y / scaleY);
			fltk::drawline(centerX + x, centerY - y, centerX + x1, centerY - y1);
		}
	}

	// draw parameters info
	fltk::setcolor(fltk::BLACK);
	for (int i = 0; i < state->paramManager->getNumParameters(); i++)
	{
		SrVec vec = state->paramManager->getVec(i);
		int x = int(vec.x / scaleX);
		int y = int(vec.y / scaleY);
		char buff[200];
//		sprintf(buff, "%s(%d,%d)", state->paramManager->getMotionName(i).c_str(), x, y);
		sprintf(buff, "%s", state->paramManager->getMotionName(i).c_str());
		fltk::drawtext(buff, float(centerX + x), float(centerY - y));
	}

	// draw parameter
	fltk::setcolor(fltk::RED);
	if (paramX != -9999 && paramY != -9999)
	{
		int recX, recY, recW, recH;
		getBound(paramX, paramY, recX, recY, recW, recH);
		fltk::fillrect(recX, recY, recW, recH);		
	}
}

int ParameterVisualization::handle(int event)
{
	int mousex = fltk::event_x();
	int mousey = fltk::event_y();
	switch (event)
	{
		case fltk::MOVE:
		{
			bool altKeyPressed = (fltk::get_key_state(fltk::LeftAltKey) || fltk::get_key_state(fltk::RightAltKey));
			if (altKeyPressed)
			{
				paramX = mousex;
				paramY = mousey;
				setSlider(paramX, paramY);
				redraw();
				break;
			}
		}
	}
	return fltk::Group::handle(event);
}

void ParameterVisualization::setup()
{
	centerX = w() / 2;
	centerY = h() / 2;
	width = w();
	height = h();

	SrVec vec = state->paramManager->getVec(state->paramManager->getMaxVec(0));
	float maxX = vec.x;
	vec = state->paramManager->getVec(state->paramManager->getMinVec(0));
	float minX = vec.x;
	if (fabs(maxX) < fabs(minX)) maxX = minX;
	vec = state->paramManager->getVec(state->paramManager->getMaxVec(1));
	float maxY = vec.y;
	vec = state->paramManager->getVec(state->paramManager->getMinVec(1));
	float minY = vec.y;
	if (fabs(maxY) < fabs(minY)) maxY = minY;
	scaleX = fabs(maxX * 3 / (float)width);
	scaleY = fabs(maxY * 3 / (float)height);
}

void ParameterVisualization::resize(int x, int y, int w, int h)
{
	fltk::Group::resize(x, y, w, h);
	setup();
	redraw();
}

void ParameterVisualization::setParam(float x, float y)
{
	paramX = centerX + int(x / scaleX);
	paramY = centerY - int(y / scaleY);
	redraw();
}

void ParameterVisualization::getBound(int ptX, int ptY, int& x, int& y, int& w, int& h)
{
	x = ptX - pad;
	y = ptY - pad;
	w = 2 * pad;
	h = 2 * pad;
}

void ParameterVisualization::setSlider(int x, int y)
{
	state->paramManager->setPrevVec(SrVec((float)x, (float)y, 0));
	double valueX = (x - centerX) * scaleX;
	double valueY = (centerY - y) * scaleY;
	if (valueX <= paramWindow->xAxis->minimum()) valueX = paramWindow->xAxis->minimum();
	if (valueX >= paramWindow->xAxis->maximum()) valueX = paramWindow->xAxis->maximum();
	paramWindow->xAxis->value((float)valueX);
	if (paramWindow->yAxis)
	{
		if (valueY <= paramWindow->yAxis->minimum()) valueY = paramWindow->yAxis->minimum();
		if (valueY >= paramWindow->yAxis->maximum()) valueY = paramWindow->yAxis->maximum();
		state->paramManager->setWeight((float)valueX, (float)valueY);
		paramWindow->yAxis->value((float)valueY);
	}
	else
		state->paramManager->setWeight((float)valueX);
	paramWindow->updateWeight();
	paramX = x;
	paramY = y;
	redraw();
}

ParameterWindow::ParameterWindow(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex) : fltk::Window(x, y, w, h, name), state(s), paWindow(window), exec(ex)
{
	this->label(state->stateName.c_str());
	this->begin();
		paramVisualization = new ParameterVisualization(4 * xDis, yDis, w - 5 * xDis, h - 5 * yDis, "", s, this);
		this->add(paramVisualization);
		this->resizable(paramVisualization);
		int type = state->paramManager->getType();
		if (type == 0)
		{
			double min = state->paramManager->getVec(state->paramManager->getMinVec(0)).x;
			double max = state->paramManager->getVec(state->paramManager->getMaxVec(0)).x;
			xAxis = new fltk::ValueSlider(4 * xDis, h - 4 * yDis, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(min);
			xAxis->maximum(max);
			xAxis->value(min);
			xAxis->callback(updateXAxisValue, this);
			double actualValue = state->paramManager->getPrevVec().x;
			if (actualValue <= max && actualValue >= min)
				paramVisualization->setSlider(int(actualValue), 0);
			yAxis = NULL;
		}
		if (type == 1)
		{
			double minX = state->paramManager->getVec(state->paramManager->getMinVec(0)).x;
			double maxX = state->paramManager->getVec(state->paramManager->getMaxVec(0)).x;
			double minY = state->paramManager->getVec(state->paramManager->getMinVec(1)).y;
			double maxY = state->paramManager->getVec(state->paramManager->getMaxVec(1)).y;
			xAxis = new fltk::ValueSlider(4 * xDis, h - 4 * yDis, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(minX);
			xAxis->maximum(maxX);
			xAxis->callback(updateAxisValue, this);
			yAxis = new fltk::ValueSlider(xDis, yDis, 2 * xDis, h - 5 * yDis, "Y");
			yAxis->minimum(minY);
			yAxis->maximum(maxY);
			yAxis->callback(updateAxisValue, this);
			yAxis->set_vertical();
			double actualValueX = state->paramManager->getPrevVec().x;
			double actualValueY = state->paramManager->getPrevVec().y;
			if (actualValueX <= maxX && actualValueX >= minX
				&& actualValueY <= maxY && actualValueY >= minY)
				paramVisualization->setSlider((int)actualValueX, (int)actualValueY);
		}
	this->end();
	
}

ParameterWindow::~ParameterWindow()
{
}

void ParameterWindow::updateXAxisValue(fltk::Widget* widget, void* data)
{
	ParameterWindow* window = (ParameterWindow*) data;
	double w = window->xAxis->value();
	window->state->paramManager->setWeight(w);
	if (window->exec)
		window->updateWeight();
	window->paramVisualization->setParam((float)w, 0);
}

void ParameterWindow::updateAxisValue(fltk::Widget* widget, void* data)
{
	ParameterWindow* window = (ParameterWindow*) data;
	double x = window->xAxis->value();
	double y = window->yAxis->value();
	window->state->paramManager->setWeight(x, y);
	if (window->exec)
		window->updateWeight();
	window->paramVisualization->setParam((float)x, (float)y);
}

void ParameterWindow::updateWeight()
{
	std::string charName = paWindow->characterList->child(paWindow->characterList->value())->label();
	std::stringstream command;
	command << "panim update char " << charName;
	int wNumber = state->getNumMotions();
	for (int j = 0; j < wNumber; j++)
		command << " " << state->weights[j];
	paWindow->execCmd(paWindow, command.str());
}

PARunTimeEditor::PARunTimeEditor(int x, int y, int w, int h, PanimationWindow* window) : fltk::Group(x, y, w, h), paWindow(window)
{
	this->label("Run Time Editor");
	this->begin();
		currentCycleState = new fltk::Output(2 * xDis + 100, yDis, 100, 2 * yDis, "Current State");
		nextCycleStates = new fltk::Browser(2 * xDis, 5 * yDis, w / 2 - 4 * xDis, h / 2, "Next State");
		nextCycleStates->callback(updateTransitionStates, this);
		availableTransitions = new fltk::Browser(w / 2 + 2 * xDis, 5 * yDis, w / 2 - 4 * xDis, h / 2, "Available Transitions");
		availableTransitions->callback(updateNonCycleState, this);
		availableTransitions->when(fltk::WHEN_ENTER_KEY_ALWAYS);
		runNextState = new fltk::Button(2 * xDis, h / 2 + 6 * yDis, 100, 2 * yDis, "Run");
		runNextState->callback(run, this);
	this->end();
	initializeRunTimeEditor();
	nonCycleParamWindow = NULL;
	cycleParamWindow = NULL;
}

PARunTimeEditor::~PARunTimeEditor()
{
}

void PARunTimeEditor::update()
{
	std::string charName = paWindow->characterList->child(paWindow->characterList->value())->label();
	SbmCharacter* character = mcuCBHandle::singleton().character_map.lookup(charName.c_str());
	if (!character)
		return;
	if (!character->param_animation_ct)
		return;
	std::string currentState = character->param_animation_ct->getCurrentStateName();
	if (currentState == "") currentState = "Idle";
	if (prevCycleState != currentState)
	{
		updateRunTimeStates(currentState);
		prevCycleState = currentState;
		currentCycleState->value(currentState.c_str());
	}

}

void PARunTimeEditor::updateRunTimeStates(std::string currentState)
{
	nextCycleStates->clear();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	PAStateData* stateData = mcu.lookUpPAState(currentState);
	if (stateData)
		if (!stateData->cycle)
			return;

	if (currentState == "Idle")
	{
		for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
		{
			if (mcu.param_anim_states[i]->cycle)
				addItem(nextCycleStates, mcu.param_anim_states[i]->stateName);
		}
	}
	else
	{
		for (size_t i = 0; i < stateData->toStates.size(); i++)
		{
			if (stateData->toStates[i]->toStates.size() == 0)
				addItem(nextCycleStates, "Idle");
			else
				for (size_t j = 0; j < stateData->toStates[i]->toStates.size(); j++)
					addItem(nextCycleStates, stateData->toStates[i]->toStates[j]->stateName.c_str());
		}
	}
	for (int i = 0; i < nextCycleStates->size(); i++)
		nextCycleStates->select(i, false);
	availableTransitions->clear();

}

void PARunTimeEditor::addItem(fltk::Browser* browser, std::string item)
{
	for (int i = 0; i < browser->size(); i++)
	{
		if (item == browser->goto_index(i)->label())
			return;
	}
	browser->add(item.c_str());
}

void PARunTimeEditor::initializeRunTimeEditor()
{
	std::string charName = paWindow->characterList->child(paWindow->characterList->value())->label();
	SbmCharacter* character = mcuCBHandle::singleton().character_map.lookup(charName.c_str());
	if (character)
	{
		if (character->param_animation_ct == NULL)
			return;
		if (character->param_animation_ct->getCurrentStateName() == "")
			currentCycleState->value("Idle");
		else
			currentCycleState->value(character->param_animation_ct->getCurrentStateName().c_str());

		nextCycleStates->clear();
		availableTransitions->clear();
		updateRunTimeStates(currentCycleState->value());
	}
	prevCycleState = "";
}

void PARunTimeEditor::updateNonCycleState(fltk::Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;
	if (editor->nonCycleParamWindow != NULL)
		delete editor->nonCycleParamWindow;

	std::string nonCycleState;
	for (int i = 0; i < editor->availableTransitions->size(); i++)
	{
		if (editor->availableTransitions->goto_index(i)->selected())
			nonCycleState = editor->availableTransitions->goto_index(i)->label();
	}
	if (mcuCBHandle::singleton().lookUpPAState(nonCycleState)->paramManager->getNumParameters() > 0)
	{
		editor->nonCycleParamWindow = new ParameterWindow(100, 100, 400, 300, "Parameter Window", mcuCBHandle::singleton().lookUpPAState(nonCycleState), editor->paWindow);
		editor->nonCycleParamWindow->show();
	}
}

void PARunTimeEditor::updateTransitionStates(fltk::Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	editor->availableTransitions->clear();
	std::string currentState = editor->currentCycleState->value();
	std::string nextState = "";
	for (int i = 0; i < editor->nextCycleStates->size(); i++)
	{
		if (editor->nextCycleStates->goto_index(i)->selected())
			nextState = editor->nextCycleStates->goto_index(i)->label();
	}
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
	{
		bool fromHit = false;
		bool toHit = false;
		if (currentState == "Idle")
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
		else if (nextState == "Idle")
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

void PARunTimeEditor::run(fltk::Widget* widget, void* data)
{
	PARunTimeEditor* editor = (PARunTimeEditor*) data;
	std::string charName = editor->paWindow->characterList->child(editor->paWindow->characterList->value())->label();
	std::string nextCycleState = "";
	for (int i = 0; i < editor->nextCycleStates->size(); i++)
		if (editor->nextCycleStates->selected(i))
		{	
			nextCycleState = editor->nextCycleStates->goto_index(i)->label();
			break;
		}
	std::string transitionState = "";
	for (int i = 0; i < editor->availableTransitions->size(); i++)
		if (editor->availableTransitions->selected(i))
		{	
			transitionState = editor->availableTransitions->goto_index(i)->label();
			break;
		}

	double timeoffset = 0.0;
	if (transitionState != "")
	{
		std::stringstream command1;
		command1 << "panim schedule char " << charName << " state " << transitionState << " loop false";
		editor->paWindow->execCmd(editor->paWindow, command1.str(), timeoffset);
		timeoffset += 0.1;
	}
	if (nextCycleState != "Idle" && nextCycleState != "")
	{
		std::stringstream command2;
		command2 << "panim schedule char " << charName << " state " << nextCycleState << " loop true";
		editor->paWindow->execCmd(editor->paWindow, command2.str(), timeoffset);
	}

	if (editor->cycleParamWindow)
		delete editor->cycleParamWindow;
	editor->cycleParamWindow = NULL;
	
	if (nextCycleState == "Idle" || nextCycleState == "") return;
	if (mcuCBHandle::singleton().lookUpPAState(nextCycleState)->paramManager->getNumParameters() > 0)
	{
		editor->cycleParamWindow = new ParameterWindow(100, 100, 400, 300, "Parameter Window", mcuCBHandle::singleton().lookUpPAState(nextCycleState), editor->paWindow, true);
		editor->cycleParamWindow->show();
	}
	if (editor->nonCycleParamWindow)
		delete editor->nonCycleParamWindow;
	editor->nonCycleParamWindow = NULL;
}