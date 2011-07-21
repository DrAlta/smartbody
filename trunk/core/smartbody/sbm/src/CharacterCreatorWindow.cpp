#include "CharacterCreatorWindow.h"
#include <sbm/mcontrol_util.h>
#include <sstream>
#include <FL/fl_ask.H>

CharacterCreatorWindow::CharacterCreatorWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h, name)
{
	begin();
		choiceSkeletons = new Fl_Choice(100, 30, 300, 25, "Skeleton");
		choiceSkeletons->when(FL_WHEN_CHANGED);
		inputName = new Fl_Input(100, 70, 100, 25, "Name");
		buttonCreate = new Fl_Button(100, 95, 60, 25, "Create");
		buttonCreate->callback(CreateCB, this);
	end();
}

CharacterCreatorWindow::~CharacterCreatorWindow()
{
}

void CharacterCreatorWindow::setSkeletons(std::vector<std::string>& skeletonNames)
{
	choiceSkeletons->clear();

	for (size_t x = 0; x < skeletonNames.size(); x++)
	{
		choiceSkeletons->add(skeletonNames[x].c_str());
	}
}

void CharacterCreatorWindow::CreateCB(Fl_Widget* w, void* data)
{
	CharacterCreatorWindow* creator = (CharacterCreatorWindow*) (data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (creator->inputName->size() == 0)
	{
		fl_alert("Please enter a character name.");
		return;
	}

	SbmCharacter* character = mcu.character_map.lookup(creator->inputName->value());
	if (character)
	{
		fl_alert("Character name already exists.");
		return;
	}

	std::string skel = creator->choiceSkeletons->menu()[creator->choiceSkeletons->value()].label();
	

	std::stringstream strstr;
	strstr << "char " << creator->inputName->value() << " init " << skel;
	mcu.execute((char*) strstr.str().c_str());

}
