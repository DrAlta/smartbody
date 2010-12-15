
#include "fltk/Slider.h"  // before vhcl.h because of LOG enum which conflicts with vhcl::Log
#include "vhcl.h"

#include "PanimationWindow.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <sbm/mcontrol_util.h>
#include <sbm/bml.hpp>
#include "ParamAnimBlock.h"


PanimationWindow::PanimationWindow(int x, int y, int w, int h, char* name) : Window(w, h, name), BMLViewer(x, y, w, h)
{
	this->begin();
	// first group: animation name and character name
	Group* firstGroup = new fltk::Group(10, 20, w - 20, h/2 - 20, "Animations");
	firstGroup->begin();
		// left part
		anim1 = new fltk::Input(100, 20, 200, 20, "Animation1 ");
		anim1->when(fltk::WHEN_ENTER_KEY);
		anim1->callback(ChangeAnimation1, this);
		anim2 = new fltk::Input(100, 50, 200, 20, "Animation2 ");
		anim2->when(fltk::WHEN_ENTER_KEY);
		anim2->callback(ChangeAnimation2, this);
		character = new fltk::Input(100, 80, 200, 20, "Character ");
		character->callback(ChangeCharacter, this);
		blendingSlider = new fltk::ValueSlider(50, 110, 250, 20, "Blending Weight");
		blendingSlider->minimum(0.0);
		blendingSlider->maximum(1.0);
		blendingSlider->callback(weightChanged, this);
		runAnimation = new fltk::Button(150, 150, 100, 30, "RUN");
		runAnimation->callback(generateBML, this);

		// right part
		Group* firstInnerGroup = new fltk::Group(w/2, 20, w/2 - 30, h/2 - 50, "BML Info");
		firstInnerGroup->begin();
			textXML = new fltk::TextDisplay(0, 0, w/2 - 30, h/2 - 50);
			textXML->color(fltk::WHITE);
			textXML->textcolor(BLACK);
			bufferXML = new fltk::TextBuffer();
			textXML->buffer(bufferXML);
			textXML->wrap_mode(true, 0);
		firstInnerGroup->end();
		firstInnerGroup->resizable(textXML);
	firstGroup->end();
	firstGroup->box(fltk::BORDER_FRAME);
	firstGroup->box(fltk::BORDER_BOX);
	firstGroup->label("Animation and Character Setting");

	// second group: animation correspondence marks
	Group* secondGroup = new fltk::Group(10, h/2 + 20, w - 20, h/2 - 20, "Animations");
		widgetParamAnim = new ParamAnimEditorWidget(10, h/2 + 80, w - 20, h/2 - 80, "Paramaterized Animation");
		addCorrespondenceMark = new fltk::Button(20, h/2 + 30, 100, 20, "Add Mark");
		addCorrespondenceMark->callback(addCorrespondenceMarkCb, this);
		delCorrespondenceMark = new fltk::Button(130, h/2 + 30, 100, 20, "Delete Mark");
		delCorrespondenceMark->callback(delCorrespondenceMarkCb, this);
		updateCorrespondence = new fltk::Button(240, h/2 + 30, 150, 20, "Update Correspondence");
		updateCorrespondence->callback(updateCorrespondenceCb, this);
	secondGroup->end();
	secondGroup->resizable(widgetParamAnim);
	secondGroup->box(fltk::BORDER_FRAME);
	secondGroup->box(fltk::BORDER_BOX);
	secondGroup->label("Correspondence Mark Editor");
	this->resizable(firstGroup);
	this->resizable(secondGroup);
	this->x(x);
	this->y(y);


	nleModel = new nle::NonLinearEditorModel();
	widgetParamAnim->setModel(nleModel);

	// For this paramterized animation engine, there would be only two track for now
	ParamAnimTrack* track1 = new ParamAnimTrack();
	track1->setName("Animation1");
	nleModel->addTrack(track1);

	ParamAnimTrack* track2 = new ParamAnimTrack();
	track2->setName("Animation2");
	nleModel->addTrack(track2);

	redraw();
}


PanimationWindow::~PanimationWindow()
{
}

void PanimationWindow::draw()
{
    nle::NonLinearEditorModel* model = widgetParamAnim->getModel();
    if (model)
		updateGUI();
    
    Window::draw();   
}

void PanimationWindow::updateGUI()
{
}

void PanimationWindow::label_viewer(std::string name)
{
	this->label(strdup(name.c_str()));
}

void PanimationWindow::show_bml_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	this->show();
}

void PanimationWindow::hide_bml_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(NULL, NULL);
	this->hide();
}

void PanimationWindow::show()
{    
    Window::show();   
}

