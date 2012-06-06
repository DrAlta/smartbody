// generated by Fast Light User Interface Designer (fluid) version 1.0300

#include <sb/SBScene.h>
#include <sb/SBFaceDefinition.h>
#include <sb/SBPhonemeManager.h>
#include <sbm/mcontrol_util.h>
#include <bml/bml_speech.hpp>

#include <FL/fl_device.H>
#include <FL/fl_draw.H>
#include <FL/Fl_File_Chooser.H>

#include "VisemeViewerWindow.h"

VisemeViewerWindow::VisemeViewerWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h)
{
	_phonemesSelected[0] = false;
	_phonemesSelected[1] = false;
	
	this->label("Diphone Viewer");
	this->begin();

	_sliderCurveAnimation = new Fl_Value_Slider(115, 460, 435, 30);
	_sliderCurveAnimation->type(5);
	_sliderCurveAnimation->textsize(14);
	_sliderCurveAnimation->deactivate();
	_sliderCurveAnimation->callback(OnSliderSelectCB, this);
	_checkEnableScrub = new Fl_Check_Button(40, 460, 65, 30, "Scrub");
	_checkEnableScrub->callback(OnEnableScrub, this);

	_buttonPlay = new Fl_Button(560, 460, 65, 30, "@>");
	_buttonPlay->callback(OnPlayCB, this);
	_inputPlayTime = new Fl_Input(630, 460, 60, 30, "");
	_inputPlayTime->value("0");

	_buttonPlayDialog = new Fl_Button(40, 500, 65, 30, "Speak");
	_buttonPlayDialog->callback(OnPlayDialogCB, this);
	_inputUtterance = new Fl_Input(115, 500, 435, 30);

	_choiceCharacter = new Fl_Choice(70, 35, 100, 25, "Character");
	_choiceCharacter->callback(OnCharacterSelectCB, this);

	_browserPhoneme[0] = new Fl_Hold_Browser(10, 80, 70, 350, "Phoneme 1");
	_browserPhoneme[0]->align(FL_ALIGN_TOP);
	_browserPhoneme[0]->callback(OnPhoneme1SelectCB, this);

	_browserPhoneme[1] = new  Fl_Hold_Browser(85, 80, 70, 350, "Phoneme 2");
	_browserPhoneme[1]->align(FL_ALIGN_TOP);
	_browserPhoneme[1]->callback(OnPhoneme2SelectCB, this);
	
	Fl_Menu_Bar* menuBar = new Fl_Menu_Bar(0, 0, w, 30);
	//menuBar->menu(menu_);
	menuBar->add("&File/Save", 0, OnSaveCB, this, NULL);
	//menuBar->callback(OnMenuSelectCB, this);

	_curveEditor = new VisemeCurveEditor(160, 80, 395, 355, "Animation Curve");
	_curveEditor->setVisemeWindow(this);
	_curveEditor->color(FL_GRAY0, FL_GRAY0);

	_browserViseme = new Fl_Multi_Browser(575, 80, 70, 350, "Visemes");
	_browserViseme->align(FL_ALIGN_TOP);
	_browserViseme->callback(OnVisemeSelectCB, this);

	_browserDiphone = new Fl_Hold_Browser(650, 80, 70, 350, "Diphones");
	_browserDiphone->align(FL_ALIGN_TOP); 
	_browserDiphone->callback(OnDiphoneSelectCB, this);

	this->end();


	loadData();
}

VisemeViewerWindow::~VisemeViewerWindow()
{
}

void VisemeViewerWindow::show()
{
	Fl_Double_Window::show();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(OnBmlRequestCB, this);
}

void VisemeViewerWindow::hide()
{
	Fl_Double_Window::hide();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(NULL, NULL);
}


