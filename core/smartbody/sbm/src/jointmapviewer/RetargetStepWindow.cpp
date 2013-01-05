#include "RetargetStepWindow.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <bml/bml.hpp>
#include <sb/SBSkeleton.h>

RetargetStepWindow::RetargetStepWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(w, h, name)
{	
	int yDis = 10;
	int yOffset = 20;
	int xOffset = 10;

	int tabGroupX = 10;
	int tabGroupY = 10;
	int tabGroupW = w - 20;
	int tabGroupH = h - 80;

	int childGroupX = 0;
	int childGroupY = 3 * yDis ;
	int childGroupW = tabGroupW- 10;
	int childGroupH = tabGroupH - 4 * yDis;

	int windowGroupX = 20;
	int windowGroupY = 1 * yDis ;
	int windowGroupW = tabGroupW- 30;
	int windowGroupH = tabGroupH - 5 * yDis;

	tabGroup = new Fl_Tabs(tabGroupX, tabGroupY, tabGroupW, tabGroupH);
	//tabGroup->callback(changeTabGroup, this);
	tabGroup->begin();
		Fl_Group* firstGroup = new Fl_Group(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, "Joint Mapper");
		firstGroup->begin();	
		jointMapViewer = new JointMapViewer(windowGroupX + childGroupX + tabGroupX, windowGroupY + childGroupY + tabGroupY , windowGroupW, windowGroupH,"Joint Mapper");
		jointMapViewer->begin();
		jointMapViewer->end();
		firstGroup->end();

		Fl_Group* secondGroup = new Fl_Group(childGroupX + tabGroupX, childGroupY + tabGroupY, childGroupW, childGroupH, "Behavior Set");
		secondGroup->begin();
		retargetViewer = new RetargetViewer(windowGroupX + childGroupX + tabGroupX, windowGroupY + childGroupY + tabGroupY, windowGroupW, windowGroupH, "Behavior Set");		
		retargetViewer->begin();
		retargetViewer->end();
		secondGroup->end();
	tabGroup->end();
	this->resizable(tabGroup);
	//this->size_range(600, 740);
	
	jointMapViewer->rootWindow = this;
	retargetViewer->rootWindow = this;

	int curY = tabGroupH + 20;	
	_buttonApplyAll = new Fl_Button(100, curY, 120, 25, "Apply All");
	_buttonApplyAll->callback(ApplyCB, this);
// 	_buttonApplyMap = new Fl_Button(100, curY, 120, 25, "Apply Joint Map");
// 	_buttonApplyMap->callback(ApplyJointMapCB,this);
// 	_buttonApplyBehaviorSet = new Fl_Button(220, curY, 120, 25, "Apply Behavior Set");
// 	_buttonApplyBehaviorSet->callback(ApplyBehaviorSetCB,this);			
	_buttonCancel = new Fl_Button(360, curY, 120, 25, "Cancel");
	_buttonCancel->callback(CancelCB, this);

	retargetViewer->setShowButton(false);
	jointMapViewer->setShowButton(false);
	/*
	Fl_Group* firstGroup = new Fl_Group(xOffset, yOffset, w/2 - 20, h - 20, "Joint Mapper");
	firstGroup->begin();		
	jointMapViewer = new JointMapViewer(xOffset,yOffset, w/2 - 20, h - 20, "");
	firstGroup->resizable(jointMapViewer);
	firstGroup->end();
	this->resizable(firstGroup);

	//yOffset =  h/4+30; 
	xOffset += w/2+20;	
	Fl_Group* secondGroup = new Fl_Group(xOffset, yOffset, w/2 - 40 , h - 20, "Behavior Selection");
	secondGroup->begin();
	retargetViewer = new RetargetViewer(xOffset, yOffset, w/2 - 40 , h - 20, "");		
	secondGroup->end();
	*/
	//secondGroup->resizable(retargetViewer);
	//this->resizable(secondGroup);
	//this->size_range(800, 480);
}


void RetargetStepWindow::setApplyType( bool applyAll )
{
	if (applyAll)
	{
		_buttonApplyAll->show();
		_buttonCancel->show();
		retargetViewer->setShowButton(false);
		jointMapViewer->setShowButton(false);
		//_buttonApplyBehaviorSet->hide();
		//_buttonApplyMap->hide();

	}
	else 
	{
		_buttonApplyAll->hide();
		_buttonCancel->hide();
		retargetViewer->setShowButton(true);
		jointMapViewer->setShowButton(true);
		//_buttonApplyBehaviorSet->show();
		//_buttonApplyMap->show();
	}

}

RetargetStepWindow::~RetargetStepWindow()
{

}

void RetargetStepWindow::setCharacterName( std::string charName )
{
	jointMapViewer->setCharacterName(charName);
	retargetViewer->setCharacterName(charName);

}

void RetargetStepWindow::setSkeletonName( std::string skName )
{
	retargetViewer->setSkeletonName(skName);
}

void RetargetStepWindow::setJointMapName( std::string jointMapName )
{
	jointMapViewer->setJointMapName(jointMapName);
}

void RetargetStepWindow::applyRetargetSteps()
{
	jointMapViewer->applyJointMap();
	retargetViewer->RetargetCB(NULL,retargetViewer);
}

void RetargetStepWindow::ApplyCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->applyRetargetSteps();
	viewer->hide();	
}

void RetargetStepWindow::CancelCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->hide();
}

void RetargetStepWindow::draw()
{
	if (jointMapViewer)
		jointMapViewer->redraw();
	Fl_Double_Window::draw();
}

void RetargetStepWindow::ApplyJointMapCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->jointMapViewer->applyJointMap();
	viewer->hide();
}

void RetargetStepWindow::ApplyBehaviorSetCB( Fl_Widget* widget, void* data )
{
	RetargetStepWindow* viewer = (RetargetStepWindow*) data;
	viewer->retargetViewer->RetargetCB(NULL,viewer->retargetViewer);
	viewer->hide();
}