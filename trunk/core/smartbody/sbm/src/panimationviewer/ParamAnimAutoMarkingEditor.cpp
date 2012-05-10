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
	isPrintDebugInfo = false;

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
	
	browserJoint = new Fl_Multi_Browser(xDis + csx + 100, 13 * yDis, 28 * xDis, 20 * yDis, "Joint");
	SBCharacter* character = stateEditor->paWindow->getCurrentCharacter();
	if (character)
	{
		const std::vector<std::string>& charJNames = character->getSkeleton()->getJointNames();
		for (size_t i = 0; i < charJNames.size(); i++)
			browserJoint->add(charJNames[i].c_str());
	}

	inputStepsPerJoint = new Fl_Input(xDis + csx + 100, 36 * yDis, 10 * xDis, 2 * yDis, "StepsPerJoint");
	PAState* curState = stateEditor->getCurrentState();
	std::vector<std::string> selectedMotions = stateEditor->getSelectedMotions();
	if (selectedMotions.size() == curState->getNumMotions())
		inputStepsPerJoint->activate();
	else
		inputStepsPerJoint->deactivate();
	int stepsPerJoint = 2;
	std::stringstream ss;
	ss << stepsPerJoint;
	inputStepsPerJoint->value(ss.str().c_str());


	browserSelectedMotions = new Fl_Browser(xDis + csx + 100, 39 * yDis, 28 * xDis, 10 * yDis, "SelectedMotions");
	browserSelectedMotions->deactivate();
	refreshSelectedMotions();

	checkDebugInfo = new Fl_Check_Button(xDis + csx + 100, 51 * yDis, 10 * xDis, 2 * yDis, "DumpDetailInformation");

	buttonConfirm = new Fl_Button(xDis + csx, 55 * yDis, 10 * xDis, 2 * yDis, "Apply");
	buttonConfirm->callback(confirmEditting, this);
	buttonCancel = new Fl_Button(xDis + csx + 120, 55 * yDis, 10 * xDis, 2 * yDis, "Leave");
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
	int stepsPerJoint = atoi(footStepEditor->inputStepsPerJoint->value());
	if (stepsPerJoint < 1)
		stepsPerJoint = 1;
	int checkDebugInfoVal = footStepEditor->checkDebugInfo->value();
	if (checkDebugInfoVal == 0)
		footStepEditor->isPrintDebugInfo = false;
	if (checkDebugInfoVal == 1)
		footStepEditor->isPrintDebugInfo = true;

	const std::vector<std::string>& selectedMotions = footStepEditor->stateEditor->getSelectedMotions();
	SBCharacter* curCharacter = footStepEditor->stateEditor->paWindow->getCurrentCharacter();
	std::vector<std::string> selectedJoints;
	for (int i = 0; i < footStepEditor->browserJoint->size(); i++)
	{
		if (footStepEditor->browserJoint->selected(i+1))
		{
			selectedJoints.push_back(footStepEditor->browserJoint->text(i + 1));
		}
	}
	if (selectedJoints.size() == 0)
	{
		fl_alert("Please select at least one joint.");
		return;
	}

	bool isConvergent = true;
	for (size_t m = 0; m < selectedMotions.size(); m++)
	{
		// shared
		std::vector<double> possibleTiming;

		// divided
		std::vector<std::vector<double> > vecOutMeans;
		vecOutMeans.resize(selectedJoints.size());
		std::vector<std::vector<double> > vecTiming;
		vecTiming.resize(selectedJoints.size());

		SBMotion* motion = SmartBody::SBScene::getScene()->getMotion(selectedMotions[m]);
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

				// get height
				const SrMat& gMat = joint->gmat();
				SrVec gPos = SrVec(gMat.get(12), gMat.get(13), gMat.get(14));

				// get speed
				int startFrame = f - speedWindow / 2;
				int endFrame = f + speedWindow / 2;
				float startTime = startFrame * (float)motion->getFrameRate();
				float endTime = endFrame * (float)motion->getFrameRate();
				float speed = motion->getJointSpeed(joint, startTime, endTime);
				
				// print info
				if (footStepEditor->isPrintDebugInfo)
					LOG("motion %s at time %f-> speed is %f, height is %f, joint is %s", motion->getName().c_str(), f * motion->getFrameRate(), speed, gPos.y, jointName.c_str());

				// filter for height
				if (gPos.y < floorHeight || gPos.y > (floorHeight + heightThresh))
					continue;

				// filter speed
				if (speed <= speedThresh)
				{
					vecTiming[jointId].push_back(f * (float)motion->getFrameRate());
					possibleTiming.push_back(f * (float)motion->getFrameRate());
				}
			}
		}

		// K means algorithm according to desired number of steps
		std::vector<double> outMeans;
		int maxNumSteps = footStepEditor->stateEditor->getCurrentState()->getNumKeys();
		if (!footStepEditor->isProcessAll)
		{
			stepsPerJoint = maxNumSteps / selectedJoints.size();
		}
		else
		{
			maxNumSteps = stepsPerJoint * selectedJoints.size();
		}

		/*
		int numSteps = footStepEditor->stateEditor->getCurrentState()->getNumKeys();
		isConvergent = footStepEditor->kMeansClustering1D(numSteps, possibleTiming, outMeans);
		*/

		for (size_t jointId = 0; jointId < selectedJoints.size(); jointId++)
		{
			if (jointId == (selectedJoints.size() - 1) && !footStepEditor->isProcessAll)
			{
				int mod = footStepEditor->stateEditor->getCurrentState()->getNumKeys() % selectedJoints.size();
				stepsPerJoint += mod;
			}

			bool retBoolean = footStepEditor->kMeansClustering1D(stepsPerJoint, vecTiming[jointId], vecOutMeans[jointId]);
			if (!retBoolean)
			{
				isConvergent = false;
				break;
			}
		}
		if (isConvergent)
		{
			outMeans.clear();
			for (size_t joinId = 0; joinId < selectedJoints.size(); joinId++)
			{
				for (size_t meanId = 0; meanId < vecOutMeans[joinId].size(); meanId++)
					outMeans.push_back(vecOutMeans[joinId][meanId]);
			}
			std::sort(outMeans.begin(), outMeans.end());
		}

		// apply it to corresponding points
		// also appending starting and ending corresponding points
		int motionIndex = currentState->getMotionId(selectedMotions[m]);
		if (isConvergent)
		{
			std::stringstream ss;
			ss << "State " << currentState->stateName << " motion " << motion->getName() << " auto foot steps detected: ";
			for (size_t i = 0; i < outMeans.size(); i++)
				ss << outMeans[i] << " ";
			LOG("%s", ss.str().c_str());
			currentState->keys[motionIndex].clear();
			if (footStepEditor->isProcessAll)
				currentState->keys[motionIndex].push_back(0);
			for (size_t i = 0; i < outMeans.size(); i++)
				currentState->keys[motionIndex].push_back(outMeans[i]);
			if (footStepEditor->isProcessAll)
				currentState->keys[motionIndex].push_back(motion->getDuration());
		}
		else
		{
			std::stringstream ss;
			ss << "State " << currentState->stateName << " motion " << motion->getName() << " auto foot steps not detected(evenly distributed): ";
			int actualNum = maxNumSteps;
			currentState->keys[motionIndex].clear();
			ss << 0 << " ";
			if (footStepEditor->isProcessAll)
				actualNum += 2;
			for (int i = 0; i < actualNum; i++)
			{
				double step = motion->getDuration() / double(actualNum - 1);
				currentState->keys[motionIndex].push_back(step * i);
				ss << step * i << " ";
			}
			ss << motion->getDuration() << " ";
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

void PAAutoFootStepsEditor::refreshSelectedMotions()
{
	browserSelectedMotions->clear();
	const std::vector<std::string>& selectedMotions = stateEditor->getSelectedMotions();
	for (size_t i = 0; i < selectedMotions.size(); i++)
		browserSelectedMotions->add(selectedMotions[i].c_str());

	if (stateEditor->getCurrentState()->getNumMotions() == stateEditor->getSelectedMotions().size())
	{
		inputStepsPerJoint->activate();
		isProcessAll = true;
	}
	else
	{
		inputStepsPerJoint->deactivate();
		isProcessAll = false;
	}
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
	std::vector<std::vector<double> > partitionBin;
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
	for (size_t i = 0; i < means.size(); i++)
	{
		double newMean = 0;
		for (size_t j = 0; j < partitionBin[i].size(); j++)
			newMean += partitionBin[i][j];
		newMean /= double(partitionBin[i].size());
		newMeans.push_back(newMean);
	}

	double diff = 0.0f;
	for (size_t i = 0; i < means.size(); i++)
	{
		diff = diff + (means[i] - newMeans[i]) * (means[i] - newMeans[i]);
	}
	diff = sqrt(diff);
	
	if (diff < convergentValue)
		return;
	else
	{
		means = newMeans;
		calculateMeans(inputPoints, means, convergentValue);
	}
}
