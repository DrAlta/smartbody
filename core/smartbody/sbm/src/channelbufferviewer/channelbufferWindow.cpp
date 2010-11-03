#include "channelbufferWindow.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <vhcl_log.h>
#include <sbm/bml.hpp>
#include <algorithm>

ChannelBufferWindow::ChannelBufferWindow(int x, int y, int w, int h, char* name) : Window(w, h, name), BMLViewer(x, y, w, h)
{
	set_default_values();
	this->begin();
	// first group: animation name and character name
	Group* firstGroup = new fltk::Group(10, 20, w - 20, h/2 - 20, "");
	firstGroup->begin();
	// left part

		character = new fltk::Choice(60, 20, w/8, 20, "Character");
		loadCharacters(character);
		character->callback(refreshChannels, this);

		refresh = new fltk::Button(60+w/8+5, 20, w/16, 20, "Refresh");
		refresh->callback(refreshCharacters, this);

		channel_list = new fltk::Browser(50+w/4+w/16, 20, w/4, 120, "Channels");
		channel_list->type(fltk::Browser::MULTI);
		loadChannels(this);

		channel_monitor = new fltk::Browser(50+w/2+w/8+20, 20, w/4, 120, "Monitored Channels");
		channel_monitor->type(fltk::Browser::MULTI);
		//loadChannels(character, channel_monitor);

		channel_add = new fltk::Button(50+w/2+w/16+10, 20, w/16, 20, ">>>");
		channel_add->callback(addMonitoredChannel, this);

		channel_remove = new fltk::Button(50+w/2+w/16+10, 50, w/16, 20, "<<<");
		channel_remove->callback(removeMonitoredChannel, this);

	firstGroup->end();
	this->resizable(firstGroup);


	Group* secondGroup = new fltk::Group(10, h/4+30, w - 20, h/2 - 40, "");
	secondGroup->begin();
		frame_num = new fltk::Input(120, 0, 40, 18, "No. of frames shown:");
		frame_num->when(fltk::WHEN_ENTER_KEY);
		frame_num->callback(refreshMaxSize, this);

		quat = new fltk::Choice(300, 0, (int)(1.5f*w/8), 18, "quaternions shown as:");
		initQuat();
		quat->callback(refreshQuat, this);

		reset_camera = new fltk::Button(w-100, 0, 80, 18, "Reset Camera");
		reset_camera->callback(resetCamera, this);

		chartview = new GlChartView(0, 20, w-20, 3*h/4-60, "");
	secondGroup->end();
	secondGroup->resizable(chartview);
	this->resizable(secondGroup);
	//redraw();
}

ChannelBufferWindow::~ChannelBufferWindow()
{
}

void ChannelBufferWindow::resetCamera(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->chartview->init_camera();
}

void ChannelBufferWindow::refreshQuat(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	int i = 0;

	if(strcmp(window->quat->get_item()->label(), "4 series of values") == 0) i = 0;
	else if(strcmp(window->quat->get_item()->label(), "Euler angle") == 0) i = 1;
	
	window->chartview->set_quat_show_type(i);
}

void ChannelBufferWindow::initQuat()
{
	quat->add("4 series of values");
	quat->add("Euler angle");
}

void ChannelBufferWindow::refreshMaxSize(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	int max_buffer_size = atoi(window->frame_num->value());
	window->chartview->set_max_buffer_size(max_buffer_size);
	int series_count = window->chartview->get_archive()->GetSeriesCount();
	for(int i = 0; i < series_count; ++i)
	{
		window->chartview->get_archive()->GetSeries(i)->SetMaxSize(max_buffer_size);
	}
}

void ChannelBufferWindow::set_default_values()
{
	num_of_frames = 800;
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

void ChannelBufferWindow::loadChannels(ChannelBufferWindow* window)
{
	fltk::Choice* character = window->character;
	fltk::Browser* channel_list = window->channel_list;
	window->buffer_index.size(0);
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	channel_list->clear();

	if(character->get_item()== NULL) return;

	SbmCharacter* actor = mcu.character_map.lookup(character->get_item()->label());

	SkSkeleton* skeleton = actor->skeleton_p;

	SkChannelArray& channels = skeleton->channels();
	int numChannels = channels.size();
	SkJoint* joint = NULL;
	char str[100];
	int channel_index = 0;
	char ext[3];
	int ext_count = 0;
	for (int i = 0; i < numChannels; i++)
	{
		joint = channels.joint(i);
		if(joint == NULL) continue;

		SkChannel& channel = channels[i];
		int channelSize = channel.size();
		if(!joint->pos()->frozen(0) && ext_count == 0) 
		{
			if(i < numChannels-1 && strcmp(channels.joint(i+1)->name().get_string(), joint->name().get_string()) == 0)
			{
				sprintf(ext, "_x");
				++ext_count;
			}
		}
		else if(!joint->pos()->frozen(1) && ext_count == 1) 
		{
			sprintf(ext, "_y");
			++ext_count;
		}
		else if(!joint->pos()->frozen(2) && ext_count == 2) 
		{
			sprintf(ext, "_z");
			++ext_count;
		}
		else 
		{
			ext[0] = '\0';
			ext_count = 0;
		}
		sprintf(str, "%s%s (%d)", joint->name().get_string(), ext, channelSize);
		channel_list->add(str);
		window->buffer_index.push() = channel_index;
		channel_index += channelSize;
	}

}

const char* ChannelBufferWindow::getSelectedCharacterName()
{
	fltk::Widget* item = character->get_item();
	if(item == NULL) return NULL;
	return character->get_item()->label();
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
	loadChannels(window);
	clearMonitoredChannel(window);
}

void ChannelBufferWindow::addMonitoredChannel(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	moveChannels(window->chartview, window->channel_list, window->channel_monitor, true, window->buffer_index);
}

void ChannelBufferWindow::clearMonitoredChannel(ChannelBufferWindow* window)
{
	fltk::Browser* monitoredchannel = window->channel_monitor;
	window->chartview->get_archive()->ClearSeries();
	monitoredchannel->clear();
}

void ChannelBufferWindow::removeMonitoredChannel(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	moveChannels(window->chartview, window->channel_monitor, window->channel_list, false, window->buffer_index);
}

void ChannelBufferWindow::moveChannels(GlChartView* chartview, fltk::Browser* from, fltk::Browser* to, bool add_series, SrArray<int>& buffer_index)
{
	for(int i = 0; i < from->size(); ++i)
	{
		if(from->selected(i))
		{
			const char* title = from->goto_index(i)->label();
			to->add(title);
			if(add_series) 
			{
				chartview->get_archive()->NewSeries(title, get_size(title), buffer_index.get(i));
				chartview->get_archive()->GetLastSeries()->SetMaxSize(chartview->max_buffer_size);
				buffer_index.remove(i);
			}
			else 
			{
				GlChartViewSeries* series = chartview->get_archive()->GetSeries(title);
				if(series == NULL)
				{
					printf("\nERROR: ChannelBufferWindow::moveChannels(). series not found");
				}
				else buffer_index.push() = series->GetBufferIndex();
				chartview->get_archive()->DeleteSeries(title);
			}
			from->remove(i);
			--i;
		}
	}
}

int ChannelBufferWindow::get_size(const char* title)
{
	int len = strlen(title);
	bool number = false;

	for(int i = len-1; i >= 0; --i)
	{
		if(number)
		{
			return title[i]-48;
		}
		if(title[i] == ')') number = true;
	}
	return -1;
}

void ChannelBufferWindow::draw()
{
	Window::draw();
	chartview->render();
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


