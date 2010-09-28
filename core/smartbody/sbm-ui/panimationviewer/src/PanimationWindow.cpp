#include "PanimationWindow.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <vhcl_log.h>
#include <sbm/mcontrol_util.h>
#include <sbm/bml.hpp>


PanimationWindow::PanimationWindow(int x, int y, int w, int h, char* name) : Window(w, h, name), BMLViewer(x, y, w, h)
{
	this->begin();
	Group* topGroup = new fltk::Group(0, 0, w, h, "Animations");
	topGroup->begin();
		anim1 = new fltk::Input(70, 0, 200, 20, "Animation1:");
		anim2 = new fltk::Input(70, 30, 200, 20, "Animation2:");
		character = new fltk::Input(70, 60, 200, 20, "Character:");
		blendingSlider = new fltk::ValueSlider(50, 90, 200, 20, "Blending Weight");
		blendingSlider->minimum(0.0);
		blendingSlider->maximum(1.0);
		blendingSlider->callback(weightChanged, this);
		runAnimation = new fltk::Button(120, 150, 100, 30, "RUN");
		runAnimation->callback(generateBML, this);
	topGroup->end();
	this->resizable(topGroup);
	this->x(x);
	this->y(y);
	redraw();
}


PanimationWindow::~PanimationWindow()
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
		}
	}
}

void PanimationWindow::weightChanged(fltk::Widget* widget, void* data)
{
	PanimationWindow* window = (PanimationWindow*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.panim_weight = (float)window->blendingSlider->value();
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


