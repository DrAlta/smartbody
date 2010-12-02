/*
 *  channelbufferWindow.cpp - part of SmartBody-lib's Test Suite
 *  Copyright (C) 2009  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Jingqiao Fu, USC
 */

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

	Group* firstGroup = new fltk::Group(10, 20, w - 20, h/4 - 20, "");
	firstGroup->begin();
		character = new fltk::Choice(60, 20, w/8, 20, "Character");
		loadCharacters(character);
		character->callback(refreshChannels, this);

		refresh = new fltk::Button(60+w/8+5, 20, w/16, 20, "Refresh");
		refresh->callback(refreshCharacters, this);

		controller = new fltk::Choice(60, 50, w/4, 20, "Controller");
		loadControllers(controller, character);
		controller->callback(refreshControllerChannels, this);

		check_hide_other_channels = new fltk::CheckButton(60, 70, 18, 18, "Hide other channels");
		check_hide_other_channels->callback(refreshHideOtherChannels, this);
		check_hide_other_channels->state(true);
		check_hide_other_channels->deactivate();

		motion = new fltk::Choice(60, 100, w/4, 20, "Motion");
		loadMotions(motion, character);
		motion->callback(refreshMotionChannels, this);

		channel_filter = new fltk::Input(50+w/4+w/16+50, 0, w/4-50, 18, "Channels:");
		channel_filter->when(fltk::WHEN_CHANGED);
		channel_filter->callback(FilterChannelItem, this);

		channel_list = new fltk::Browser(50+w/4+w/16, 20, w/4, 120, "");
		channel_list->type(fltk::Browser::MULTI);
		loadChannels(this);
		refreshChannelsWidget(this);
		
		channel_monitored_filter = new fltk::Input(50+w/2+w/8+20+50, 0, w/4-50, 18, "Monitored:");
		channel_monitored_filter->when(fltk::WHEN_CHANGED);
		channel_monitored_filter->callback(FilterMonitoredChannelItem, this);

		channel_monitor = new fltk::Browser(50+w/2+w/8+20, 20, w/4, 120, "");
		channel_monitor->type(fltk::Browser::MULTI);
		channel_monitor->when(fltk::WHEN_CHANGED);
		channel_monitor->callback(refreshBold, this);
		refreshMonitoredChannelsWidget(this);

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
		quat->callback(refreshQuat, this);

		show_x = new fltk::CheckButton(300+(int)(1.5f*w/8), 0, 18, 18, "X");
		show_x->callback(refreshShowX, this);
		show_x->state(true);
		show_y = new fltk::CheckButton(300+(int)(1.5f*w/8)+30, 0, 18, 18, "Y");
		show_y->callback(refreshShowY, this);
		show_y->state(true);
		show_z = new fltk::CheckButton(300+(int)(1.5f*w/8)+60, 0, 18, 18, "Z");
		show_z->callback(refreshShowZ, this);
		show_z->state(true);
		show_w = new fltk::CheckButton(300+(int)(1.5f*w/8)+90, 0, 18, 18, "W");
		show_w->callback(refreshShowW, this);
		show_w->state(true);

		initQuat();

		freeze = new fltk::Button(w-200, 0, 80, 18, "Freeze");
		freeze->callback(freezeView, this);
		is_freezed = false;

		reset_camera = new fltk::Button(w-100, 0, 80, 18, "Reset Camera");
		reset_camera->callback(resetCamera, this);

		chartview = new GlChartView(0, 20, w-20, 3*h/4-60, "");
	secondGroup->end();
	secondGroup->resizable(chartview);
	this->resizable(secondGroup);
}

ChannelBufferWindow::~ChannelBufferWindow()
{
	clearChannelItem(this);
}

void ChannelBufferWindow::clearChannelItem(ChannelBufferWindow* window)
{
	for(int i = 0; i < window->Channel_item_list.size(); ++i)
	{
		delete window->Channel_item_list.get(i).label;
	}
	window->Channel_item_list.capacity(0);
}

void ChannelBufferWindow::initChannelItem(ChannelBufferWindow* window, int num)
{
	window->Channel_item_list.capacity(num);
	window->Channel_item_list.size(num);
	for(int i = 0; i < num; ++i)
	{
		window->Channel_item_list.get(i).label = new SrString();
		window->Channel_item_list.get(i).name = new SrString();
	}
}

