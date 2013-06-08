#include "vhcl.h"
#include "RetargetViewer.h"
#include <sb/SBBehaviorSetManager.h>
#include <sb/SBBehaviorSet.h>
#include <sb/SBScene.h>
#include <FL/Fl_Check_Button.H>
#include <sstream>
#include <cstring>

#ifndef WIN32
#define _strdup strdup
#endif

RetargetViewer::RetargetViewer(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h, name)
{
	rootWindow = NULL;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	begin();

	int curY = 10;

	_choiceCharacters = new Fl_Choice(110, curY, 150, 20, "Character");
//	choiceCharacters->callback(CharacterCB, this);
	const std::vector<std::string>& characters = scene->getCharacterNames();
	for (size_t c = 0; c < characters.size(); c++)
	{
		_choiceCharacters->add(characters[c].c_str());
	}
	_choiceCharacters->callback(CharacterCB,this);
	curY += 25;

	_choiceSkeletons = new Fl_Choice(110, curY, 150, 20, "Skeleton");
//	choiceSkeleton->callback(SkeletonCB, this);
	std::vector<std::string> skeletons = scene->getSkeletonNames();
	for (size_t c = 0; c < skeletons.size(); c++)
	{
		_choiceSkeletons->add(skeletons[c].c_str());
	}
	_choiceSkeletons->callback(SkeletonCB,this);
	curY += 45;
	int choiceSize = _choiceSkeletons->size();


	SmartBody::SBBehaviorSetManager* behavMgr = SmartBody::SBScene::getScene()->getBehaviorSetManager();
	std::map<std::string, SmartBody::SBBehaviorSet*>& behavSets = behavMgr->getBehaviorSets();

	int scrollHeight = this->h() - curY - 50;
	_scrollGroup = new Fl_Scroll(10, curY, this->w() - 20, scrollHeight, "");
	_scrollGroup->type(Fl_Scroll::VERTICAL);
	_scrollGroup->begin();
	int itemWidth = this->w() - 40 - 20;
	for (std::map<std::string, SmartBody::SBBehaviorSet*>::iterator iter = behavSets.begin();
		 iter != behavSets.end();
		 iter++)
	{
		std::string name = (*iter).first;
		Fl_Check_Button* check = new Fl_Check_Button(40, curY, itemWidth, 20, _strdup(name.c_str()));
		curY += 25;
	}
	_scrollGroup->end();

	_retargetButton = new Fl_Button(40, curY, 120, 20, "Retarget");
	_retargetButton->callback(RetargetCB, this);
	_cancelButton = new Fl_Button(180, curY, 120, 20, "Cancel");
	_cancelButton->callback(CancelCB, this);

	end();
}

RetargetViewer::~RetargetViewer()
{
}

void RetargetViewer::setCharacterName(const std::string& name)
{
	_charName = name;	
	for (int c = 0; c < _choiceCharacters->size()-1; c++)
	{
		if (name == _choiceCharacters->text(c))
		{
			_choiceCharacters->value(c);
			break;
		}
	}
}

void RetargetViewer::setSkeletonName(const std::string& name)
{
	_skelName = name;	
	for (int c = 0; c < _choiceSkeletons->size()-1; c++)
	{
		std::string choiceName = _choiceSkeletons->text(c);
		if (name == choiceName)
		{
			_choiceSkeletons->value(c);
			break;
		}
	}
}

const std::string& RetargetViewer::getCharacterName()
{
	return _charName;
}

const std::string& RetargetViewer::getSkeletonName()
{
	return _skelName;
}

void RetargetViewer::setShowButton(bool showButton)
{
	if (showButton)
	{
		_retargetButton->show();
		_cancelButton->show();
	}
	else
	{
		_retargetButton->hide();
		_cancelButton->hide();
	}	
}


void RetargetViewer::RetargetCB(Fl_Widget* widget, void* data)
{
	RetargetViewer* viewer = (RetargetViewer*) data;

	SmartBody::SBBehaviorSetManager* behavMgr = SmartBody::SBScene::getScene()->getBehaviorSetManager();

	// run the script associated with any checked behavior sets
	int numChildren = viewer->_scrollGroup->children();
	for (int c = 0; c < numChildren; c++)
	{
		Fl_Check_Button* check = dynamic_cast<Fl_Check_Button*>(viewer->_scrollGroup->child(c));
		if (check)
		{
			if (check->value())
			{
				SmartBody::SBBehaviorSet* behavSet = behavMgr->getBehaviorSet(check->label());				
				if (behavSet && viewer->getCharacterName() != "" && viewer->getSkeletonName() != "")
				{
					LOG("Retargetting %s on %s with %s...", check->label(), viewer->getCharacterName().c_str(), viewer->getSkeletonName().c_str());
					const std::string& script = behavSet->getScript();
					SmartBody::SBScene::getScene()->runScript(script.c_str());
					std::stringstream strstr;
					strstr << "setupBehaviorSet()";
					SmartBody::SBScene::getScene()->run(strstr.str());
					std::stringstream strstr2;
					strstr2 << "retargetBehaviorSet('" << viewer->getCharacterName() << "', '" << viewer->getSkeletonName() << "')";
					SmartBody::SBScene::getScene()->run(strstr2.str());
				}
			}
		}
	}
	//viewer->hide();
	if (viewer->rootWindow)
	{
		viewer->rootWindow->hide();
	}
}

void RetargetViewer::CancelCB(Fl_Widget* widget, void* data)
{
	RetargetViewer* viewer = (RetargetViewer*) data;
	if (viewer->rootWindow)
	{
		viewer->rootWindow->hide();
	}
	//viewer->hide();
}

void RetargetViewer::CharacterCB( Fl_Widget* widget, void* data )
{
	RetargetViewer* viewer = (RetargetViewer*) data;	
	Fl_Choice* charChoice = dynamic_cast<Fl_Choice*>(widget);	
	viewer->setCharacterName(charChoice->text());
}

void RetargetViewer::SkeletonCB( Fl_Widget* widget, void* data )
{
	RetargetViewer* viewer = (RetargetViewer*) data;	
	Fl_Choice* skelChoice = dynamic_cast<Fl_Choice*>(widget);	
	viewer->setSkeletonName(skelChoice->text());
}