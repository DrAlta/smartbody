#include "vhcl.h"
#include "RetargetViewer.h"
#include <sb/SBBehaviorSetManager.h>
#include <sb/SBBehaviorSet.h>
#include <sb/SBScene.h>
#include <FL/Fl_Check_Button.H>
#include <sstream>
#include <cstring>

RetargetViewer::RetargetViewer(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h, name)
{

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();


	begin();

	int curY = 10;

	_choiceCharacters = new Fl_Choice(110, curY, 150, 20, "Character");
	//choiceCharacters->callback(CharacterCB, this);
	std::vector<std::string> characters = scene->getCharacterNames();
	for (size_t c = 0; c < characters.size(); c++)
	{
		_choiceCharacters->add(characters[c].c_str());
	}
	curY += 25;

	_choiceSkeletons = new Fl_Choice(110, curY, 150, 20, "Skeleton");
	//choiceSkeleton->callback(SkeletonCB, this);
	std::vector<std::string> skeletons = scene->getSkeletonNames();
	for (size_t c = 0; c < skeletons.size(); c++)
	{
		_choiceSkeletons->add(skeletons[c].c_str());
	}
	curY += 25;


	SmartBody::SBBehaviorSetManager* behavMgr = SmartBody::SBScene::getScene()->getBehaviorSetManager();
	std::map<std::string, SmartBody::SBBehaviorSet*>& behavSets = behavMgr->getBehaviorSets();

	_scrollGroup = new Fl_Scroll(10, curY, this->w() - 20, 150, "");
	_scrollGroup->type(Fl_Scroll::VERTICAL);
	_scrollGroup->begin();

	for (std::map<std::string, SmartBody::SBBehaviorSet*>::iterator iter = behavSets.begin();
		 iter != behavSets.end();
		 iter++)
	{
		std::string name = (*iter).first;
		Fl_Check_Button* check = new Fl_Check_Button(20, curY, 100, 20, _strdup(name.c_str()));
		curY += 25;
	}
	_scrollGroup->end();

	Fl_Button* buttonRetarget = new Fl_Button(20, curY, 60, 20, "Retarget");
	buttonRetarget->callback(RetargetCB, this);
	Fl_Button* buttonCancel = new Fl_Button(100, curY, 60, 20, "Cancel");
	buttonCancel->callback(CancelCB, this);

	end();
}

RetargetViewer::~RetargetViewer()
{
}

void RetargetViewer::setCharacterName(const std::string& name)
{
	_charName = name;
	for (int c = 0; c < _choiceCharacters->size(); c++)
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
	for (int c = 0; c < _choiceSkeletons->size(); c++)
	{
		if (name == _choiceSkeletons->text(c))
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
				if (behavSet)
				{
					LOG("Retargetting %s...", check->label());
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
	viewer->hide();
}

void RetargetViewer::CancelCB(Fl_Widget* widget, void* data)
{
	RetargetViewer* viewer = (RetargetViewer*) data;

	viewer->hide();
}