void ChannelBufferWindow::refreshBold(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	for(int i = 0; i < window->chartview->get_archive()->GetSeriesCount(); ++i)
	{
		window->chartview->get_archive()->GetSeries(i)->bold = false;
	}
	for(int j = 0; j < window->channel_monitor->size(); ++j)
	{
		if(window->channel_monitor->selected(j))
		{
			const char* label = window->channel_monitor->goto_index(j)->label();
			for(int i = 0; i < window->Channel_item_list.size(); ++i)
			{
				ChannelItem& item = window->Channel_item_list.get(i);
				if(item.monitored)
				{
					if(strcmp(&(item.label->get(0)), label) == 0)
					{
						for(int k = 0; k < window->chartview->get_archive()->GetSeriesCount(); ++k)
						{
							if(strcmp(&(window->chartview->get_archive()->GetSeries(k)->title.get(0)), &(item.label->get(0)))==0)
							{
								window->chartview->get_archive()->GetSeries(k)->bold = true;
								break;
							}
						}
					}
				}
			}
		}
	}
}

void ChannelBufferWindow::FilterChannelItem(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	FilterItem(window, window->channel_list, window->channel_filter, false);
	refreshChannelsWidget(window);
}

void ChannelBufferWindow::FilterMonitoredChannelItem(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	FilterItem(window, window->channel_monitor, window->channel_monitored_filter, true);
	refreshMonitoredChannelsWidget(window);
}

void ChannelBufferWindow::FilterItem(ChannelBufferWindow* window, fltk::Browser* list, fltk::Input* filter, bool monitored)
{
	const char* keyword = filter->value();
	if(keyword[0] == '\0')
	{
		for(int i = 0; i < window->Channel_item_list.size(); ++i)
		{
			if(window->Channel_item_list.get(i).monitored == monitored)
			{
				window->Channel_item_list.get(i).not_in_search = false;
			}
		}
		return;
	}
	for(int i = 0; i < window->Channel_item_list.size(); ++i)
	{
		if(window->Channel_item_list.get(i).monitored != monitored) continue;
		const char* item = &(window->Channel_item_list.get(i).label->get(0));
		if(strstr(item, keyword)!= NULL) 
		{
			window->Channel_item_list.get(i).not_in_search = false;
		}
		else 
		{
			window->Channel_item_list.get(i).not_in_search = true;
		}
	}
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

void ChannelBufferWindow::refreshShowX(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->chartview->show_x = !window->chartview->show_x;
}

void ChannelBufferWindow::refreshShowY(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->chartview->show_y = !window->chartview->show_y;
}

void ChannelBufferWindow::refreshShowZ(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->chartview->show_z = !window->chartview->show_z;
}

void ChannelBufferWindow::refreshShowW(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->chartview->show_w = !window->chartview->show_w;
}

void ChannelBufferWindow::refreshQuat(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	int i = 0;

	if(strcmp(window->quat->get_item()->label(), "Quaternion") == 0) i = 0;
	else if(strcmp(window->quat->get_item()->label(), "Euler angle") == 0) i = 1;
	
	window->chartview->set_quat_show_type(i);
	setXYZVisibility(window);
}

void ChannelBufferWindow::initQuat()
{
	quat->add("Quaternion");
	quat->add("Euler angle");
	setXYZVisibility(this);
}

void ChannelBufferWindow::setXYZVisibility(ChannelBufferWindow* window)
{
	fltk::Color color;
	if(strcmp(window->quat->get_item()->label(), "Quaternion") == 0)
	{
		color = 0x38;
		window->show_x->activate();
		window->show_y->activate();
		window->show_z->activate();
		window->show_w->activate();
		window->show_x->textcolor(color);
		window->show_y->textcolor(color);
		window->show_z->textcolor(color);
		window->show_w->textcolor(color);
	}
	else if(strcmp(window->quat->get_item()->label(), "Euler angle") == 0)
	{
		color = 47;
		window->show_x->activate();
		window->show_y->activate();
		window->show_z->activate();
		window->show_w->deactivate();
		window->show_w->textcolor(color);
	}
}

void ChannelBufferWindow::refreshMaxSize(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	if(window->mode == 2) 
	{
		window->num_of_frames = atoi(window->frame_num->value());
		return;
	}
	//window->num_of_frames = atoi(window->frame_num->value());
	refreshMaxSize(window, atoi(window->frame_num->value()));
}

void ChannelBufferWindow::refreshMaxSize(ChannelBufferWindow* window, int num)
{
	window->num_of_frames = num;
	int series_count = window->chartview->get_archive()->GetSeriesCount();
	for(int i = 0; i < series_count; ++i)
	{
		window->chartview->get_archive()->GetSeries(i)->SetMaxSize(window->num_of_frames);
	}
	window->chartview->coordinate.SetXSize((float)window->num_of_frames);
}

void ChannelBufferWindow::set_default_values()
{
	num_of_frames = 800;
	mode = 1;
	hide_other_channels = true;
}

void ChannelBufferWindow::loadMotions(fltk::Choice* motion, fltk::Choice* character)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	motion->clear();
	//if(character->get_item()== NULL) return;
	motion->add("------");
	for (std::map<std::string, SkMotion*>::iterator it = mcu.motion_map.begin(); it != mcu.motion_map.end(); ++it)
	{
		motion->add((*it).first.c_str());
	}
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
		controller->add(actor->ct_tree_p->controller(i)->name());
		actor->ct_tree_p->controller(i)->record_buffer_changes(true);
	}
}

