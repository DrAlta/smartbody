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
	char value[10];
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

		controller = new fltk::Choice(60, 50, w/4, 20, "Controller");
		loadControllers(controller, character);
		controller->callback(refreshControllerChannels, this);

		channel_filter = new fltk::Input(50+w/4+w/16+50, 0, w/4-50, 18, "Channels:");
		channel_filter->when(fltk::WHEN_CHANGED);
		channel_filter->callback(FilterChannelItem, this);

		channel_list = new fltk::Browser(50+w/4+w/16, 20, w/4, 120, "");
		channel_list->type(fltk::Browser::MULTI);
		loadChannels(this);

		channel_monitored_filter = new fltk::Input(50+w/2+w/8+20+50, 0, w/4-50, 18, "Monitored:");
		channel_monitored_filter->when(fltk::WHEN_CHANGED);
		channel_monitored_filter->callback(FilterMonitoredChannelItem, this);

		channel_monitor = new fltk::Browser(50+w/2+w/8+20, 20, w/4, 120, "");
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
		sprintf(value, "%d", num_of_frames);
		frame_num->when(fltk::WHEN_ENTER_KEY);
		frame_num->callback(refreshMaxSize, this);
		frame_num->value(value);

		quat = new fltk::Choice(300, 0, (int)(1.5f*w/8), 18, "Rotation shown as:");
		initQuat();
		quat->callback(refreshQuat, this);

		freeze = new fltk::Button(w-200, 0, 80, 18, "Freeze");
		freeze->callback(freezeView, this);
		is_freezed = false;

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

void ChannelBufferWindow::FilterChannelItem(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	FilterItem(window->channel_list, window->channel_filter);
}

void ChannelBufferWindow::FilterMonitoredChannelItem(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	FilterItem(window->channel_monitor, window->channel_monitored_filter);
}

void ChannelBufferWindow::FilterItem(fltk::Browser* list, fltk::Input* filter)
{
	const char* keyword = filter->value();
	if(keyword[0] == '\0')
	{
		for(int i = 0; i < list->size(); ++i)
		{
			list->goto_index(i)->show();
		}
		return;
	}
	for(int i = 0; i < list->size(); ++i)
	{
		const char* item = list->goto_index(i)->label();
		if(strstr(item, keyword)!= NULL) 
		{
			list->goto_index(i)->show();
		}
		else 
		{
			list->goto_index(i)->hide();
		}
	}
	//list->redraw();
}

void ChannelBufferWindow::resetCamera(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	int i = -1;
	if(strcmp(window->quat->get_item()->label(), "Quaternion") == 0) i = 0;
	else if(strcmp(window->quat->get_item()->label(), "Euler angle") == 0) i = 1;
	window->chartview->init_camera(i);
	window->chartview->update_coordinate = true;
}

void ChannelBufferWindow::freezeView(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->is_freezed = !window->is_freezed;
	if(window->is_freezed) window->freeze->label("Unfreeze");
	else window->freeze->label("Freeze");

}

void ChannelBufferWindow::refreshQuat(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	int i = 0;

	if(strcmp(window->quat->get_item()->label(), "Quaternion") == 0) i = 0;
	else if(strcmp(window->quat->get_item()->label(), "Euler angle") == 0) i = 1;
	
	window->chartview->set_quat_show_type(i);
}

void ChannelBufferWindow::initQuat()
{
	quat->add("Quaternion");
	quat->add("Euler angle");
}

