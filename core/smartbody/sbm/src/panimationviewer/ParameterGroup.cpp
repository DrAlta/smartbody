#include "ParameterGroup.h"
#include <sbm/mcontrol_util.h>
#include "ParameterVisualization.h"
#include "Parameter3DVisualization.h"


ParameterGroup::ParameterGroup(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex) : Fl_Group(x, y, w, h, name), state(s), paWindow(window), exec(ex)
{
	//printf("Create parameter group, x = %d, y = %d\n",x,y);
	this->label(s->stateName.c_str());
	this->begin();
		int type = state->paramManager->getType();
		if (type == 0)
		{			
			int paraH =  h - 5 * yDis;
			paramVisualization = new ParameterVisualization(4 * xDis + x, yDis + y, w - 5 * xDis, paraH, (char*)"", s, this);
			// since begin() is automatically called by the constructor for Fl_Group
			paramVisualization->end();

			this->resizable(paramVisualization);
			yAxis = NULL;
			zAxis = NULL;
			double min = state->paramManager->getVec(state->paramManager->getMinVecX()).x;
			double max = state->paramManager->getVec(state->paramManager->getMaxVecX()).x;
			xAxis = new Fl_Value_Slider(4 * xDis + x, h - 4 * yDis + y, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(min);
			xAxis->maximum(max);			
			xAxis->type(FL_HORIZONTAL);			
			xAxis->callback(updateXAxisValue, this);
			float actualValue;
			s->paramManager->getParameter(actualValue);
			int actualX = 0;
			int actualY = 0;
			paramVisualization->getActualPixel(actualValue, 0.0f, actualX, actualY);
			paramVisualization->setSlider(actualX, actualY);
			param3DVisualization = NULL;
		}
		if (type == 1)
		{
			int paraH =  h - 5 * yDis;
			paramVisualization = new ParameterVisualization(4 * xDis + x, yDis + y, w - 5 * xDis, h - 5 * yDis, (char*)"", s, this);
			paramVisualization->end();
			this->resizable(paramVisualization);
			double minX = state->paramManager->getVec(state->paramManager->getMinVecX()).x;
			double maxX = state->paramManager->getVec(state->paramManager->getMaxVecX()).x;
			double minY = state->paramManager->getVec(state->paramManager->getMinVecY()).y;
			double maxY = state->paramManager->getVec(state->paramManager->getMaxVecY()).y;
			xAxis = new Fl_Value_Slider(4 * xDis + x, h - 4 * yDis + y, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(minX);
			xAxis->maximum(maxX);
			xAxis->type(FL_HORIZONTAL);
			xAxis->callback(updateXYAxisValue, this);
			yAxis = new Fl_Value_Slider(xDis + x, yDis + y, 3 * xDis, h - 5 * yDis, "Y");
			yAxis->minimum(minY);
			yAxis->maximum(maxY);
			yAxis->callback(updateXYAxisValue, this);
			yAxis->type(FL_VERTICAL);
			float actualValueX, actualValueY;
			s->paramManager->getParameter(actualValueX, actualValueY);
			int actualX = 0;
			int actualY = 0;
			paramVisualization->getActualPixel(actualValueX, actualValueY, actualX, actualY);
			paramVisualization->setSlider(actualX, actualY);
			param3DVisualization = NULL;
		}
		if (type == 2)
		{
			param3DVisualization = new Parameter3DVisualization(4 * xDis + x, 4 * yDis + y, w - 5 * xDis, h - 8 * yDis, (char*)"", s, this);
			param3DVisualization->end();
			this->resizable(param3DVisualization);	
			paramVisualization = NULL;
			double minX = state->paramManager->getVec(state->paramManager->getMinVecX()).x;
			double maxX = state->paramManager->getVec(state->paramManager->getMaxVecX()).x;
			double minY = state->paramManager->getVec(state->paramManager->getMinVecY()).y;
			double maxY = state->paramManager->getVec(state->paramManager->getMaxVecY()).y;
			xAxis = new Fl_Value_Slider(4 * xDis + x, h - 4 * yDis + y, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(minX);
			xAxis->maximum(maxX);
			xAxis->type(FL_HORIZONTAL);
			xAxis->callback(updateXYZAxisValue, this);
			yAxis = new Fl_Value_Slider(xDis + x, yDis + y, 3 * xDis, h - 5 * yDis, "Y");
			yAxis->minimum(minY);
			yAxis->maximum(maxY);
			yAxis->callback(updateXYZAxisValue, this);
			yAxis->type(FL_VERTICAL);
			zAxis = new Fl_Value_Slider(4 * xDis + x, yDis + y, w - 5 * xDis, 2 * yDis, "Z");
			zAxis->minimum(-90);	// TODO: remove this hard code part
			zAxis->maximum(90);
			zAxis->type(FL_HORIZONTAL);
			zAxis->callback(updateXYZAxisValue, this);
		}
	this->end();	
	this->redraw();
	paWindow->redraw();
}

void ParameterGroup::resize(int x, int y, int w, int h)
{
	Fl_Group::resize(x, y, w, h);
}


ParameterGroup::~ParameterGroup()
{
}

void ParameterGroup::updateXAxisValue(Fl_Widget* widget, void* data)
{
	ParameterGroup* group = (ParameterGroup*) data;
	PAStateData* state = group->getCurrentPAStateData();
	double w = group->xAxis->value();
	bool success = false;
	success = state->paramManager->setWeight(w);
	if (success)
		group->getCurrentCharacter()->param_animation_ct->updateWeights();
	group->redraw();
}

void ParameterGroup::updateXYAxisValue(Fl_Widget* widget, void* data)
{
	ParameterGroup* group = (ParameterGroup*) data;
	PAStateData* state = group->getCurrentPAStateData();
	double x = group->xAxis->value();
	double y = group->yAxis->value();
	bool success = false;
	success = state->paramManager->setWeight(x, y);
	if (success)
		group->getCurrentCharacter()->param_animation_ct->updateWeights();
	group->redraw();
}

void ParameterGroup::updateXYZAxisValue(Fl_Widget* widget, void* data)
{		
	ParameterGroup* group = (ParameterGroup*) data;
	PAStateData* state = group->getCurrentPAStateData();
	double x = group->xAxis->value();
	double y = group->yAxis->value();
	double z = group->zAxis->value();
	bool success = false;
	success = state->paramManager->setWeight(x, y, z);
	if (success)
		group->getCurrentCharacter()->param_animation_ct->updateWeights();
	group->redraw();	
}

void ParameterGroup::updateWeight()
{
//	if (!state->cycle)
//		return;
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	std::stringstream command;
	command << "panim update char " << charName;
	int wNumber = state->getNumMotions();
	if (wNumber == 1)
		state->weights[0] = 1.0;
	for (int j = 0; j < wNumber; j++)
		command << " " << state->weights[j];
	paWindow->execCmd(paWindow, command.str());
}

PAStateData* ParameterGroup::getCurrentPAStateData()
{
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	SbmCharacter* character = mcuCBHandle::singleton().getCharacter(charName);
	if (!character)
		return NULL;
	if (!character->param_animation_ct)
		return NULL;
	return character->param_animation_ct->getCurrentPAStateData();
}

SbmCharacter* ParameterGroup::getCurrentCharacter()
{
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	return mcuCBHandle::singleton().getCharacter(charName);	
}
