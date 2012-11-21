#include "MotionEditorWindow.h"
#include "sb/SBScene.h"
#include <FL/Fl_File_Chooser.H>

MotionEditorWindow::MotionEditorWindow(int x, int y, int w, int h, char* label) : Fl_Double_Window(x, y, w, h, label)
{
	this->label("Motion Editor");
	this->begin();
		_choiceCharacaterList = new Fl_Choice(80, 10, 100, 20, "Characters");
		_choiceCharacaterList->callback(OnChoiceCharacterList, this);
		_buttonRefresh = new Fl_Button(200, 10, 100, 20, "Refresh");
		_buttonRefresh->callback(OnButtonRefresh, this);
		_buttonSaveMotion = new Fl_Button(300, 10, 100, 20, "Save");
		_buttonSaveMotion->callback(OnButtonSaveMotion, this);
		_browserMotionList = new Fl_Hold_Browser(10, 40, 300, 200, "Motion List");
		_browserMotionList->callback(OnBrowserMotionList, this);
		_checkButtonPlayMotion = new Fl_Check_Button(10, 260, 50, 20, "Play");
		_checkButtonPlayMotion->callback(OnButtonPlayMotion, this);
		_sliderMotionFrame = new Fl_Value_Slider(60, 260, 300, 20);
		_sliderMotionFrame->type(FL_HORIZONTAL);
		_sliderMotionFrame->callback(OnSliderMotionFrame, this);
		_sliderMotionFrame->deactivate();

		_groupMetaInfo = new Fl_Group(10, 300, 400, 355, "Motion MetaData");
		int groupMetaInfoX = _groupMetaInfo->x();
		int groupMetaInfoY = _groupMetaInfo->y();
		_groupMetaInfo->begin();
			_sliderStart = new Fl_Value_Slider(80 + groupMetaInfoX, 10 + groupMetaInfoY, 240, 20, "start");
			_sliderStart->type(FL_HORIZONTAL);
			_sliderStart->align(FL_ALIGN_LEFT);
			_sliderStart->callback(OnSliderSyncPoints, this);
			_buttonGetStartTime = new Fl_Button(325 + groupMetaInfoX, 10 + groupMetaInfoY, 25, 20, "<<");
			_buttonGetStartTime->tooltip("start");
			_buttonGetStartTime->callback(OnButtonGetSyncPoints, this);

			_sliderReady = new Fl_Value_Slider(80 + groupMetaInfoX, 30 + groupMetaInfoY, 240, 20, "ready");
			_sliderReady->type(FL_HORIZONTAL);
			_sliderReady->align(FL_ALIGN_LEFT);
			_sliderReady->callback(OnSliderSyncPoints, this);
			_buttonGetReadyTime = new Fl_Button(325 + groupMetaInfoX, 30 + groupMetaInfoY, 25, 20, "<<");
			_buttonGetReadyTime->tooltip("ready");
			_buttonGetReadyTime->callback(OnButtonGetSyncPoints, this);

			_sliderStrokeStart = new Fl_Value_Slider(80 + groupMetaInfoX, 50 + groupMetaInfoY, 240, 20, "stroke_start");
			_sliderStrokeStart->type(FL_HORIZONTAL);
			_sliderStrokeStart->align(FL_ALIGN_LEFT);
			_sliderStrokeStart->callback(OnSliderSyncPoints, this);
			_buttonGetStrokeStartTime = new Fl_Button(325 + groupMetaInfoX, 50 + groupMetaInfoY, 25, 20, "<<");
			_buttonGetStrokeStartTime->tooltip("stroke_start");
			_buttonGetStrokeStartTime->callback(OnButtonGetSyncPoints, this);

			_sliderStroke = new Fl_Value_Slider(80 + groupMetaInfoX, 70 + groupMetaInfoY, 240, 20, "stroke");
			_sliderStroke->type(FL_HORIZONTAL);
			_sliderStroke->align(FL_ALIGN_LEFT);
			_sliderStroke->callback(OnSliderSyncPoints, this);
			_buttonGetStrokeTime = new Fl_Button(325 + groupMetaInfoX, 70 + groupMetaInfoY, 25, 20, "<<");
			_buttonGetStrokeTime->tooltip("stroke");
			_buttonGetStrokeTime->callback(OnButtonGetSyncPoints, this);

			_sliderStrokeEnd = new Fl_Value_Slider(80 + groupMetaInfoX, 90 + groupMetaInfoY, 240, 20, "stroke_stop");
			_sliderStrokeEnd->type(FL_HORIZONTAL);
			_sliderStrokeEnd->align(FL_ALIGN_LEFT);
			_sliderStrokeEnd->callback(OnSliderSyncPoints, this);
			_buttonGetStrokeEndTime = new Fl_Button(325 + groupMetaInfoX, 90 + groupMetaInfoY, 25, 20, "<<");
			_buttonGetStrokeEndTime->tooltip("stroke_stop");
			_buttonGetStrokeEndTime->callback(OnButtonGetSyncPoints, this);

			_sliderRelax = new Fl_Value_Slider(80 + groupMetaInfoX, 110 + groupMetaInfoY, 240, 20, "relax");
			_sliderRelax->type(FL_HORIZONTAL);
			_sliderRelax->align(FL_ALIGN_LEFT);
			_sliderRelax->callback(OnSliderSyncPoints, this);
			_buttonGetRelaxTime = new Fl_Button(325 + groupMetaInfoX, 110 + groupMetaInfoY, 25, 20, "<<");
			_buttonGetRelaxTime->tooltip("relax");
			_buttonGetRelaxTime->callback(OnButtonGetSyncPoints, this);

			_sliderEnd = new Fl_Value_Slider(80 + groupMetaInfoX, 130 + groupMetaInfoY, 240, 20, "stop");
			_sliderEnd->type(FL_HORIZONTAL);
			_sliderEnd->align(FL_ALIGN_LEFT);
			_sliderEnd->callback(OnSliderSyncPoints, this);
			_buttonGetEndTime = new Fl_Button(325 + groupMetaInfoX, 130 + groupMetaInfoY, 25, 20, "<<");
			_buttonGetEndTime->tooltip("stop");
			_buttonGetEndTime->callback(OnButtonGetSyncPoints, this);

			_browserMetaNames = new Fl_Hold_Browser(10 + groupMetaInfoX, 160 + groupMetaInfoY, 150, 100, "Extra Meta Names");
			_browserMetaNames->callback(OnBrowserMetaNames, this);
			_browserMetaValues = new Fl_Hold_Browser(200 + groupMetaInfoX, 160 + groupMetaInfoY, 150, 100, "Extra Meta Values");
			_browserMetaValues->callback(OnBrowserMetaValues, this);
			_inputMetaNames = new Fl_Input(100 + groupMetaInfoX, 280 + groupMetaInfoY, 250, 20, "MetaData Name:");
			_inputMetaValues = new Fl_Input(100 + groupMetaInfoX, 300 + groupMetaInfoY, 250, 20, "MetaData Value:");
			_buttonAddMetaEntry = new Fl_Button(80 + groupMetaInfoX, 325 + groupMetaInfoY, 100, 20, "Add");
			_buttonAddMetaEntry->callback(OnButtonAddMetaEntry, this);
			_buttonDeleteMetaEntry = new Fl_Button(200 + groupMetaInfoX, 325 + groupMetaInfoY, 100, 20, "Remove");
			_buttonDeleteMetaEntry->callback(OnButtonDeleteMetaEntry, this);
		_groupMetaInfo->end();
		_groupMetaInfo->box(FL_BORDER_BOX);
	this->end();

	loadCharacters();
	loadMotions();

	redraw();
}