Fl_Menu_Item VisemeViewerWindow::menu_[] = {
	{"File", 0,  0, 0, 64, FL_NORMAL_LABEL, 0, 14, 0},
	{"Save", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
	{"Load", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0}
};

void VisemeViewerWindow::draw()
{
	_curveEditor->redraw();

	Fl_Double_Window::draw();

	drawNames();
}

bool  VisemeViewerWindow::loadData()
{
	for (int x = 0; x < 2; x++)
	{
		_browserPhoneme[x]->add("Aa");   /// Viseme for aa
		_browserPhoneme[x]->add("Ah");   /// Viseme for aa, ae, ah
		_browserPhoneme[x]->add("Ao");
		_browserPhoneme[x]->add("Aw");   /// aw
		_browserPhoneme[x]->add("Ay");  /// ay
		_browserPhoneme[x]->add("BMP");
		_browserPhoneme[x]->add("D");
		_browserPhoneme[x]->add("EE");
		_browserPhoneme[x]->add("Eh");   /// ey, eh, uh
		_browserPhoneme[x]->add("Er");
		_browserPhoneme[x]->add("f");
		_browserPhoneme[x]->add("H");  /// h
		_browserPhoneme[x]->add("Ih");   /// y, iy, ih, ix
		_browserPhoneme[x]->add("j");
		_browserPhoneme[x]->add("kg");
		_browserPhoneme[x]->add("Ih");
		_browserPhoneme[x]->add("L");   /// l
		_browserPhoneme[x]->add("ng");
		_browserPhoneme[x]->add("Oh");
		_browserPhoneme[x]->add("OO");
		_browserPhoneme[x]->add("Ow");   /// ow
		_browserPhoneme[x]->add("Oy");  /// oy
		_browserPhoneme[x]->add("R");
		_browserPhoneme[x]->add("Sh");   /// sh, ch, jh, zh
		_browserPhoneme[x]->add("Th");
		_browserPhoneme[x]->add("W");
		_browserPhoneme[x]->add("Z");
		_browserPhoneme[x]->add("_");	/// silence
	}

	const std::vector<std::string>& characterNames = SmartBody::SBScene::getScene()->getCharacterNames();
	for (size_t i = 0; i < characterNames.size(); i++)
	{
		_choiceCharacter->add(characterNames[i].c_str());
	}
	if (characterNames.size() > 0)
	{
		_choiceCharacter->value(0);
		OnCharacterSelectCB(this->_choiceCharacter, this);
	}
	initializeVisemes();
	
	return true;
}

void VisemeViewerWindow::initializeVisemes()
{
	_curveEditor->generateCurves(_browserViseme->size());
}

void VisemeViewerWindow::resetViseme()
{
	std::string characterName = getCurrentCharacterName();
	for (int i = 0; i < _browserViseme->size(); i++)
	{
		std::stringstream strstr;
		strstr << "char " << characterName << " viseme " << _browserViseme->text(i + 1) << " " << 0.0f;
		SmartBody::SBScene::getScene()->command(strstr.str());		
	}
	_checkEnableScrub->value(0);
	_sliderCurveAnimation->value(0);
	_sliderCurveAnimation->deactivate();
}

SBCharacter* VisemeViewerWindow::getCurrentCharacter()
{
	int characterIndex = _choiceCharacter->value();
	if (characterIndex >= 0)
	{
		const char* characterName = _choiceCharacter->menu()[_choiceCharacter->value()].label();
		SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(characterName);	
		return character;
	}
	else
		return NULL;
}

std::string VisemeViewerWindow::getCurrentCharacterName()
{
	int characterIndex = _choiceCharacter->value();
	if (characterIndex >= 0)
	{
		const char* characterName = _choiceCharacter->menu()[_choiceCharacter->value()].label();
		return characterName;
	}
	else
		return "";
}

SBDiphone* VisemeViewerWindow::getCurrentDiphone()
{
	if (_browserPhoneme[0]->value() <= 0)
		return NULL;

	if (_browserPhoneme[1]->value() <= 0)
		return NULL;

	std::string phoneme1 = _browserPhoneme[0]->text(_browserPhoneme[0]->value());
	std::string phoneme2 = _browserPhoneme[1]->text(_browserPhoneme[1]->value());
	SBDiphone* diphone = SmartBody::SBScene::getScene()->getDiphoneManager()->getDiphone(phoneme1, phoneme2, getCurrentCharacterName());
	return diphone;
}

void VisemeViewerWindow::updateViseme()
{
	for(int i = 1; i <= _browserViseme->size(); i++)
	{
		if(_browserViseme->selected(i)> 0)
			_curveEditor->setVisibility(i - 1, true);
		else
			_curveEditor->setVisibility(i - 1, false);
	}
}

void VisemeViewerWindow::refreshData()
{
	SBDiphoneManager* diphoneManager = SmartBody::SBScene::getScene()->getDiphoneManager();
	SBDiphone* diphone = getCurrentDiphone();
	if (!diphone)
	{
		std::string phoneme1 = _browserPhoneme[0]->text(_browserPhoneme[0]->value());
		std::string phoneme2 = _browserPhoneme[1]->text(_browserPhoneme[1]->value());
		diphone = diphoneManager->createDiphone(phoneme1, phoneme2, getCurrentCharacterName());
	}
	else
	{
		diphone->clean();
	}

	for (int i = 0; i < _browserViseme->size(); i++)
	{
		if (_browserViseme->selected(i + 1))
		{
			std::vector<float>& key = diphone->getKeys(_browserViseme->text(i + 1));
			key.clear();

			for (size_t j = 0; j < _curveEditor->getCurves()[i].size(); j++)
			{
				key.push_back(_curveEditor->getCurves()[i][j].x);
				key.push_back(_curveEditor->getCurves()[i][j].y);
			}
		}
	}
}

bool VisemeViewerWindow::isPlayingViseme()
{
	int val = _checkEnableScrub->value();
	bool ret = false;
	if (val == 1)
		ret = true;
	return ret;
}

float VisemeViewerWindow::getSliderValue()
{
	return (float)_sliderCurveAnimation->value();
}

void VisemeViewerWindow::OnPhoneme1SelectCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;
	
	if(viewer->_browserPhoneme[0]->value() > 0)
		viewer->_phonemesSelected[0] = true;
	else 
	{
		viewer->_phonemesSelected[0] = false;
	}

	if(viewer->_phonemesSelected[0] && viewer->_phonemesSelected[1]){
		int lineSelected1 = viewer->_browserPhoneme[0]->value();
		int lineSelected2 = viewer->_browserPhoneme[1]->value();
		viewer->selectViseme(viewer->_browserPhoneme[0]->text(lineSelected1), viewer->_browserPhoneme[1]->text(lineSelected2));
	}
	viewer->resetViseme();
	viewer->updateViseme();
	viewer->draw();
}