void ChannelBufferWindow::refreshChannelsWidget(ChannelBufferWindow* window)
{
	window->channel_list->clear();
	int num = window->Channel_item_list.size();
	fltk::Color color;
	color = 45;
	for(int i = 0; i < num; ++i)
	{
		ChannelItem& item = window->Channel_item_list.get(i);
		if(!item.monitored && !item.not_in_search && !item.motion_filtered)
		{
			if(window->mode == 2)
			{
				window->channel_list->add(&(item.label->get(0)));
			}
			else 
			{
				if(!item.channel_filtered)
				{
					window->channel_list->add(&(item.label->get(0)));
				}
				else if(!window->hide_other_channels)
				{
					window->channel_list->add(&(item.label->get(0)));
					window->channel_list->goto_index(window->channel_list->size()-1)->textcolor(color);
				}
			}
		}
	}
}

void ChannelBufferWindow::refreshMonitoredChannelsWidget(ChannelBufferWindow* window)
{
	fltk::Color color;
	color = 47;
	window->channel_monitor->clear();
	int num = window->Channel_item_list.size();
	for(int i = 0; i < num; ++i)
	{
		ChannelItem& item = window->Channel_item_list.get(i);
		if(item.monitored && !item.not_in_search)
		{
			window->channel_monitor->add(&(item.label->get(0)));
			if(item.motion_filtered) window->channel_monitor->goto_index(window->channel_monitor->size()-1)->textcolor(color);
		}
	}
}