MotionEditorWindow::~MotionEditorWindow()
{
}

void MotionEditorWindow::show()
{
	Fl_Double_Window::show();
}

void MotionEditorWindow::hide()
{
	Fl_Double_Window::hide();
}

void MotionEditorWindow::loadCharacters()
{
	const std::vector<std::string>& charNames = SmartBody::SBScene::getScene()->getCharacterNames();
	for (size_t i = 0; i < charNames.size(); ++i)
	{
		_choiceCharacaterList->add(charNames[i].c_str());
	}
	_choiceCharacaterList->value(0);
}

SmartBody::SBCharacter* MotionEditorWindow::getCurrentCharacter()
{
	std::string curCharName = _choiceCharacaterList->text();
	return SmartBody::SBScene::getScene()->getCharacter(curCharName);
}

void MotionEditorWindow::loadMotions()
{
	const std::vector<std::string>& motionNames = SmartBody::SBScene::getScene()->getMotionNames();
	for (size_t i = 0; i < motionNames.size(); ++i)
	{
		_browserMotionList->add(motionNames[i].c_str());
	}
}

SmartBody::SBMotion* MotionEditorWindow::getCurrentMotion()
{
	for (int i = 0; i < _browserMotionList->size(); ++i)
	{
		if (_browserMotionList->selected(i + 1))
		{
			return SmartBody::SBScene::getScene()->getMotion(_browserMotionList->text(i + 1));
		}
	}
	return NULL;
}