void ChannelBufferWindow::refreshMaxSize(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->num_of_frames = atoi(window->frame_num->value());
	//window->chartview->set_max_buffer_size(max_buffer_size);
	int series_count = window->chartview->get_archive()->GetSeriesCount();
	for(int i = 0; i < series_count; ++i)
	{
		window->chartview->get_archive()->GetSeries(i)->SetMaxSize(window->num_of_frames);
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
	int ind = 0;
	character->set_item(&ind, 0);
}
void ChannelBufferWindow::loadControllers(fltk::Choice* controller, fltk::Choice* character)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	controller->clear();

	if(character->get_item()== NULL) return;

	SbmCharacter* actor = mcu.character_map.lookup(character->get_item()->label());

	controller->add("All controllers");

	int ct_num = actor->ct_tree_p->count_controllers();
	for(int i = 0; i < ct_num; ++i)
	{
		//ct = actor->ct_tree_p->controller(i)->name();
		controller->add(actor->ct_tree_p->controller(i)->name());
		actor->ct_tree_p->controller(i)->record_buffer_changes(true);
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

void ChannelBufferWindow::refreshControllers(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	loadControllers(window->controller, window->character);
	//loadChannels(window->character, window->channel_list);
}

void ChannelBufferWindow::refreshChannels(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	loadChannels(window);
	clearMonitoredChannel(window);
}

void ChannelBufferWindow::refreshControllerChannels(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	if(window->character->get_item() == NULL) return;
	fltk::Browser* channels = window->channel_list;

	if(strcmp(window->controller->get_item()->label(), "All controllers") == 0)
	{
		for(int i = 0; i < channels->size(); ++i)
		{
			channels->goto_index(i)->show();
		}
		return;
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* actor = mcu.character_map.lookup(window->character->get_item()->label());
	
	for(int i = 0; i < channels->size(); ++i)
	{
		channels->goto_index(i)->hide();
	}

	int ct_num = actor->ct_tree_p->count_controllers();
	for(int i = 0; i < ct_num; ++i)
	{
		if(strcmp(actor->ct_tree_p->controller(i)->name(), window->controller->get_item()->label())== 0)
		{
			std::vector<float> buff = actor->ct_tree_p->controller(i)->get_buffer_changes();
			SkChannelArray& channelsInUse = actor->ct_tree_p->controller(i)->controller_channels();
			for(int j = 0; j < channelsInUse.size(); ++j)
			{
				int index = actor->ct_tree_p->controller(i)->getContextChannel(j);
				channels->goto_index(index)->show();
			}
		}
	}
}

void ChannelBufferWindow::addMonitoredChannel(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	moveChannels(window, window->channel_list, window->channel_monitor, true, window->buffer_index);
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
	moveChannels(window, window->channel_monitor, window->channel_list, false, window->buffer_index);
}

void ChannelBufferWindow::moveChannels(ChannelBufferWindow* cbufwindow, fltk::Browser* from, fltk::Browser* to, bool add_series, SrArray<int>& buffer_index)
{
	for(int i = 0; i < from->size(); ++i)
	{
		if(from->selected(i))
		{
			const char* title = from->goto_index(i)->label();
			to->add(title);
			if(add_series) 
			{
				cbufwindow->chartview->get_archive()->NewSeries(title, get_size(title), buffer_index.get(i));
				cbufwindow->chartview->get_archive()->GetLastSeries()->SetMaxSize(cbufwindow->num_of_frames);
				buffer_index.remove(i);
			}
			else 
			{
				GlChartViewSeries* series = cbufwindow->chartview->get_archive()->GetSeries(title);
				if(series == NULL)
				{
					printf("\nERROR: ChannelBufferWindow::moveChannels(). series not found");
				}
				else buffer_index.push() = series->GetBufferIndex();
				cbufwindow->chartview->get_archive()->DeleteSeries(title);
			}
			from->remove(i);
			--i;
		}
	}
}

/*void ChannelBufferWindow::moveChannels(GlChartView* chartview, fltk::Browser* from, fltk::Browser* to, bool add_series, SrArray<int>& buffer_index)
{
	for(int i = 0; i < from->size(); ++i)
	{
		if(from->selected(i))
		{
			const char* title = from->goto_index(i)->label();
			
			if(add_series) 
			{
				chartview->get_archive()->NewSeries(title, get_size(title), buffer_index.get(i));
				chartview->get_archive()->GetLastSeries()->SetMaxSize(chartview->max_buffer_size);
				from->goto_index(i)->hide();
				to->add(title);
				//buffer_index.remove(i);
			}
			else 
			{
				GlChartViewSeries* series = chartview->get_archive()->GetSeries(title);
				if(series == NULL)
				{
					printf("\nERROR: ChannelBufferWindow::moveChannels(). series not found");
				}
				//else buffer_index.push() = series->GetBufferIndex();
				chartview->get_archive()->DeleteSeries(title);
				for(int j = 0; j < to->size(); ++j)
				{
					const char* str = to->goto_index(j)->label();
					if(strcmp(str, title) == 0) to->goto_index(j)->set_visible();
				}
				from->remove(i);
				--i;
			}
		}
	}
	from->redraw();
	to->redraw();
}*/

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