void VisemeViewerWindow::OnPhoneme2SelectCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;
	
	if(viewer->_browserPhoneme[1]->value() > 0)
		viewer->_phonemesSelected[1] = true;
	else 
		viewer->_phonemesSelected[1] = false;

	if(viewer->_phonemesSelected[0] && viewer->_phonemesSelected[1]){
		int lineSelected1 = viewer->_browserPhoneme[0]->value();
		int lineSelected2 = viewer->_browserPhoneme[1]->value();
		viewer->selectViseme(viewer->_browserPhoneme[0]->text(lineSelected1), viewer->_browserPhoneme[1]->text(lineSelected2));
	}
	viewer->resetViseme();
	viewer->updateViseme();
	viewer->draw();
}


void VisemeViewerWindow::OnVisemeSelectCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;
	int viseme = viewer->_browserViseme->value();
	std::vector<float> curveData;

	SBDiphone* diphone = viewer->getCurrentDiphone();
	if (diphone)
	{
		const std::vector<std::string>& visemeNames = diphone->getVisemeNames();

		for (size_t i = 0; i < visemeNames.size(); i++)
		{
			if (visemeNames[i] == viewer->_browserViseme->text(viseme))
			{
				curveData = diphone->getKeys(visemeNames[i]);
				break;
			}
		}
	}

	viewer->_curveEditor->changeCurve(viseme - 1, curveData);
	viewer->refreshData();

	viewer->_curveEditor->selectLine(viseme - 1);	
	viewer->resetViseme();
	viewer->updateViseme();
	viewer->draw();
}

void VisemeViewerWindow::drawNames()
{
	std::vector<VisemeCurve>& curves = _curveEditor->getCurves();

	for (size_t i = 0; i < curves.size(); i++)
	{
		fl_color(FL_BLACK);

		if(!curves[i].isVisible())
			continue;

		SrVec point1 = _curveEditor->mapCurveData(curves[i][0]);

		fl_draw(_browserViseme->text((int)(i+1)), (int)point1.x, (int)point1.y - 10);
	}
}

