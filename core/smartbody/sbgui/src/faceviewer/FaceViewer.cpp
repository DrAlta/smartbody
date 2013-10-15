#include "vhcl.h"
#include "FaceViewer.h"
#include <FL/Fl_Value_Slider.H>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sbm/action_unit.hpp>
#include <sbm/lin_win.h>

FaceViewer::FaceViewer(int x, int y, int w, int h, char* name) : GenericViewer(x, y, w, h), Fl_Double_Window(x, y, w, h, name), SBWindowListener()
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

	topGroup->end();

	bottomGroup = new Fl_Scroll(0, 45, w, h - 40);
	bottomGroup->begin();	
	bottomGroup->box(FL_DOWN_BOX);
		
	bottomGroup->end();	


	end();
	this->resizable(bottomGroup);

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
	faceViewer->_sliders.clear();
	faceViewer->_weights.clear();

	int curY = faceViewer->bottomGroup->y() + 25;
	const Fl_Menu_Item* menu = faceViewer->choiceCharacters->menu();
	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(menu[faceViewer->choiceCharacters->value()].label());
	if (character)
	{	
		SmartBody::SBFaceDefinition* faceDefinition = character->getFaceDefinition();
		if (!faceDefinition)
			return;

		std::vector<int>& auNums = faceDefinition->getAUNumbers();
		for (size_t a = 0; a < auNums.size(); a++)
		{
			ActionUnit* au = faceDefinition->getAU(auNums[a]);
			if (au->is_left())
			{
				Fl_Value_Slider* slider = new Fl_Value_Slider(100, curY, 150, 25, _strdup(au->getLeftName().c_str()));
				slider->type(FL_HORIZONTAL);
				slider->align(FL_ALIGN_LEFT);
				slider->range(0.0, 1.0);
				slider->callback(FaceCB, faceViewer);
				faceViewer->bottomGroup->add(slider);
				faceViewer->_sliders.push_back(slider);
				faceViewer->_weights.push_back(NULL);
				curY += 30;
			}

			if (au->is_right())
			{
				Fl_Value_Slider* slider = new Fl_Value_Slider(100, curY, 150, 25, _strdup(au->getRightName().c_str()));
				slider->type(FL_HORIZONTAL);
				slider->align(FL_ALIGN_LEFT);
				slider->range(0.0, 1.0);
				slider->callback(FaceCB, faceViewer);
				faceViewer->bottomGroup->add(slider);
				faceViewer->_sliders.push_back(slider);
				faceViewer->_weights.push_back(NULL);
				curY += 30;
			}

			if (au->is_bilateral())
			{
				Fl_Value_Slider* slider = new Fl_Value_Slider(100, curY, 150, 25, _strdup(au->getBilateralName().c_str()));
				slider->type(FL_HORIZONTAL);
				slider->align(FL_ALIGN_LEFT);
				slider->range(0.0, 1.0);
				slider->callback(FaceCB, faceViewer);
				faceViewer->bottomGroup->add(slider);
				faceViewer->_sliders.push_back(slider);
				faceViewer->_weights.push_back(NULL);
				curY += 30;
			}
			

		}

		std::vector<std::string>& visemeNames = faceDefinition->getVisemeNames();
		for (size_t v = 0; v < visemeNames.size(); v++)
		{
			Fl_Value_Slider* slider = new Fl_Value_Slider(100, curY, 150, 25, _strdup(visemeNames[v].c_str()));
			slider->type(FL_HORIZONTAL);
			slider->align(FL_ALIGN_LEFT);
			slider->range(0.0, 1.0);
			slider->callback(FaceCB, faceViewer);
			faceViewer->bottomGroup->add(slider);
			faceViewer->_sliders.push_back(slider);
			
			std::string weightLabel = visemeNames[v] + " weight";
			Fl_Value_Slider* weightSlider = new Fl_Value_Slider(330, curY, 100, 25, _strdup(weightLabel.c_str()));
			weightSlider->type(FL_HORIZONTAL);
			weightSlider->align(FL_ALIGN_LEFT);
			weightSlider->range(0.0, 1.0);
			weightSlider->callback(FaceWeightCB, faceViewer);
			faceViewer->bottomGroup->add(weightSlider);
			// set the initial weight
			float initialWeight = faceDefinition->getVisemeWeight(visemeNames[v]);
			weightSlider->value(initialWeight);
			faceViewer->_weights.push_back(weightSlider);

			curY += 30;
		}

		faceViewer->updateGUI();
		faceViewer->bottomGroup->damage(FL_DAMAGE_ALL);
	}
}

void FaceViewer::RefreshCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;
	faceViewer->choiceCharacters->clear();	
	const std::vector<std::string>& charNames = SmartBody::SBScene::getScene()->getCharacterNames();
	for (size_t i = 0; i < charNames.size(); i++)
	{
		const std::string & charName = charNames[i];
		faceViewer->choiceCharacters->add(charName.c_str());
	}
	if (charNames.size() > 0)
		faceViewer->choiceCharacters->activate();
	else
		faceViewer->choiceCharacters->deactivate();

	faceViewer->redraw();
}