void ChannelBufferWindow::loadChannels(ChannelBufferWindow* window)
{
	fltk::Choice* character = window->character;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if(character->get_item()== NULL) return;
	SbmCharacter* actor = mcu.character_map.lookup(character->get_item()->label());
	SkSkeleton* skeleton = actor->skeleton_p;

	SkChannelArray& channels = skeleton->channels();
	int numChannels = channels.size();

	clearChannelItem(window);
	initChannelItem(window, numChannels);

	SkJoint* joint = NULL;
	char str[100];
	int channel_index = 0;
	char ext[3];

	for (int i = 0; i < numChannels; i++)
	{
		joint = channels.joint(i);
		if(joint == NULL) continue;

		SkChannel& channel = channels[i];
		int channelSize = channel.size();
		if(channel.type == SkChannel::Type::XPos) sprintf(ext, "_x");
		else if(channel.type == SkChannel::Type::YPos) sprintf(ext, "_y");
		else if(channel.type == SkChannel::Type::ZPos) sprintf(ext, "_z");
		else ext[0] = '\0';

		sprintf(str, "%s%s (%d)", joint->name().get_string(), ext, channelSize);
		ChannelItem& item = window->Channel_item_list.get(i);
		item.channel_filtered = false;
		item.motion_filtered = false;
		item.monitored = false;
		item.not_in_search = false;
		item.index = channel_index;
		item.label->set(str);
		item.name->set(joint->name().get_string());
		item.type = channel.type;
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
	loadControllers(window->controller, window->character);
	loadMotions(window->motion, window->character);
}

void ChannelBufferWindow::refreshControllers(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	loadControllers(window->controller, window->character);
}

void ChannelBufferWindow::refreshChannels(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	loadChannels(window);
	refreshChannelsWidget(window);
}

void ChannelBufferWindow::refreshMotionChannels(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	int j = 0;
	refreshControllerVisibilities(window);
	
	if(strcmp(window->motion->get_item()->label(), "------") == 0)
	{
		for(j = 0; j < window->Channel_item_list.size(); ++j)
		{
			ChannelItem& item = window->Channel_item_list.get(j);
			item.motion_filtered = false;
			if(item.monitored) 
			{
				window->chartview->get_archive()->GetSeries(&(item.label->get(0)))->Reset();
			}
		}
		window->mode = 1;
		refreshMaxSize(window, atoi(window->frame_num->value()));
		refreshChannelsWidget(window);
		return;
	}
	window->mode = 2;
	SbmCharacter* actor = mcu.character_map.lookup(window->character->get_item()->label());
	std::map<std::string, SkMotion*>::iterator motionIter = mcu.motion_map.find(window->motion->get_item()->label());
	if (motionIter != mcu.motion_map.end())
	{
		SkMotion* motion = (*motionIter).second;
		motion->connect(actor->skeleton_p);
		refreshMaxSize(window, motion->frames());
		SkChannelArray& channels = motion->channels();
		for(int i = 0; i < window->Channel_item_list.size(); ++i)
		{
			window->Channel_item_list.get(i).motion_filtered = true;
		}
		for(int i = 0; i < channels.size(); ++i)
		{
			if(channels.joint(i) == NULL) continue;
			const char* name = channels.joint(i)->name().get_string();
			SkChannel::Type type = channels.get(i).type;
			for(j = 0; j < window->Channel_item_list.size(); ++j)
			{
				if(strcmp(&(window->Channel_item_list.get(j).name->get(0)), name) == 0
					&& window->Channel_item_list.get(j).type == type)
				{
					window->Channel_item_list.get(j).motion_filtered = false;
					if(window->Channel_item_list.get(j).monitored) fillSeriesWithMotionData(window, motion, NULL, window->Channel_item_list.get(j));
					break;
				}
			}
		}
	}
	refreshChannelsWidget(window);
}

void ChannelBufferWindow::refreshHideOtherChannels(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	window->hide_other_channels = !window->hide_other_channels;
	refreshChannelsWidget(window);
}

void ChannelBufferWindow::refreshControllerVisibilities(ChannelBufferWindow* window)
{
	if(strcmp(window->motion->get_item()->label(), "------") == 0)
	{
		window->controller->activate();
	}
	else 
	{
		window->controller->deactivate();
		window->check_hide_other_channels->deactivate();
		return;
	}
	if(strcmp(window->controller->get_item()->label(), "All controllers") == 0)
	{
		window->check_hide_other_channels->deactivate();
	}
	else window->check_hide_other_channels->activate();
}

void ChannelBufferWindow::refreshControllerChannels(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	if(window->character->get_item() == NULL) return;
	fltk::Browser* channels = window->channel_list;

	refreshControllerVisibilities(window);
	if(strcmp(window->controller->get_item()->label(), "All controllers") == 0)
	{
		for(int i = 0; i < window->Channel_item_list.size(); ++i)
		{
			window->Channel_item_list.get(i).channel_filtered = false;
		}
		//window->check_hide_other_channels->deactivate();
		refreshChannelsWidget(window);
		return;
	}
	//window->check_hide_other_channels->activate();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* actor = mcu.character_map.lookup(window->character->get_item()->label());
	
	for(int i = 0; i < window->Channel_item_list.size(); ++i)
	{
		window->Channel_item_list.get(i).channel_filtered = true;
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
				window->Channel_item_list.get(index).channel_filtered = false;
			}
		}
	}
	refreshChannelsWidget(window);
}

void ChannelBufferWindow::addMonitoredChannel(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	GlChartViewSeries* series = NULL;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkMotion*>::iterator motionIter;
	SkMotion* motion = NULL;
	if(window->mode == 2)
	{
		SbmCharacter* actor = mcu.character_map.lookup(window->character->get_item()->label());
		motionIter = mcu.motion_map.find(window->motion->get_item()->label());
		if (motionIter != mcu.motion_map.end())
		{
			motion = (*motionIter).second;
			motion->connect(actor->skeleton_p);
		}
	}

	for(int i = 0; i < window->channel_list->size(); ++i)
	{
		if(window->channel_list->selected(i))
		{
			for(int j = 0; j < window->Channel_item_list.size(); ++j)
			{
				if(strcmp(&(window->Channel_item_list.get(j).label->get(0)), window->channel_list->goto_index(i)->label()) == 0)
				{
					ChannelItem& item = window->Channel_item_list.get(j);
					item.monitored = true;
					const char* label = &(item.label->get(0));
					window->chartview->get_archive()->NewSeries(label, get_size(label), window->Channel_item_list.get(j).index);
					series = window->chartview->get_archive()->GetLastSeries();
					series->SetMaxSize(window->num_of_frames);
					if(window->mode == 2)
					{
						fillSeriesWithMotionData(window, motion, series, item);
					}
					break;
				}
			}
		}
	}
	refreshChannelsWidget(window);
	refreshMonitoredChannelsWidget(window);
}