void VisemeViewerWindow::selectViseme(const char * phoneme1, const char * phoneme2)
{
	if (_browserViseme->size() == 0)
		return;
	
	if (!phoneme1 || !phoneme2)
		return;

	SBDiphone* diphone = SmartBody::SBScene::getScene()->getDiphoneManager()->getDiphone(phoneme1, phoneme2, getCurrentCharacterName());
	if (diphone)
	{
		_browserViseme->deselect();
		const std::vector<std::string>& visemeNames = diphone->getVisemeNames();
		for (int i = 0; i < _browserViseme->size(); i++)
		{
			for (size_t j = 0; j < visemeNames.size(); j++)
			{
				if (visemeNames[j] == _browserViseme->text(i + 1))
				{
					_browserViseme->select(i + 1);

					_curveEditor->changeCurve(i, diphone->getKeys(visemeNames[j]));
					// set the curve visible
					_curveEditor->setVisibility(i, true);
					_curveEditor->selectLine(i);
					break;
				}
			}
		}
	}
	else
	{
		_browserViseme->deselect();
	}

	/*
	int viseme = rand() % _browserViseme->size() + 1;

	// unselect first
	_browserViseme->deselect();
	_browserViseme->select(viseme);

	_curveEditor->setVisibility(viseme - 1, true);
	_curveEditor->selectLine(viseme - 1);
	*/
}

void VisemeViewerWindow::selectPhonemes(const char * viseme)
{
	/*
	int phoneme1 = rand() % _browserPhoneme[0]->size() + 1;
	int phoneme2 = rand() % _browserPhoneme[1]->size() + 1;

	// unselect phonemes first
	_browserPhoneme[0]->deselect();
	_browserPhoneme[1]->deselect();

	_browserPhoneme[0]->select(phoneme1);
	_browserPhoneme[1]->select(phoneme2);
	*/
}

void VisemeViewerWindow::OnSliderSelectCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;
	SBDiphone* curDiphone = viewer->getCurrentDiphone();
	if (!curDiphone)
		return;

	std::string characterName = viewer->getCurrentCharacterName();
	const std::vector<std::string>& visemeNames = curDiphone->getVisemeNames();
	float sliderValue = (float)viewer->_sliderCurveAnimation->value();
	for (size_t i = 0; i < visemeNames.size(); i++)
	{
		float curveValue = 0.0f;
		std::vector<float>& key = curDiphone->getKeys(visemeNames[i]);
		for (size_t k = 0; k < key.size() / 2 - 1; k++)
		{
			if (key[k * 2] <= sliderValue && key[(k + 1) * 2] >= sliderValue)
			{
				float f = (sliderValue - key[k * 2]) / (key[(k + 1) * 2] - key[k * 2]);
				curveValue = f * (key[(k + 1) * 2 + 1] - key[k * 2 + 1]) + key[k * 2 + 1];
				break;
			}
		}
		std::stringstream strstr;
		strstr << "char " << characterName << " viseme " << visemeNames[i] << " " << curveValue;
		//LOG("%s", strstr.str().c_str());
		SmartBody::SBScene::getScene()->command(strstr.str());
	}
	viewer->_curveEditor->redraw();
}

void VisemeViewerWindow::OnCharacterSelectCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;

	SmartBody::SBCharacter* character = viewer->getCurrentCharacter();

	SmartBody::SBFaceDefinition* faceDefinition = character->getFaceDefinition();
	if (faceDefinition)
	{
		int numViseme = faceDefinition->getNumVisemes();
		viewer->_browserViseme->clear();
		for (int i = 0; i < numViseme; i++)
			viewer->_browserViseme->add(faceDefinition->getVisemeName(i).c_str());
	}
	viewer->initializeVisemes();
	viewer->_browserPhoneme[0]->deselect();
	viewer->_browserPhoneme[1]->deselect();
	viewer->resetViseme();
	viewer->updateViseme();
	viewer->_curveEditor->redraw();
}


void VisemeViewerWindow::OnEnableScrub(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;
	if (viewer->_checkEnableScrub->value() == 1)
	{
		viewer->_sliderCurveAnimation->activate();
	}
	else
	{	
		viewer->_sliderCurveAnimation->value(0);
		viewer->_sliderCurveAnimation->deactivate();
	}
}

void VisemeViewerWindow::OnPlayCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;

	float playTime = (float)atof(viewer->_inputPlayTime->value());
	if (playTime > 0)
	{
		for (int i = 0; i < viewer->_browserViseme->size(); i++)
		{
			if (viewer->_browserViseme->selected(i + 1))
			{
				std::stringstream strstr;
				strstr << "char " << viewer->getCurrentCharacterName() << " viseme " << viewer->_browserViseme->text(i + 1) << " curve ";
				strstr << viewer->_curveEditor->getCurves()[i].size() << " ";
				for (size_t j = 0; j < viewer->_curveEditor->getCurves()[i].size(); j++)
					strstr << viewer->_curveEditor->getCurves()[i][j].x * playTime << " " << viewer->_curveEditor->getCurves()[i][j].y << " ";
				SmartBody::SBScene::getScene()->command(strstr.str());		
			}
		}
	}
}