void FaceViewer::ResetCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	const Fl_Menu_Item* menu = faceViewer->choiceCharacters->menu();
	SbmCharacter* character = SmartBody::SBScene::getScene()->getCharacter(menu[faceViewer->choiceCharacters->value()].label());
	if (character)
	{
		int numSliders = faceViewer->bottomGroup->children();
		for (int c = 0; c < numSliders; c++)
		{
			Fl_Value_Slider* slider = dynamic_cast<Fl_Value_Slider*>(faceViewer->bottomGroup->child(c));
			if (slider)
			{
				std::string name = slider->label();
				if (name.find(" weight") == std::string::npos)
					slider->value(0);

				//std::string message = vhcl::Format("char %s viseme %s %f", faceViewer->choiceCharacters->menu()[faceViewer->choiceCharacters->value()].label(), name.c_str(), slider->value());
				std::string message = vhcl::Format("char %s viseme clear", faceViewer->choiceCharacters->menu()[faceViewer->choiceCharacters->value()].label());
				if (!SmartBody::SBScene::getScene()->isRemoteMode())
				{
					SmartBody::SBScene::getScene()->command(message);
				}
				else
				{
					std::string sendStr = "send sbm " + message;
					SmartBody::SBScene::getScene()->command(sendStr);
				}
			}

		}

	}
}

void FaceViewer::FaceCB(Fl_Widget* widget, void* data)
{
	FaceViewer* faceViewer = (FaceViewer*) data;

	Fl_Value_Slider* slider = dynamic_cast<Fl_Value_Slider*>(widget);
	if (!slider)
		return;

	std::string name = slider->label();
	std::string message = vhcl::Format("char %s viseme %s %f", faceViewer->choiceCharacters->menu()[faceViewer->choiceCharacters->value()].label(), name.c_str(), slider->value());
	if (!SmartBody::SBScene::getScene()->isRemoteMode())
	{
		SmartBody::SBScene::getScene()->command(message);
	}
	else
	{
		std::string sendStr = "send sbm " + message;
		SmartBody::SBScene::getScene()->command(sendStr);
	}
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

	std::string message = vhcl::Format("char %s visemeweight %s %f", faceViewer->choiceCharacters->menu()[faceViewer->choiceCharacters->value()].label(), visemeName.c_str(), weightSlider->value());
	if (!SmartBody::SBScene::getScene()->isRemoteMode())
	{
		SmartBody::SBScene::getScene()->command(message);
	}
	else
	{
		std::string sendStr = "send sbm " + message;
		SmartBody::SBScene::getScene()->command(sendStr);
	}
}

void FaceViewer::updateGUI()
{
	if (!choiceCharacters->menu())
		return;

	std::string selectedCharacter = "";
	if (choiceCharacters->menu()->size() > 0)
	{
		selectedCharacter = choiceCharacters->menu()[choiceCharacters->value()].label();
	}

	if (selectedCharacter == "")
		return;

	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(selectedCharacter);
	if (character)
	{	
		SmartBody::SBSkeleton* skeleton = character->getSkeleton();
		SmartBody::SBFaceDefinition* faceDefinition = character->getFaceDefinition();

		for (size_t s = 0; s < _sliders.size(); s++)
		{
			Fl_Value_Slider* slider = _sliders[s];
			SmartBody::SBJoint* joint = skeleton->getJointByName(slider->label());
			if (joint)
			{
				SrVec position = joint->getPosition();
				if (slider->value() != position.x)
					slider->value(position.x);
			}

			if (_weights[s])
			{
				float weight = faceDefinition->getVisemeWeight(slider->label());
				_weights[s]->value(weight);
			}
		}
	}

}


void FaceViewer::show_viewer()
{
	show();
}

void FaceViewer::hide_viewer()
{
	hide();
}

void FaceViewer::show()
{
	SBWindowListener::windowShow();
	Fl_Double_Window::show();
}
void FaceViewer::hide()
{
	SBWindowListener::windowHide();
	Fl_Double_Window::hide();
}

void FaceViewer::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	std::string selectedCharacter = "";
	if (!choiceCharacters->menu())
		return;
	if (choiceCharacters->menu()->size() > 0)
	{
		selectedCharacter = choiceCharacters->menu()[choiceCharacters->value()].label();
	}
	RefreshCB(buttonRefresh, this);

	if (selectedCharacter != "")
	{
		for (int c = 0; c < choiceCharacters->size(); c++)
		{
			if (selectedCharacter == choiceCharacters->menu()[c].label())
			{
				choiceCharacters->value(c);
				CharacterCB(choiceCharacters, this);
			}
		}
	}
	else
	{
		choiceCharacters->value(0);
	}

}

void FaceViewer::OnCharacterDelete( const std::string & name )
{
	std::string selectedCharacter = "";
	if (!choiceCharacters->menu())
		return;

	if (choiceCharacters->menu()->size() > 0)
	{
		selectedCharacter = choiceCharacters->menu()[choiceCharacters->value()].label();
	}
	if (selectedCharacter == name)
	{
		choiceCharacters->deactivate();
		FaceViewer::CharacterCB(this, this);	
	}
}


void FaceViewer::OnCharacterUpdate( const std::string & name )
{
	OnCharacterCreate(name, "");
}

void FaceViewer::OnReset()
{
}

void FaceViewer::OnSimulationUpdate()
{
	updateGUI();
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
