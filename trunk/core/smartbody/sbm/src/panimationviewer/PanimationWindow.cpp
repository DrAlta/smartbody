/*
 *  PanimationWindow.cpp - part of SmartBody-lib's Test Suite
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


#include "vhcl.h"
#include "PanimationWindow.h"
#include "ParamAnimBlock.h"

#include <sbm/mcontrol_util.h>
#include <sbm/bml.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

PanimationWindow::PanimationWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(w, h, name), GenericViewer(x, y, w, h)
{
	this->begin();
		int tabGroupX = 10;
		int tabGroupY = 30;
		int tabGroupW = w - 20;
		int tabGroupH = 3 * h / 4 - 40;
		int childGroupX = 0;
		int childGroupY = 2 * yDis ;
		int childGroupW = tabGroupW- 10;
		int childGroupH = tabGroupH - 2 * yDis;

		motionPlayerMode = new Fl_Check_Button(10, 3 * h / 4 + 10, 200, 20, "Motion Player Mode");
		motionPlayerMode->value(0);
		motionPlayerMode->callback(changeMotionPlayerMode, this);
		characterList = new Fl_Choice(300, 3 * h / 4 + 10, 200, 20, "Character List");
		loadCharacters(characterList);
		
		refresh = new Fl_Button(550, 3 * h / 4 + 10, 100, 20, "Refresh");
		refresh->callback(refreshUI, this);
		resetCharacter = new Fl_Button(670, 3 * h / 4 + 10, 100, 20, "Reset");
		resetCharacter->callback(reset, this);
		textDisplay = new Fl_Text_Display(10, 3 * h / 4 + 40, w - 120, h / 4 - 60);
		textDisplay->color(FL_WHITE);
		textDisplay->textcolor(FL_BLUE);
		textBuffer = new Fl_Text_Buffer();
		textDisplay->buffer(textBuffer);
		
		// for some reason, this line causes fltk 1.3 to slow down significantly when many lines of commands are inserted into display text.				
		// this does not happen in fltk 2.0. Need to find out the correct way of doing this.
		//textDisplay->wrap_mode(true, 0);

		this->resizable(textDisplay);
		clearHistoryButton = new Fl_Button(w - 100, h - 90, 80, 20, "Clear");
		clearHistoryButton->callback(clearTextDisplay, this);

		tabGroup = new Fl_Tabs(tabGroupX, tabGroupY, tabGroupW, tabGroupH);
		tabGroup->begin();
			stateEditor = new PAStateEditor(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, this);
			stateEditor->begin();
			stateEditor->end();
			transitionEditor = new PATransitionEditor(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, this);
			transitionEditor->begin();
			transitionEditor->end();
			scriptEditor = new PAScriptEditor(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, this);
			scriptEditor->begin();
			scriptEditor->end();
			runTimeEditor = new PARunTimeEditor(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, this);
			runTimeEditor->begin();
			runTimeEditor->end();
		tabGroup->end();
	this->end();
	this->resizable(tabGroup);
	this->size_range(800, 740);
	redraw();

//	tabGroup->selected_child(stateEditor);
//	tabGroup->selected_child(transitionEditor);
//	tabGroup->selected_child(scriptEditor);
	tabGroup->value(runTimeEditor);
	lastCommand = "";
}


PanimationWindow::~PanimationWindow()
{
}

void PanimationWindow::draw()
{
	motionPlayerUpdate();	
	Fl_Double_Window::draw();   
}

void PanimationWindow::label_viewer(std::string name)
{
	this->label(strdup(name.c_str()));
}

void PanimationWindow::show_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	this->show();
}

void PanimationWindow::hide_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(NULL, NULL);
	this->hide();
}

void PanimationWindow::update_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string charName = characterList->menu()[characterList->value()].label();
	SbmCharacter* character = mcu.character_map.lookup(charName.c_str());
	if (!character)
		return;

	if (tabGroup->value() == scriptEditor)
		scriptEditor->update();
	if (tabGroup->value() ==runTimeEditor)
		runTimeEditor->update();
		
}

void PanimationWindow::show()
{    
    Fl_Double_Window::show();   
}

void PanimationWindow::motionPlayerUpdate()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (motionPlayerMode->value())
	{
		std::string charName = characterList->menu()[characterList->value()].label();
		std::string motionName = "";
		std::stringstream command;
		double time = 0.0;
		if (tabGroup->value() == stateEditor)
			getSelectedMarkInfo(stateEditor->stateEditorNleModel, motionName, time);
		if (tabGroup->value() == transitionEditor)
			getSelectedMarkInfo(transitionEditor->transitionEditorNleModel, motionName, time);
		if (motionName != "")
		{
			std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(motionName);
			if (time > iter->second->duration())
				time -= iter->second->duration();
			double delta = iter->second->duration() / double(iter->second->frames() - 1);
			int frameNumber = int(time / delta);
			command << "motionplayer " << charName << " " << motionName << " " << frameNumber;
			execCmd(this, command.str(), 0);
		}
	}
}

void PanimationWindow::getSelectedMarkInfo(nle::NonLinearEditorModel* model, std::string& blockName, double& time)
{
	for (int i = 0; i < model->getNumTracks(); i++)
	{
		nle::Track* track = model->getTrack(i);
		nle::Block* block = track->getBlock(0);
		for (int j = 0; j < block->getNumMarks(); j++)
		{
			nle::Mark* mark = block->getMark(j);
			if (mark->isSelected())
			{
				blockName = block->getName();
				time = mark->getStartTime();
				break;
			}
		}
		if (blockName != "")
			break;
	}
}

bool PanimationWindow::checkCommand(std::string command)
{
	if (lastCommand == command)
		return false;
	else
		lastCommand = command;
	return true;
}

void PanimationWindow::execCmd(PanimationWindow* window, std::string cmd, double tOffset)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	Fl_Text_Display* display = window->textDisplay;	
	BML::SbmCommand* command = new BML::SbmCommand(cmd, (float)(mcu.time + tOffset));
	bool success = true;
	srCmdSeq *seq = new srCmdSeq(); //sequence that holds the commands
	if( command != NULL ) 
	{
		if( seq->insert( (float)(command->time), command->command.c_str() ) != CMD_SUCCESS ) 
		{
			std::stringstream strstr;
			strstr << "ERROR: PanimationWindow::generateBML \""
			     << "Failed to insert SbmCommand \"" << (command->command) << "\" at time " << (command->time) << "Aborting remaining commands.";
			LOG(strstr.str().c_str());
			success = false;
		}
		delete command;
	}
	if( success )
	{
		if( mcu.execute_seq(seq) != CMD_SUCCESS ) 
			LOG("ERROR: PanimationWindow::generateBML: Failed to execute sequence.");
		else
		{
			bool shouldAppend = window->checkCommand(cmd);
			if (shouldAppend)
			{				
				display->insert(cmd.c_str());
				display->insert("\n");
			}
		}
		display->redraw();
		display->show_insert_position();
	}
}

void PanimationWindow::addTimeMark(nle::NonLinearEditorModel* model, bool selective)
{
	for (int i = 0; i < model->getNumTracks(); i++)
	{
		if (selective && !model->getTrack(i)->isSelected())
			continue;

		nle::Block* block = model->getTrack(i)->getBlock(0);
		CorrespondenceMark* toAddMark = new CorrespondenceMark();
		toAddMark->setStartTime(0.0);
		toAddMark->setEndTime(toAddMark->getStartTime());
		toAddMark->setColor(FL_RED);
		char buff[256];
		sprintf(buff, "%6.2f", toAddMark->getStartTime());
		toAddMark->setName(buff);
		toAddMark->setShowName(true);
		if (block)
			block->addMark(toAddMark);
	}
}

void PanimationWindow::removeTimeMark(nle::NonLinearEditorModel* model)
{
	CorrespondenceMark* attachedMark = NULL;
	for (int t = 0; t < model->getNumTracks(); t++)
	{
		nle::Track* track = model->getTrack(t);
		for (int b = 0; b < track->getNumBlocks(); b++)
		{
			nle::Block* block = track->getBlock(b);
			for (int m = 0; m < block->getNumMarks(); m++)
			{
				nle::Mark* mark = block->getMark(m);
				if (mark->isSelected())
				{
					CorrespondenceMark* cMark = dynamic_cast<CorrespondenceMark*>(mark);
					attachedMark = cMark->getAttachedMark();
					if (attachedMark)	attachedMark->attach(NULL);
					block->removeMark(mark);
				}
			}
		}
	}
}

void PanimationWindow::addTimeMarkToBlock(nle::Block* block, double t)
{
	CorrespondenceMark* mark = new CorrespondenceMark();
	mark->setStartTime(t);
	mark->setEndTime(mark->getStartTime());
	mark->setColor(FL_RED);
	char buff[256];
	sprintf(buff, "%6.2f", mark->getStartTime());
	mark->setName(buff);
	mark->setShowName(true);
	block->addMark(mark);	
}

std::vector<std::string> PanimationWindow::tokenize(const std::string& str,const std::string& delimiters)
{
	std::vector<string> tokens;
    	
	// skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	
	// find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
    	// found a token, add it to the vector.
    	tokens.push_back(str.substr(lastPos, pos - lastPos));
	
    	// skip delimiters.  Note the "not_of"
    	lastPos = str.find_first_not_of(delimiters, pos);
	
    	// find next "non-delimiter"
    	pos = str.find_first_of(delimiters, lastPos);
	}

	return tokens;
}

void PanimationWindow::refreshUI(Fl_Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	if (window->tabGroup->value() == window->stateEditor)
		window->stateEditor->refresh();
	if (window->tabGroup->value() == window->scriptEditor)
		window->scriptEditor->refresh();
}

void PanimationWindow::changeMotionPlayerMode(Fl_Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	std::string charName = window->characterList->menu()[window->characterList->value()].label();
	std::stringstream command;
	if (window->motionPlayerMode->value())
		command << "motionplayer " << charName << " on";
	else
		command << "motionplayer " << charName << " off";
	execCmd(window, command.str(), 0);
}

void PanimationWindow::loadCharacters(Fl_Choice* characterList)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* char_p;
	mcu.character_map.reset();
	while(char_p = mcu.character_map.next())
		characterList->add(char_p->name);
	characterList->value(0);
}

void PanimationWindow::clearTextDisplay(Fl_Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	window->textDisplay->buffer()->remove(0, window->textDisplay->buffer()->length());
	window->textDisplay->redraw();
}


void PanimationWindow::reset(Fl_Widget* widget, void* data)
{	
	PanimationWindow* window = (PanimationWindow*) data;
	std::string charName = window->characterList->menu()[window->characterList->value()].label();
	std::stringstream command;
	command << "panim unschedule char " << charName;
	execCmd(window, command.str());
	std::stringstream resetPosCommand;
	resetPosCommand << "set char " << charName << " world_offset x 0 z 0 h 0 p 0 r 0";
	execCmd(window, resetPosCommand.str());
}

PanimationWindow* PanimationWindow::getPAnimationWindow( Fl_Widget* w )
{
	PanimationWindow* panimWindow = NULL;
	Fl_Widget* widget = w;
	while (widget)
	{
		panimWindow = dynamic_cast<PanimationWindow*>(widget);
		if (panimWindow)
			break;
		else
			widget = widget->parent();
	}
	return panimWindow;
}

PanimationViewerFactory::PanimationViewerFactory()
{
}

GenericViewer* PanimationViewerFactory::create(int x, int y, int w, int h)
{
	PanimationWindow* panimationWindow = new PanimationWindow(x, y, w, h, (char*)"Parameterized Animation");
	return panimationWindow;
}

void PanimationViewerFactory::destroy(GenericViewer* viewer)
{
	delete viewer;
}