void VisemeViewerWindow::OnPlayDialogCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;
	std::string utterance = viewer->_inputUtterance->value(); 	
	if (utterance != "")
	{
		std::stringstream strstr;
		strstr << "python bml.execBML('" << viewer->getCurrentCharacterName() << "', '<speech type=\"text/plain\">" << viewer->_inputUtterance->value() << "</speech>')";
		SmartBody::SBScene::getScene()->command(strstr.str());
	}
}

void VisemeViewerWindow::OnSaveCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;
	const char* fileName = fl_file_chooser("Diphone file:", "*.py", NULL);
	if (!fileName)
		return;

	// fill in
	std::stringstream strstr;
	strstr << "# diphone for character" << viewer->getCurrentCharacterName() << "\n";
	strstr << "# autogenerated by SmartBody\n";
	strstr << "\n";
	strstr << "diphoneManager = scene.getDiphoneManager()\n";
	strstr << "\n";
	
	std::vector<SBDiphone*>& diphones = SmartBody::SBScene::getScene()->getDiphoneManager()->getDiphones(viewer->getCurrentCharacterName());
	for (size_t i = 0; i < diphones.size(); i++)
	{
		strstr << "diphone = diphoneManager.createDiphone(\"" << diphones[i]->getFromPhonemeName() << "\", \"" << diphones[i]->getToPhonemeName() << "\", \"" << viewer->getCurrentCharacterName() << "\")" << "\n";
		const std::vector<std::string>& visemeNames = diphones[i]->getVisemeNames();
		for (size_t n = 0; n < visemeNames.size(); n++)
		{
			std::vector<float>& key = diphones[i]->getKeys(visemeNames[n]);
			for (size_t k = 0; k < key.size() / 2; k++)
			{
				strstr << "diphone.addKey(\"" << visemeNames[n] << "\", " << key[k * 2 + 0] << ", " << key[k * 2 + 1] << ")" << "\n";
			}
			strstr << "\n";
		}
	}

	// save to the file
	std::ofstream file(fileName);
	if (file.is_open() != true)
	{
		fl_alert("Problem writing to file %s, diphone was not saved.", fileName);
		return;
	}
	file << strstr.str();
	file.close();	
}


void VisemeViewerWindow::OnBmlRequestCB(BML::BmlRequest* request, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;

	BML::VecOfBehaviorRequest b = request->behaviors;
	for (BML::VecOfBehaviorRequest::iterator iter = b.begin();
		iter != b.end();
		iter++)
	{
		BML::BehaviorRequestPtr requestPtr = (*iter);
		BML::BehaviorRequest* behavior = requestPtr.get();
		BML::SpeechRequest* speechRequest = dynamic_cast<BML::SpeechRequest*> (behavior);
		if (speechRequest)
		{
			viewer->_browserDiphone->clear();
			for ( size_t i = 0; i < (speechRequest->getPhonemes().size() - 1); i++ )
			{
				VisemeData* v1 = speechRequest->getPhonemes()[i];
				VisemeData* v2 = speechRequest->getPhonemes()[i + 1];
				std::stringstream strstr;
				strstr << v1->id() << " - " << v2->id();
				viewer->_browserDiphone->add(strstr.str().c_str());
			}
		}
	}
}

void VisemeViewerWindow::OnDiphoneSelectCB(Fl_Widget* widget, void* data)
{
	VisemeViewerWindow* viewer = (VisemeViewerWindow*) data;

	std::string str = viewer->_browserDiphone->text(viewer->_browserDiphone->value());
	std::vector<string> diphones;
	std::stringstream ss(str);

	string tok;

	while (ss >> tok)
	{
		if (tok == "-")
			continue;
        
		diphones.push_back(tok);
	}
	
	viewer->_browserPhoneme[0]->deselect();
	viewer->_browserPhoneme[1]->deselect();

	for(int i =0; i < 2; i++){
		for(int j = 1; j <= viewer->_browserPhoneme[0]->size(); j++){
			std::string diphone = viewer->_browserPhoneme[i]->text(j);
			
			if(diphone == diphones[i])
			{
				viewer->_browserPhoneme[i]->select(j);
				break;
			}
		}
	}

	OnPhoneme1SelectCB(widget, data);
}