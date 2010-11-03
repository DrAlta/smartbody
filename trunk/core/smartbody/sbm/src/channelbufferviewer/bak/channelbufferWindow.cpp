#include "channelbufferwindow.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <vhcl_log.h>
#include <sbm/mcontrol_util.h>
#include <sbm/bml.hpp>
#include <algorithm>

ChannelBufferWindow::ChannelBufferWindow(int x, int y, int w, int h, char* name) : Window(w, h, name), BMLViewer(x, y, w, h)
{
	this->begin();
	// first group: animation name and character name
	Group* firstGroup = new fltk::Group(10, 20, w - 20, h/2 - 20, "");
	firstGroup->begin();
		// left part
		
		character = new fltk::Choice(50, 20, w/4, 20, "Character");
		loadCharacters(character);
		character->callback(refreshChannels, this);

		refresh = new fltk::Button(50+w/4, 20, w/16, 20, "Refresh");
		refresh->callback(refreshCharacters, this);

		channel_list = new fltk::Browser(50+w/4+w/16, 20, w/4, 120, "Channels");
		channel_list->type(fltk::Browser::MULTI);
		loadChannels(character, channel_list);

		channel_monitor = new fltk::Browser(50+w/2+w/8+20, 20, w/4, 120, "Monitored Channels");
		channel_monitor->type(fltk::Browser::MULTI);
		loadChannels(character, channel_monitor);

		channel_add = new fltk::Button(50+w/2+w/16+10, 20, w/16, 20, ">>>");
		channel_add->callback(refreshChannels, this);

		channel_remove = new fltk::Button(50+w/2+w/16+10, 50, w/16, 20, "<<<");
		channel_add->callback(refreshChannels, this);
		//character->when
		//chart = new fltk::GlutWindow(10, 20, 100, 200, "chart");
		//chart->when(fltk::WHEN_ENTER_KEY);
		
	firstGroup->end();
	this->resizable(firstGroup);


	Group* secondGroup = new fltk::Group(10, h/4+30, w - 20, h/2 - 40, "");
	secondGroup->begin();
		chartview = new GlChartView(0, 0, w-20, 3*h/4-40, "chart");
	secondGroup->end();
	secondGroup->resizable(chartview);
	this->resizable(secondGroup);
	//redraw();
}


ChannelBufferWindow::~ChannelBufferWindow()
{
}

void ChannelBufferWindow::loadCharacters(fltk::Choice* character)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	character->clear();
	SbmCharacter* actor = NULL;
	mcu.character_map.reset();
	for(int i = 0; i < mcu.character_map.get_num_entries(); ++i)
	{
		actor = mcu.character_map.next();
		character->add(actor->name);
	}

}

void ChannelBufferWindow::loadChannels(fltk::Choice* character, fltk::Browser* channel_list)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	channel_list->clear();

	if(character->get_item()== NULL) return;

	SbmCharacter* actor = mcu.character_map.lookup(character->get_item()->label());

	SkSkeleton* skeleton = actor->skeleton_p;

	SkChannelArray& channels = skeleton->channels();
    int numChannels = channels.size();
	SkJoint* joint = NULL;
	char str[100];
    for (int i = 0; i < numChannels; i++)
    {
		joint = channels.joint(i);
		if(joint == NULL) continue;
        
		SkChannel& channel = channels[i];
        int channelSize = channel.size();
		sprintf(str, "%s (%d)", joint->name().get_string(), channelSize);
		channel_list->add(str);
	}

}

void ChannelBufferWindow::refreshCharacters(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	loadCharacters(window->character);
	//loadChannels(window->character, window->channel_list);
}

void ChannelBufferWindow::refreshChannels(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	loadChannels(window->character, window->channel_list);
}

void ChannelBufferWindow::draw()
{
    Window::draw();   
}

void ChannelBufferWindow::updateGUI()
{
}

void ChannelBufferWindow::label_viewer(std::string name)
{
	this->label(strdup(name.c_str()));
}

void ChannelBufferWindow::show_bml_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	this->show();
}

void ChannelBufferWindow::hide_bml_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(NULL, NULL);
	this->hide();
}

void ChannelBufferWindow::show()
{    
    Window::show();   
}


ChannelBufferViewerFactory::ChannelBufferViewerFactory()
{
}

BMLViewer* ChannelBufferViewerFactory::create(int x, int y, int w, int h)
{
	ChannelBufferWindow* channelbufferWindow = new ChannelBufferWindow(x, y, w, h, "Channel Buffer");
	return channelbufferWindow;
}

void ChannelBufferViewerFactory::destroy(BMLViewer* viewer)
{
	delete viewer;
}


