#include "ParamAnimAutoMarkingEditor.h"
#include "ParamAnimEditorWidget.h"
#include "ParamAnimStateEditor.h"
#include <sb/SBCharacter.h>
#include <sb/SBJoint.h>
#include <sb/SBScene.h>
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>

PAAutoFootStepsEditor::PAAutoFootStepsEditor(PAStateEditor* editor, int x, int y, int w, int h) : Fl_Window(x, y, w, h)
{
	set_modal();
	stateEditor = editor;
	xDis = 10;
	yDis = 10;
	int csx = xDis;
	int csy = yDis;	

	this->label("Auto Footsteps Editor");
	this->begin();
	inputFloorHeight = new Fl_Input(xDis + csx + 100, yDis, 10 * xDis, 2 * yDis, "FloorHeight");
	inputFloorHeight->value("0");
	inputHeightThreshold = new Fl_Input(xDis + csx + 100, 4 * yDis, 10 * xDis, 2 * yDis, "HeightThreshold");
	inputHeightThreshold->value("15");
	inputSpeedThreshold = new Fl_Input(xDis + csx + 100, 7 * yDis, 10 * xDis, 2 * yDis, "SpeedThreshold");
	inputSpeedThreshold->value("10");
	inputSpeedDetectWindow = new Fl_Input(xDis + csx + 100, 10 * yDis, 10 * xDis, 2 * yDis, "SpeedDetectWindow");
	inputSpeedDetectWindow->value("3");
	inputSpeedDetectWindow->deactivate();
	
	browserJoint = new Fl_Multi_Browser(xDis + csx + 100, 13 * yDis, 20 * xDis, 20 * yDis, "Joint");
	SBCharacter* character = stateEditor->paWindow->getCurrentCharacter();
	if (character)
	{
		std::vector<std::string>& charJNames = character->getSkeleton()->getJointNames();
		for (size_t i = 0; i < charJNames.size(); i++)
			browserJoint->add(charJNames[i].c_str());
	}

	inputMaxSteps = new Fl_Input(xDis + csx + 100, 36 * yDis, 10 * xDis, 2 * yDis, "MaxSteps");
	PAState* curState = stateEditor->getCurrentState();
	int maxSteps = 4;
	if (curState)
		maxSteps = curState->getNumKeys();
	std::stringstream ss;
	ss << maxSteps;
	inputMaxSteps->value(ss.str().c_str());

	buttonConfirm = new Fl_Button(xDis + csx, 40 * yDis, 10 * xDis, 2 * yDis, "Apply");
	buttonConfirm->callback(confirmEditting, this);
	buttonCancel = new Fl_Button(xDis + csx + 120, 40 * yDis, 10 * xDis, 2 * yDis, "Leave");
	buttonCancel->callback(cancelEditting, this);
	this->end();
}


PAAutoFootStepsEditor::~PAAutoFootStepsEditor()
{
}

