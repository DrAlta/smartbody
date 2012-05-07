#include "ParameterGroup.h"
#include <sbm/mcontrol_util.h>
#include "ParameterVisualization.h"
#include "Parameter3DVisualization.h"
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>

ParameterGroup::ParameterGroup(int x, int y, int w, int h, char* name, PAStateData* s, PanimationWindow* window, bool ex) : Fl_Group(x, y, w, h, name), stateData(s), paWindow(window), exec(ex)
{
	//printf("Create parameter group, x = %d, y = %d\n",x,y);
	this->label(stateData->state->stateName.c_str());
	this->begin();
		int type = stateData->state->getType();
		if (type == 0)
		{			
			int paraH =  h - 5 * yDis;
			paramVisualization = new ParameterVisualization(true, 4 * xDis + x, yDis + y, w - 5 * xDis, paraH, (char*)"", s, this);
			// since begin() is automatically called by the constructor for Fl_Group
			paramVisualization->end();

			this->resizable(paramVisualization);
			yAxis = NULL;
			zAxis = NULL;
			double min = stateData->state->getVec(stateData->state->getMinVecX()).x;
			double max = stateData->state->getVec(stateData->state->getMaxVecX()).x;
			xAxis = new Fl_Value_Slider(4 * xDis + x, h - 4 * yDis + y, w - 5 * xDis, 2 * yDis, "X");
			xAxis->minimum(min);
			xAxis->maximum(max);			
			xAxis->type(FL_HORIZONTAL);			
			xAxis->callback(updateXAxisValue, this);
			float actualValue;
			stateData->state->getParametersFromWeights(actualValue, stateData->weights);
			int actualX = 0;
			int actualY = 0;
			paramVisualization->getActualPixel(actualValue, 0.0f, actualX, actualY);
			param3DVisualization = NULL;
		}
		else if (type == 1)
		{
			int paraH =  h - 5 * yDis;
			paramVisualization = new ParameterVisualization(true, 4 * xDis + x, yDis + y, w - 5 * xDis, h - 5 * yDis, (char*)"", s, this);
			paramVisualization->end();
			this->resizable(paramVisualization);
			double minX = stateData->state->getVec(stateData->state->getMinVecX()).x;
			double maxX = stateData->state->getVec(stateData->state->getMaxVecX()).x;
			double minY = stateData->state->getVec(stateData->state->getMinVecY()).y;
			double maxY = stateData->state->getVec(stateData->state->getMaxVecY()).y;
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
			stateData->state->getParametersFromWeights(actualValueX, actualValueY, stateData->weights);
			int actualX = 0;
			int actualY = 0;
			paramVisualization->getActualPixel(actualValueX, actualValueY, actualX, actualY);
			param3DVisualization = NULL;
		}
		else if (type == 2)
		{
			param3DVisualization = new Parameter3DVisualization(4 * xDis + x, 4 * yDis + y, w - 5 * xDis, h - 8 * yDis, (char*)"", s, this);
			param3DVisualization->end();
			this->resizable(param3DVisualization);	
			paramVisualization = NULL;
			double minX = stateData->state->getVec(stateData->state->getMinVecX()).x;
			double maxX = stateData->state->getVec(stateData->state->getMaxVecX()).x;
			double minY = stateData->state->getVec(stateData->state->getMinVecY()).y;
			double maxY = stateData->state->getVec(stateData->state->getMaxVecY()).y;
			double minZ = stateData->state->getVec(stateData->state->getMinVecZ()).z;
			double maxZ = stateData->state->getVec(stateData->state->getMaxVecZ()).z;
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
			zAxis->minimum(minZ);
			zAxis->maximum(maxZ);
			zAxis->type(FL_HORIZONTAL);
			zAxis->callback(updateXYZAxisValue, this);
		}
		else
		{
			param3DVisualization = NULL;
			paramVisualization = NULL;
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
	PAStateData* stateData = group->getCurrentPAStateData();
	double w = group->xAxis->value();
	bool success = false;
	success = stateData->state->getWeightsFromParameters(w, stateData->weights);
	if (success)
		group->getCurrentCharacter()->param_animation_ct->updateWeights(stateData->weights);
	group->redraw();
}

void ParameterGroup::updateXYAxisValue(Fl_Widget* widget, void* data)
{
	ParameterGroup* group = (ParameterGroup*) data;
	PAStateData* stateData = group->getCurrentPAStateData();
	double x = group->xAxis->value();
	double y = group->yAxis->value();
	bool success = false;
	std::vector<double> weights;
	weights.resize(stateData->state->getNumMotions());
	success = stateData->state->getWeightsFromParameters(x, y, weights);
	if (success)
		group->getCurrentCharacter()->param_animation_ct->updateWeights(weights);
	group->redraw();
}

void ParameterGroup::updateXYZAxisValue(Fl_Widget* widget, void* data)
{		
	ParameterGroup* group = (ParameterGroup*) data;
	PAStateData* stateData = group->getCurrentPAStateData();
	double x = group->xAxis->value();
	double y = group->yAxis->value();
	double z = group->zAxis->value();
	bool success = false;
	std::vector<double> weights;
	weights.resize(stateData->state->getNumMotions());
	success = stateData->state->getWeightsFromParameters(x, y, z, weights);
	if (success)
		group->getCurrentCharacter()->param_animation_ct->updateWeights(weights);
	group->redraw();	
}

void ParameterGroup::updateWeight()
{
//	if (!state->cycle)
//		return;
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	std::stringstream command;
	command << "panim update char " << charName;
	int wNumber = stateData->state->getNumMotions();
	for (int j = 0; j < wNumber; j++)
		command << " " << stateData->weights[j];
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

SmartBody::SBCharacter* ParameterGroup::getCurrentCharacter()
{
	std::string charName = paWindow->characterList->menu()[paWindow->characterList->value()].label();
	return SmartBody::SBScene::getScene()->getCharacter(charName);
}
