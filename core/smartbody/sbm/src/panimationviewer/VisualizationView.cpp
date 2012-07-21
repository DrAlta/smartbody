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

#include "VisualizationView.h"
#include <sb/SBCharacter.h>
#include <sb/SBAnimationState.h>
#include <FL/gl.h>
#include <GL/glu.h>
#include <FL/fl_draw.H>
#include <sbm/mcontrol_util.h>
#include "ErrorVisualization.h"

VisualizationView::VisualizationView(int x, int y, int w, int h, PanimationWindow* window) : Fl_Group(x, y, w, h)
{
	paWindow = window;
	this->label("Visualization");
	this->begin();
	currentCycleState = new Fl_Output(2 * xDis + 100 + x, yDis + y, 200, 2 * yDis, "Current State");
	visShapeChoice = new Fl_Choice(2 * xDis + 100 + x, yDis*5 + y, 200, 2 * yDis, "Surface Type");
	visShapeChoice->add("curve");
	visShapeChoice->add("flat");
	visShapeChoice->value(1);

	buildVizButton = new Fl_Button(2 * xDis + 350 + x, yDis*10 + y , 200, 2 * yDis, "Build Viz");
	buildVizButton->callback(buildViz,this);

	currentViz = new Fl_Choice(2 * xDis + 100 + x, yDis*10 + y, 200, 2 * yDis, "Viz Type");
	currentViz->add("error");
	currentViz->add("smooth");
	currentViz->value(0);
	currentViz->callback(updateVizType,this);
	
	parameterGroup = new Fl_Group(2 * xDis + x , h / 10 + 9 * yDis + y, w - 2 * xDis, 9 * h / 10 - 10 * yDis);
	parameterGroup->box(FL_UP_BOX);
	this->end();
	this->resizable(parameterGroup);	

	errorViz = new ErrorVisualization(this->parameterGroup->x(), this->parameterGroup->y(), parameterGroup->w(), parameterGroup->h(), "");
	parameterGroup->add(errorViz);
	errorViz->show();
	errorViz->redraw();
}

VisualizationView::~VisualizationView()
{
}

void VisualizationView::update()
{
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	SbmCharacter* character = mcuCBHandle::singleton().getCharacter(charName);
	if (!character)
		return;

	if (!character->param_animation_ct)
		return;

	std::string currentState = "";
	if (character->param_animation_ct->getCurrentPABlendData())
	{
		currentState = character->param_animation_ct->getCurrentPABlendData()->state->stateName;

		PABlendData* curStateData = character->param_animation_ct->getCurrentPABlendData();		
		SmartBody::SBAnimationBlend* curBlend = dynamic_cast<SmartBody::SBAnimationBlend*>(curStateData->state);
		if (curBlend)
		{
			errorViz->setAnimationState(curBlend);
		}
	}	
	if (prevCycleState != currentState)
	{		
		prevCycleState = currentState;
		currentCycleState->value(currentState.c_str());
		paWindow->redraw();
	}	
	errorViz->redraw();
}

void VisualizationView::buildVisualization()
{
	SmartBody::SBCharacter* sbChar = paWindow->getCurrentCharacter();
	if (!sbChar) return;
	if (!sbChar->param_animation_ct) return;
	PABlendData* blendData = sbChar->param_animation_ct->getCurrentPABlendData();
	if (!blendData) return;
	SmartBody::SBAnimationBlend* curBlend = dynamic_cast<SmartBody::SBAnimationBlend*>(blendData->state);
	if (!curBlend) return;
	std::string surfType = visShapeChoice->text(visShapeChoice->value());
	curBlend->buildVisSurfaces("error",surfType,4,50);
	curBlend->buildVisSurfaces("smooth",surfType,4,50);
	errorViz->setAnimationState(curBlend);
}

void VisualizationView::buildViz( Fl_Widget* widget, void* data )
{
	VisualizationView* vizView = (VisualizationView*)(data);
	vizView->buildVisualization();
}

void VisualizationView::updateVizType( Fl_Widget* widget, void* data )
{
	VisualizationView* vizView = (VisualizationView*)(data);
	std::string drawType = vizView->currentViz->text(vizView->currentViz->value());
	vizView->errorViz->setDrawType(drawType);
}