void PAAutoFootStepsEditor::confirmEditting(Fl_Widget* widget, void* data)
{
	PAAutoFootStepsEditor* footStepEditor = (PAAutoFootStepsEditor*) data;
	
	PAState* currentState = footStepEditor->stateEditor->getCurrentState();
	if (!currentState)
	{	
		LOG("PAAutoFootStepsEditor::confirmEditting WARNING: please select a state!");
		return;
	}


	// take down previous correspondence points first
	footStepEditor->stateEditor->previousKeys.clear();
	footStepEditor->stateEditor->previousKeys.resize(currentState->getNumMotions());
	for (int i = 0; i < currentState->getNumMotions(); i++)
	{
		footStepEditor->stateEditor->previousKeys[i].resize(currentState->getNumKeys());
		footStepEditor->stateEditor->previousKeys[i] = currentState->keys[i];
	}


	// auto foot steps algorithm
	float floorHeight = (float)atof(footStepEditor->inputFloorHeight->value());
	float heightThresh = (float)atof(footStepEditor->inputHeightThreshold->value());
	float speedThresh = (float)atof(footStepEditor->inputSpeedThreshold->value());
	int speedWindow = atoi(footStepEditor->inputSpeedDetectWindow->value());
	int maxNumSteps = atoi(footStepEditor->inputMaxSteps->value());
	if (maxNumSteps < 2)
		maxNumSteps = 2;

	SBCharacter* curCharacter = footStepEditor->stateEditor->paWindow->getCurrentCharacter();
	std::vector<std::string> selectedJoints;
	for (int i = 0; i < footStepEditor->browserJoint->size(); i++)
	{
		if (footStepEditor->browserJoint->selected(i+1))
		{
			selectedJoints.push_back(footStepEditor->browserJoint->text(i + 1));
		}
	}


	for (int m = 0; m < currentState->getNumMotions(); m++)
	{
		std::vector<double> possibleTiming;
		SBMotion* motion = dynamic_cast<SBMotion*>(currentState->motions[m]);
		if (!motion)
			continue;
		motion->connect(curCharacter->getSkeleton());

		for(int f = 0; f < motion->getNumFrames(); f++)
		{
			motion->apply_frame(f);
			motion->connected_skeleton()->update_global_matrices();

			for (size_t jointId = 0; jointId < selectedJoints.size(); jointId ++)
			{
				std::string jointName = selectedJoints[jointId];
				SBJoint* joint = curCharacter->getSkeleton()->getJointByName(jointName);
				if (!joint)
					continue;

				const SrMat& gMat = joint->gmat();
				SrVec gPos = SrVec(gMat.get(12), gMat.get(13), gMat.get(14));

				// filter for height
				if (gPos.y < floorHeight || gPos.y > (floorHeight + heightThresh))
					continue;

				// filter for speed
				int startFrame = f - speedWindow / 2;
				int endFrame = f + speedWindow / 2;
				float startTime = startFrame * (float)motion->getFrameRate();
				float endTime = endFrame * (float)motion->getFrameRate();
				float speed = motion->getJointSpeed(joint, startTime, endTime);
				if (speed <= speedThresh)
				{
					//LOG("Speed for motion %s at time %f is %f", motion->getName().c_str(), f * motion->getFrameRate(), speed);
					possibleTiming.push_back(f * (float)motion->getFrameRate());
				}
			}
		}

		// K means algorithm according to desired number of steps
		std::vector<double> outMeans;	
		bool isConvergent = footStepEditor->kMeansClustering1D(maxNumSteps, possibleTiming, outMeans);

		// apply it to corresponding points
		if (isConvergent)
		{
			std::stringstream ss;
			ss << "State " << currentState->stateName << " motion " << motion->getName() << " auto foot steps detected: ";
			for (size_t i = 0; i < outMeans.size(); i++)
				ss << outMeans[i] << " ";
			LOG("%s", ss.str().c_str());
			currentState->keys[m].clear();
			currentState->keys[m].resize(maxNumSteps);
			currentState->keys[m] = outMeans;
		}
		else
		{
			std::stringstream ss;
			ss << "State " << currentState->stateName << " motion " << motion->getName() << " auto foot steps not detected(evenly distributed): ";
			currentState->keys[m].clear();
			currentState->keys[m].push_back(0);
			ss << 0 << " "; 
			for (int i = 1; i < maxNumSteps - 1; i++)
			{
				double step = currentState->motions[m]->duration() / double(maxNumSteps - 1);
				currentState->keys[m].push_back(step * i);
				ss << step * i << " ";
			}
			currentState->keys[m].push_back(currentState->motions[m]->duration());
			ss << currentState->motions[m]->duration() << " ";
			LOG("%s", ss.str().c_str());
		}

		motion->disconnect();
	}
	footStepEditor->stateEditor->buttonUndoAutoFootSteps->activate();
	footStepEditor->stateEditor->refresh();
	footStepEditor->hide();	
}

void PAAutoFootStepsEditor::cancelEditting(Fl_Widget* widget, void* data)
{
	PAAutoFootStepsEditor* footStepEditor = (PAAutoFootStepsEditor*) data;
	footStepEditor->stateEditor->refresh();
	footStepEditor->stateEditor->autoFootStepsEditor = NULL;
	footStepEditor->hide();
	delete footStepEditor;
}

bool PAAutoFootStepsEditor::kMeansClustering1D(int num, std::vector<double>& inputPoints, std::vector<double>& outMeans)
{
	if ((int)inputPoints.size() < num)
	{
		LOG("PAAutoFootStepsEditor::kMeansClustering1D Warning: Input points are less than number of means");
		return false;
	}

	// pick initial point
	int step = inputPoints.size() / num;
	for (int i = 0; i < num; i++)
		outMeans.push_back(inputPoints[i * step]);

	double convergence = 0.1;
	calculateMeans(inputPoints, outMeans, convergence);

	outMeans.resize(num);
	return true;
}


void PAAutoFootStepsEditor::calculateMeans(std::vector<double>&inputPoints, std::vector<double>& means, double convergentValue)
{
	std::vector<std::vector<double>> partitionBin;
	partitionBin.resize(means.size());

	// partition
	for (size_t i = 0; i < inputPoints.size(); i++)
	{
		double minDist = 9999;
		int minDistId = -1;
		for (size_t j = 0; j < means.size(); j++)
		{
			double dist = fabs(inputPoints[i] - means[j]);
			if (dist < minDist)
			{
				minDist = dist;
				minDistId = j;
			}
		}
		if (minDistId >= 0)
		{
			partitionBin[minDistId].push_back(inputPoints[i]);
		}
	}

	// get new means
	std::vector<double> newMeans;
	newMeans.resize(means.size());

	double diff = 0.0f;
	for (size_t i = 0; i < means.size(); i++)
	{
		diff = diff + (means[i] - newMeans[i]) * (means[i] - newMeans[i]);
	}
	diff = sqrt(diff);
	
	if (diff < convergentValue)
		return;
	else
		calculateMeans(inputPoints, newMeans, convergentValue);
}