void MotionEditorWindow::OnChoiceCharacterList(Fl_Widget* widget, void* data)
{
	MotionEditorWindow::OnButtonRefresh(widget, data);
}

void MotionEditorWindow::OnButtonRefresh(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	editor->_browserMotionList->deselect();
	OnBrowserMotionList(widget, data);
	editor->_checkButtonPlayMotion->value(0);
	editor->_sliderMotionFrame->deactivate();
	OnButtonPlayMotion(widget, data);
}

void MotionEditorWindow::OnButtonSaveMotion(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	SmartBody::SBMotion* curMotion = editor->getCurrentMotion();
	if (!curMotion)
		return;
	curMotion->saveToSkm(curMotion->getMotionFileName());
}

void MotionEditorWindow::OnBrowserMotionList(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	editor->updateSyncPointsUI();
	editor->updateMetaDataUI();
	SmartBody::SBMotion* curMotion = editor->getCurrentMotion();
	if (!curMotion)
		return;
	double dur = curMotion->getDuration();
	editor->_sliderMotionFrame->range(0, dur);
	editor->redraw();
}

void MotionEditorWindow::OnButtonPlayMotion(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	SmartBody::SBCharacter* curChar = editor->getCurrentCharacter();
	if (!curChar)	return;
	SmartBody::SBMotion* curMotion = editor->getCurrentMotion();
	if (!curMotion)	
	{
		const std::vector<std::string>& charNames = SmartBody::SBScene::getScene()->getCharacterNames();
		std::stringstream ss;
		for (size_t i = 0; i < charNames.size(); ++i)
		{
			ss.str("");
			ss << "motionplayer " << charNames[i] << " off";
			SmartBody::SBScene::getScene()->command(ss.str());
		}
		return;
	}

	if (editor->_checkButtonPlayMotion->value() == 1)
	{
		editor->_sliderMotionFrame->activate();
		double playTime = editor->_sliderMotionFrame->value();
		double delta = curMotion->duration() / double(curMotion->frames() - 1);
		int frameNumber = int(playTime / delta);
		std::stringstream ss;
		ss << "motionplayer " << curChar->getName() << " on";
		SmartBody::SBScene::getScene()->command(ss.str());
		ss.str("");
		ss << "motionplayer " << curChar->getName() << " " << curMotion->getName() << " " << frameNumber;
		SmartBody::SBScene::getScene()->command(ss.str());
	}
	if (editor->_checkButtonPlayMotion->value() == 0)
	{
		std::stringstream ss;
		ss << "motionplayer " << curChar->getName() << " off";
		SmartBody::SBScene::getScene()->command(ss.str());
		editor->_sliderMotionFrame->deactivate();
		editor->_sliderMotionFrame->value(0);
	}
}