void PanimationWindow::generateBML(fltk::Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;

	std::string anim1 = window->anim1->text();
	std::string anim2 = window->anim2->text();
	std::string character = window->character->text();
	float blendingWeight = (float)window->blendingSlider->value();

	// generate animation
	std::ostringstream cmd;
	cmd << "test bml char " << character << " <sbm:panimation anim1=\"" << anim1 << "\" anim2=\"" << anim2 << "\" sbm:value=\"" << blendingWeight << "\" loop=\"true\"/>";

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.panim_weight = blendingWeight;
	mcu.is_fixed_weight = false;
	BML::SbmCommand* command = new BML::SbmCommand(cmd.str(), (float)mcu.time);
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
		{
			std::stringstream strstr;
			strstr << "ERROR: PanimationWindow::generateBML: Failed to execute sequence.";
			LOG(strstr.str().c_str());
			window->textXML->buffer()->replace(0, window->textXML->buffer()->length(), strstr.str().c_str());
		}
		else
			window->textXML->buffer()->replace(0, window->textXML->buffer()->length(), cmd.str().c_str());
		window->textXML->relayout();
		window->textXML->redraw();
	}
}

void PanimationWindow::weightChanged(fltk::Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.panim_weight = (float)window->blendingSlider->value();
}

void PanimationWindow::ChangeAnimation1(fltk::Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	nle::Track* track = window->nleModel->getTrack(0);
	track->removeAllBlocks();

	const char* motionName = window->anim1->value();
	if (motionName == NULL)
	{
		nle::Track* track = window->nleModel->getTrack(0);
		window->updateGUI();
		window->redraw();
		return;
	}

	std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(motionName);
	if (iter != mcu.motion_map.end())
	{
		SkMotion* motion = (*iter).second;
		ParamAnimBlock* block1 = new ParamAnimBlock();
		block1->setName(motionName);
		block1->setStartTime(0);
		block1->setEndTime(motion->duration());
		nle::Track* track = window->nleModel->getTrack(0);
		track->addBlock(block1);

		window->updateCorrespondenceMarks();

		window->updateGUI();
		window->redraw();
		return;
	}
	else
	{
		nle::Track* track = window->nleModel->getTrack(0);
		window->updateGUI();
		window->redraw();
		return;
	}
}

void PanimationWindow::ChangeAnimation2(fltk::Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	nle::Track* track = window->nleModel->getTrack(1);
	track->removeAllBlocks();

	const char* motionName = window->anim2->value();
	if (motionName == NULL)
	{
		nle::Track* track = window->nleModel->getTrack(1);
		window->updateGUI();
		window->redraw();
		return;
	}

	std::map<std::string, SkMotion*>::iterator iter = mcu.motion_map.find(motionName);
	if (iter != mcu.motion_map.end())
	{
		SkMotion* motion = (*iter).second;
		ParamAnimBlock* block1 = new ParamAnimBlock();
		block1->setName(motionName);
		block1->setStartTime(0);
		block1->setEndTime(motion->duration());
		nle::Track* track = window->nleModel->getTrack(1);
		track->addBlock(block1);

		window->updateCorrespondenceMarks();

		window->updateGUI();
		window->redraw();
		return;
	}
	else
	{
		nle::Track* track = window->nleModel->getTrack(1);
		window->updateGUI();
		window->redraw();
		return;
	}
}


void PanimationWindow::addCorrespondenceMarkCb(fltk::Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	nle::Track* track1 = window->nleModel->getTrack(0);
	nle::Block* block1 = NULL;
	if (track1->getNumBlocks() > 0)
		block1 = window->nleModel->getTrack(0)->getBlock(0);
	nle::Track* track2 = window->nleModel->getTrack(1);
	nle::Block* block2 = NULL;
	if (track2->getNumBlocks() > 0)
		block2 = window->nleModel->getTrack(1)->getBlock(0);
	CorrespondenceMark* toAddMark1 = new CorrespondenceMark();
	toAddMark1->setStartTime(0.0);
	toAddMark1->setEndTime(toAddMark1->getStartTime());
	toAddMark1->setColor(fltk::RED);
	char buff[256];
	sprintf(buff, "%6.2f", toAddMark1->getStartTime());
	toAddMark1->setName(buff);
	toAddMark1->setShowName(true);
	if (block1)
		block1->addMark(toAddMark1);

	CorrespondenceMark* toAddMark2 = new CorrespondenceMark();
	toAddMark2->setStartTime(0.0);
	toAddMark2->setEndTime(toAddMark2->getStartTime());
	toAddMark2->setColor(fltk::RED);
	sprintf(buff, "%6.2f", toAddMark2->getStartTime());
	toAddMark2->setName(buff);
	toAddMark2->setShowName(true);
	if (block2)
		block2->addMark(toAddMark2);

	window->updateGUI();
	window->redraw();
}