void ChannelBufferWindow::fillSeriesWithMotionData(ChannelBufferWindow* window, SkMotion* motion, GlChartViewSeries* series, ChannelItem& item)
{
	if(motion == NULL || motion->connected_skeleton() == NULL) return;
	if(series == NULL)
	{
		series = window->chartview->get_archive()->GetSeries(&(item.label->get(0)));
	}
	float val[4];
	int index = 0;
	SkChannelArray& channels = motion->channels();
	for(index = 0; index < channels.size(); ++index)
	{
		if(channels[index].joint == NULL) continue;
		const char* name = channels[index].joint->name().get_string();
		if(strcmp(name, &(item.name->get(0))) == 0 && item.type == channels[index].type)
		{
			break;
		}
	}
	if(index == channels.size()) 
	{
		series->Clear();
		return;
	}
	for(int k = 0; k < window->num_of_frames; ++k)
	{
		motion->apply_frame(k);
		channels[index].get(val);
		if(series->data_type == 1)
		{
			series->Push(val[0]);
		}
		else if(series->data_type == 3)
		{
			series->Push(val[0], val[1], val[2]);
		}
		else if(series->data_type == 4)
		{
			series->Push(val[0], val[1], val[2], val[3]);
		}
		
	}
}

void ChannelBufferWindow::removeMonitoredChannel(fltk::Widget* widget, void* data)
{
	ChannelBufferWindow* window = (ChannelBufferWindow*) data;
	for(int i = 0; i < window->channel_monitor->size(); ++i)
	{
		if(window->channel_monitor->selected(i))
		{
			for(int j = 0; j < window->Channel_item_list.size(); ++j)
			{
				if(strcmp(&(window->Channel_item_list.get(j).label->get(0)), window->channel_monitor->goto_index(i)->label()) == 0)
				{
					ChannelItem& item = window->Channel_item_list.get(j);
					const char* label = &(item.label->get(0));
					item.monitored = false;
					window->chartview->get_archive()->DeleteSeries(label);
					break;
				}
			}
		}
	}
	refreshChannelsWidget(window);
	refreshMonitoredChannelsWidget(window);
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

void ChannelBufferWindow::update()
{
	if(!is_freezed)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SbmPawn* pawn_p = NULL;
		//SbmCharacter* char_p = NULL;
		SbmCharacter* actor = mcu.character_map.lookup(character->get_item()->label());
		mcu.pawn_map.reset();
		while(pawn_p = mcu.pawn_map.next())
		{
			const char* name = getSelectedCharacterName();
			if( name != NULL && strcmp(pawn_p->name, name) == 0) break;
		}
		//char_p = mcu.character_map.lookup( pawn_p->name );
		if(pawn_p != NULL)
		{
			if(mode != 2)
			{
				SrBuffer<float>& buffer = pawn_p->ct_tree_p->getLastFrame().buffer();
				chartview->get_archive()->Update(buffer);
			}
			if(mode == 1)
			{
				int buff_counter = 0;
				int ct_num = actor->ct_tree_p->count_controllers();
				for(int i = 0; i < ct_num; ++i)
				{
					if(strcmp(actor->ct_tree_p->controller(i)->name(), controller->get_item()->label())== 0)
					{
						std::vector<float> buff = actor->ct_tree_p->controller(i)->get_buffer_changes();
						SkChannelArray& channelsInUse = actor->ct_tree_p->controller(i)->controller_channels();
						for(int j = 0; j < channelsInUse.size(); ++j)
						{
							int index = actor->ct_tree_p->controller(i)->getContextChannel(j);
							if(Channel_item_list.get(index).monitored)
							{
								if(Channel_item_list.get(index).type == SkChannel::Type::XPos
								|| Channel_item_list.get(index).type == SkChannel::Type::YPos
								|| Channel_item_list.get(index).type == SkChannel::Type::ZPos)
								{
									chartview->get_archive()->GetSeries(&(Channel_item_list.get(index).label->get(0)))->SetLast(buff[buff_counter]);
								}
								else if(Channel_item_list.get(index).type == SkChannel::Type::Quat) 
								{
									chartview->get_archive()->GetSeries(&(Channel_item_list.get(index).label->get(0)))->SetLast(buff[buff_counter], buff[buff_counter+1], buff[buff_counter+2], buff[buff_counter+3]);
								}
							}
							if(Channel_item_list.get(index).type == SkChannel::Type::XPos
							|| Channel_item_list.get(index).type == SkChannel::Type::YPos
							|| Channel_item_list.get(index).type == SkChannel::Type::ZPos)
							{
								++buff_counter;
							}
							else if(Channel_item_list.get(index).type == SkChannel::Type::Quat) 
							{	
								buff_counter+= 4;
							}
						}
						break;
					}
				}
			}
		}
	}
	chartview->render();
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


