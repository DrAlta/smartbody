#include "CharacterCreatorWindow.h"
#include <sbm/mcontrol_util.h>
#include <sstream>
#include <fltk/ask.h>

CharacterCreatorWindow::CharacterCreatorWindow(int x, int y, int w, int h, char* name) : Window(x, y, w, h, name)
{
	begin();
	browserSkeletons = new fltk::Browser(10, 30, 300, 300, "Skeletons");
	inputName = new fltk::Input(100, 360, 100, 25, "Name");
	buttonCreate = new fltk::Button(10, 390, 60, 25, "Create");
	buttonCreate->callback(CreateCB, this);
	end();
}

CharacterCreatorWindow::~CharacterCreatorWindow()
{
}

void CharacterCreatorWindow::setSkeletons(std::vector<std::string>& skeletonNames)
{
	browserSkeletons->clear();

	for (size_t x = 0; x < skeletonNames.size(); x++)
	{
		browserSkeletons->add(skeletonNames[x].c_str());
	}
}

void CharacterCreatorWindow::CreateCB(fltk::Widget* w, void* data)
{
	CharacterCreatorWindow* creator = (CharacterCreatorWindow*) (data);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if (creator->inputName->size() == 0)
	{
		fltk::alert("Please enter a character name.");
		return;
	}

	SbmCharacter* character = mcu.character_map.lookup(creator->inputName->value());
	if (character)
	{
		fltk::alert("Character name already exists.");
		return;
	}

	std::string skel = creator->browserSkeletons->child(creator->browserSkeletons->value())->label();
	

	std::stringstream strstr;
	strstr << "char " << creator->inputName->value() << " init " << skel;
	mcu.execute((char*) strstr.str().c_str());

}