void PanimationWindow::delCorrespondenceMarkCb(fltk::Widget* widget, void* data)
{
	CorrespondenceMark* attachedMark = NULL;
	PanimationWindow* window = (PanimationWindow*) data;
	for (int t = 0; t < window->nleModel->getNumTracks(); t++)
	{
		nle::Track* track = window->nleModel->getTrack(t);
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
	window->updateGUI();
	window->redraw();
}

void PanimationWindow::updateCorrespondenceCb(fltk::Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	nle::Track* track1 = window->nleModel->getTrack(0);
	nle::Block* block1 = NULL;
	if (track1->getNumBlocks() > 0)
		block1 = window->nleModel->getTrack(0)->getBlock(0);
	nle::Track* track2 = window->nleModel->getTrack(1);
	nle::Block* block2 = NULL;
	if (track2->getNumBlocks() > 0)
		block2 = window->nleModel->getTrack(1)->getBlock(0);


	std::map<std::string, SkMotion*>::iterator iter;
	std::map<std::string, std::vector<double>>::iterator iter1;
	if (block1)
	{
		std::string animName1 = window->anim1->value();
		iter = mcu.motion_map.find(animName1);
		iter1 = mcu.panim_key_map.find(animName1);
		if (iter != mcu.motion_map.end())
		{
			std::vector<double> key1;
			for (int m1 = 0; m1 < block1->getNumMarks(); m1++)
			{
				nle::Mark* mark1 = block1->getMark(m1);
				key1.push_back(mark1->getStartTime());
			}
			sort(key1.begin(), key1.end());
			if (iter1 != mcu.panim_key_map.end())
				iter1->second = key1;
			else
				mcu.panim_key_map.insert(std::make_pair(animName1, key1));
		}
	}

	if (block2)
	{
		std::string animName2 = window->anim2->value();
		iter = mcu.motion_map.find(animName2);
		iter1 = mcu.panim_key_map.find(animName2);
		if (iter != mcu.motion_map.end())
		{
			std::vector<double> key2;
			for (int m2 = 0; m2 < block2->getNumMarks(); m2++)
			{
				nle::Mark* mark2 = block2->getMark(m2);
				key2.push_back(mark2->getStartTime());
			}
			sort(key2.begin(), key2.end());
			if (iter1 != mcu.panim_key_map.end())
				iter1->second = key2;
			else
				mcu.panim_key_map.insert(std::make_pair(animName2, key2));
		}
	}
	// connect the correspondence points
	window->updateCorrespondenceMarks();
	window->updateGUI();
	window->redraw();
}

void PanimationWindow::updateCorrespondenceMarks()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<CorrespondenceMark*> marks1;
	std::vector<CorrespondenceMark*> marks2;

	nle::Track* track1 = this->nleModel->getTrack(0);
	if (track1->getNumBlocks() > 0)
	{
		nle::Block* block1 = track1->getBlock(0);
		block1->removeAllMarks();
		std::string animName1 = this->anim1->value();
		std::map<std::string, std::vector<double>>::iterator iter1 = mcu.panim_key_map.find(animName1);

		if (iter1 != mcu.panim_key_map.end())
		{
			std::vector<double>& keys = (*iter1).second;
			for (size_t k = 0; k < keys.size(); k++)
			{
				CorrespondenceMark* mark = new CorrespondenceMark();
				mark->setStartTime(keys[k]);
				mark->setEndTime(mark->getStartTime());
				mark->setColor(fltk::RED);
				char buff[256];
				sprintf(buff, "%6.2f", mark->getStartTime());
				mark->setName(buff);
				mark->setShowName(true);
				block1->addMark(mark);
				marks1.push_back(mark);
			}
		}
	}

	nle::Track* track2 = this->nleModel->getTrack(1);
	if (track2->getNumBlocks() > 0)
	{
		nle::Block* block2 = track2->getBlock(0);
		block2->removeAllMarks();
		std::string animName2 = this->anim2->value();
		std::map<std::string, std::vector<double>>::iterator iter2 = mcu.panim_key_map.find(animName2);\
		if (iter2 != mcu.panim_key_map.end())
		{
			std::vector<double>& keys = (*iter2).second;
			for (size_t k = 0; k < keys.size(); k++)
			{
				CorrespondenceMark* mark = new CorrespondenceMark();
				mark->setStartTime(keys[k]);
				mark->setEndTime(mark->getStartTime());
				mark->setColor(fltk::RED);
				char buff[256];
				sprintf(buff, "%6.2f", mark->getStartTime());
				mark->setName(buff);
				mark->setShowName(true);
				block2->addMark(mark);
				marks2.push_back(mark);
			}
		}
	}

	// connect the correspondence points
	if (marks1.size() == marks2.size())
	{
		for (size_t m = 0; m < marks1.size(); m++)
		{
			marks1[m]->attach(marks2[m]);
			marks2[m]->attach(marks1[m]);
		}
	}

}

void PanimationWindow::ChangeCharacter(fltk::Widget* widget, void* data)
{
}


PanimationViewerFactory::PanimationViewerFactory()
{
}

BMLViewer* PanimationViewerFactory::create(int x, int y, int w, int h)
{
	PanimationWindow* panimationWindow = new PanimationWindow(x, y, w, h, "Parameterized Animation");
	return panimationWindow;
}

void PanimationViewerFactory::destroy(BMLViewer* viewer)
{
	delete viewer;
}


