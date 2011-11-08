#include "FaceViewer.h"
#include <sbm/mcontrol_util.h>
#include <FL/Fl_Value_Slider.H>
#include <sstream>

FaceViewer::FaceViewer(int x, int y, int w, int h, char* name) : GenericViewer(x, y, w, h), Fl_Double_Window(x, y, w, h, name)
{
	begin();

	topGroup = new Fl_Group(0, 0, w, 40);
	topGroup->begin();	
		choiceCharacters = new Fl_Choice(100, 10, 100, 25, "Characters");
		choiceCharacters->callback(CharacterCB, this);
		buttonRefresh = new Fl_Button(220, 10, 60, 25, "Refresh");
		buttonRefresh->callback(RefreshCB, this);
		buttonReset = new Fl_Button(220, 10, 60, 25, "Reset");
		buttonReset->callback(ResetCB, this);

		buttonCommands = new Fl_Button(360, 10, 100, 24, "Show Commands");
		buttonCommands->callback(ShowCommandsCB, this);
	topGroup->end();

	bottomGroup = new Fl_Scroll(0, 45, w, h - 40);
	bottomGroup->begin();	
	bottomGroup->box(FL_DOWN_BOX);
		
	bottomGroup->end();	


	end();

	FaceViewer::RefreshCB(this, this);

	if (choiceCharacters->size() > 0)
	{
		choiceCharacters->value(0);
		FaceViewer::CharacterCB(this, this);
	}

}

FaceViewer::~FaceViewer()
{
}

void FaceViewer::CharacterCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	faceViewer->bottomGroup->clear();

	int curY = faceViewer->bottomGroup->y() + 25;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	const Fl_Menu_Item* menu = faceViewer->choiceCharacters->menu();
	SbmCharacter* character = mcu.getCharacter(menu[faceViewer->choiceCharacters->value()].label());
	if (character)
	{	
		int startIndex = character->viseme_channel_start_pos;
		int endIndex = character->viseme_channel_end_pos;
		
		if (startIndex < 0 || endIndex < 0)
			return;

		SmartBody::SBFaceDefinition* faceDefinition = character->getFaceDefinition();

		SkChannelArray& channels = character->getSkeleton()->channels();
		for (int c = startIndex; c <= endIndex; c++)
		{
			std::stringstream strstr;

			SkChannel& channel = channels[c];
			const std::string& jointName = channels.name(c).c_str();

			strstr << jointName;

			Fl_Value_Slider* slider = new Fl_Value_Slider(100, curY, 150, 25, _strdup(strstr.str().c_str()));
			slider->type(FL_HORIZONTAL);
			slider->align(FL_ALIGN_LEFT);
			slider->range(0.0, 1.0);
			slider->callback(FaceCB, faceViewer);
			faceViewer->bottomGroup->add(slider);

			// for visemes, add a viseme weight
			if (jointName.find_first_of("au_") != 0)
			{
				std::string weightLabel = strstr.str() + " weight";
				Fl_Value_Slider* weightSlider = new Fl_Value_Slider(330, curY, 100, 25, _strdup(weightLabel.c_str()));
				weightSlider->type(FL_HORIZONTAL);
				weightSlider->align(FL_ALIGN_LEFT);
				weightSlider->range(0.0, 1.0);
				weightSlider->callback(FaceWeightCB, faceViewer);
				faceViewer->bottomGroup->add(weightSlider);
				// set the initial weight
				if (faceDefinition)
				{
					float initialWeight = faceDefinition->getVisemeWeight(strstr.str());
					weightSlider->value(initialWeight);
				}
			}
			curY += 30;
			

			faceViewer->bottomGroup->damage(FL_DAMAGE_ALL);
		}
	}
}

void FaceViewer::RefreshCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	faceViewer->choiceCharacters->clear();

	mcuCBHandle& mcu = mcuCBHandle::singleton();

	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		faceViewer->choiceCharacters->add(character->getName().c_str());
	}
}

void FaceViewer::ResetCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	const Fl_Menu_Item* menu = faceViewer->choiceCharacters->menu();
	SbmCharacter* character = mcu.getCharacter(menu[faceViewer->choiceCharacters->value()].label());
	if (character)
	{
		int numSliders = faceViewer->bottomGroup->children();
		for (int c = 0; c < numSliders; c++)
		{
			Fl_Value_Slider* slider = dynamic_cast<Fl_Value_Slider*>(faceViewer->bottomGroup->child(c));
			if (slider)
			{
				slider->value(0);
				std::stringstream strstr;
				std::string name = slider->label();
				strstr << "char " << faceViewer->choiceCharacters->menu()[faceViewer->choiceCharacters->value()].label() << " viseme " << name << " " << slider->value();
				mcu.execute((char*) strstr.str().c_str());
			}

		}

	}
}

void FaceViewer::ShowCommandsCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	const Fl_Menu_Item* menu = faceViewer->choiceCharacters->menu();
	SbmCharacter* character = mcu.getCharacter(menu[faceViewer->choiceCharacters->value()].label());
	if (character)
	{
		SmartBody::SBFaceDefinition* faceDefinition = character->getFaceDefinition();
		if (!faceDefinition)
		{
			LOG("No face definition for character %s.", character->getName().c_str());
			return;
		}
		
		int numVisemes = faceDefinition->getNumVisemes();
		LOG("Commands for generating viseme weights for character %s.", character->getName().c_str());
		for (int v = 0; v < numVisemes; v++)
		{
			std::stringstream strstr;
			const std::string& visemeName = faceDefinition->getVisemeName(v);
			float weight = faceDefinition->getVisemeWeight(visemeName);
			strstr << "char " << character->getName() << " visemeweight " << visemeName << " " << weight;
			LOG(strstr.str().c_str());
		}
	}
}

void FaceViewer::FaceCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	Fl_Value_Slider* slider = dynamic_cast<Fl_Value_Slider*>(widget);
	if (!slider)
		return;

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::stringstream strstr;
	std::string name = slider->label();
	strstr << "char " << faceViewer->choiceCharacters->menu()[faceViewer->choiceCharacters->value()].label() << " viseme " << name << " " << slider->value();
	mcu.execute((char*) strstr.str().c_str());
	
}

void FaceViewer::FaceWeightCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	Fl_Value_Slider* weightSlider = dynamic_cast<Fl_Value_Slider*>(widget);
	if (!weightSlider)
		return;

	// get the name of the viseme
	std::string labelName = (const char*) weightSlider->label(); 
	std::string visemeName = "";
	int pos = labelName.find_last_of(" ");
	if (pos != std::string::npos)
	{
		visemeName = labelName.substr(0, pos);
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::stringstream strstr;
	strstr << "char " << faceViewer->choiceCharacters->menu()[faceViewer->choiceCharacters->value()].label()  << " " << " visemeweight " << visemeName  << " " << weightSlider->value();
	mcu.execute((char*) strstr.str().c_str());
}


void FaceViewer::show_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	show();
}

void FaceViewer::hide_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	hide();
}

GenericViewer* FaceViewerFactory::create(int x, int y, int w, int h)
{
	FaceViewer* faceViewer = new FaceViewer(x, y, w, h, (char*)"Face View");
	return faceViewer;
}

void FaceViewerFactory::destroy(GenericViewer* viewer)
{
	delete viewer;
}

FaceViewerFactory::FaceViewerFactory()
{

}