void MotionEditorWindow::OnSliderMotionFrame(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	SmartBody::SBCharacter* curChar = editor->getCurrentCharacter();
	if (!curChar)	return;
	SmartBody::SBMotion* curMotion = editor->getCurrentMotion();
	if (!curMotion)	return;

	double playTime = editor->_sliderMotionFrame->value();
	double delta = curMotion->duration() / double(curMotion->frames() - 1);
	int frameNumber = int(playTime / delta);
	std::stringstream ss;
	ss << "motionplayer " << curChar->getName() << " " << curMotion->getName() << " " << frameNumber;
	SmartBody::SBScene::getScene()->command(ss.str());
}

void MotionEditorWindow::updateSyncPointsUI()
{
	SmartBody::SBMotion* curMotion = getCurrentMotion();
	if (!curMotion)
	{
		_sliderStart->range(0, 1);
		_sliderReady->range(0, 1);
		_sliderStrokeStart->range(0, 1);
		_sliderStroke->range(0, 1);
		_sliderStrokeEnd->range(0, 1);
		_sliderRelax->range(0, 1);
		_sliderEnd->range(0, 1);
		_sliderStart->value(0);
		_sliderReady->value(0);
		_sliderStrokeStart->value(0);
		_sliderStroke->value(0);
		_sliderStrokeEnd->value(0);
		_sliderRelax->value(0);
		_sliderEnd->value(0);
		return;
	}
	double dur = curMotion->getDuration();
	_sliderStart->range(0, dur);
	_sliderReady->range(0, dur);
	_sliderStrokeStart->range(0, dur);
	_sliderStroke->range(0, dur);
	_sliderStrokeEnd->range(0, dur);
	_sliderRelax->range(0, dur);
	_sliderEnd->range(0, dur);

	_sliderStart->value(curMotion->synch_points.get_time(srSynchPoints::START));
	_sliderReady->value(curMotion->synch_points.get_time(srSynchPoints::READY));
	_sliderStrokeStart->value(curMotion->synch_points.get_time(srSynchPoints::STROKE_START));
	_sliderStroke->value(curMotion->synch_points.get_time(srSynchPoints::STROKE));
	_sliderStrokeEnd->value(curMotion->synch_points.get_time(srSynchPoints::STROKE_STOP));
	_sliderRelax->value(curMotion->synch_points.get_time(srSynchPoints::RELAX));
	_sliderEnd->value(curMotion->synch_points.get_time(srSynchPoints::STOP));
	redraw();
}

void MotionEditorWindow::updateMotionSyncPoints(const std::string& type)
{
	SmartBody::SBMotion* curMotion = getCurrentMotion();
	if (!curMotion)
		return;
	curMotion->validateSyncPoint(type);
	
}

void MotionEditorWindow::OnSliderSyncPoints(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	SmartBody::SBMotion* curMotion = editor->getCurrentMotion();
	if (!curMotion)	return;

	const std::string& type = widget->label();
	if (type == "start")
	{
		double playTime = editor->_sliderStart->value();
		curMotion->synch_points.set_time(srSynchPoints::START, playTime);
	}
	if (type == "ready")
	{
		double playTime = editor->_sliderReady->value();
		curMotion->synch_points.set_time(srSynchPoints::READY, playTime);
	}
	if (type == "stroke_start")
	{
		double playTime = editor->_sliderStrokeStart->value();
		curMotion->synch_points.set_time(srSynchPoints::STROKE_START, playTime);
	}
	if (type == "stroke")
	{
		double playTime = editor->_sliderStroke->value();
		curMotion->synch_points.set_time(srSynchPoints::STROKE, playTime);
	}
	if (type == "stroke_stop")
	{
		double playTime = editor->_sliderStrokeEnd->value();
		curMotion->synch_points.set_time(srSynchPoints::STROKE_STOP, playTime);
	}
	if (type == "relax")
	{
		double playTime = editor->_sliderRelax->value();
		curMotion->synch_points.set_time(srSynchPoints::RELAX, playTime);
	}
	if (type == "stop")
	{
		double playTime = editor->_sliderEnd->value();
		curMotion->synch_points.set_time(srSynchPoints::STOP, playTime);
	}
	editor->updateMotionSyncPoints(type);
	editor->updateSyncPointsUI();
}

void MotionEditorWindow::OnButtonGetSyncPoints(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	SmartBody::SBMotion* curMotion = editor->getCurrentMotion();
	if (!curMotion)	return;
	const std::string& type = widget->tooltip();
	double playTime = editor->_sliderMotionFrame->value();
	if (type == "start")
	{
		curMotion->synch_points.set_time(srSynchPoints::START, playTime);
	}
	if (type == "ready")
	{
		curMotion->synch_points.set_time(srSynchPoints::READY, playTime);
	}
	if (type == "stroke_start")
	{
		curMotion->synch_points.set_time(srSynchPoints::STROKE_START, playTime);
	}
	if (type == "stroke")
	{
		curMotion->synch_points.set_time(srSynchPoints::STROKE, playTime);
	}
	if (type == "stroke_stop")
	{
		curMotion->synch_points.set_time(srSynchPoints::STROKE_STOP, playTime);
	}
	if (type == "relax")
	{
		curMotion->synch_points.set_time(srSynchPoints::RELAX, playTime);
	}
	if (type == "stop")
	{
		curMotion->synch_points.set_time(srSynchPoints::STOP, playTime);
	}
	editor->updateMotionSyncPoints(type);
	editor->updateSyncPointsUI();
}

void MotionEditorWindow::updateMetaDataUI()
{
	SmartBody::SBMotion* curMotion = getCurrentMotion();
	if (!curMotion)
	{
		_browserMetaNames->clear();
		_browserMetaValues->clear();
		_inputMetaNames->value("");
		_inputMetaValues->value("");
		return;
	}
	_browserMetaNames->clear();
	_browserMetaNames->deselect();
	_browserMetaValues->clear();
	_browserMetaValues->deselect();
	const std::vector<std::string>& metaDataTags = curMotion->getMetaDataTags();
	for (size_t i = 0; i < metaDataTags.size(); ++i)
	{
		const std::string& metaDataString = curMotion->getMetaDataString(metaDataTags[i]);
		_browserMetaNames->add(metaDataTags[i].c_str());
		_browserMetaValues->add(metaDataString.c_str());
	}
	redraw();
}

void MotionEditorWindow::addMotionMetaData(const std::string& name, const std::string& value)
{
	SmartBody::SBMotion* curMotion = getCurrentMotion();
	if (!curMotion)
		return;

	curMotion->addMetaData(name, value);
}

void MotionEditorWindow::OnBrowserMetaNames(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	editor->_browserMetaValues->deselect();
	if (editor->_browserMetaNames->value() > 0)
		editor->_browserMetaValues->value(editor->_browserMetaNames->value());
}

void MotionEditorWindow::OnBrowserMetaValues(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	editor->_browserMetaNames->deselect();
	if (editor->_browserMetaValues->value() > 0)
		editor->_browserMetaNames->value(editor->_browserMetaValues->value());
}

void MotionEditorWindow::OnButtonAddMetaEntry(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	const std::string& name = editor->_inputMetaNames->value();
	const std::string& value = editor->_inputMetaValues->value();
	if (name != "" && value != "")
		editor->addMotionMetaData(name, value);
	editor->updateMetaDataUI();
}

void MotionEditorWindow::OnButtonDeleteMetaEntry(Fl_Widget* widget, void* data)
{
	MotionEditorWindow* editor = (MotionEditorWindow*) data;
	if (editor->_browserMetaNames->value() <= 0)
		return;
	SmartBody::SBMotion* curMotion = editor->getCurrentMotion();
	if (!curMotion)
		return;
	const std::string& name = editor->_browserMetaNames->text(editor->_browserMetaNames->value());
	curMotion->removeMetaData(name);
	editor->updateMetaDataUI();
}
