/*
 *  channelbufferWindow.hpp - part of SmartBody-lib's Test Suite
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

#ifndef _CHANNEL_BUFFER_WINDOW_H_
#define _CHANNEL_BUFFER_WINDOW_H_

#include <fltk/Window.h>
#include <fltk/Choice.h>
#include <fltk/Browser.h>
#include <fltk/Button.h>
#include <fltk/CheckButton.h>
#include <fltk/LightButton.h>
#include <fltk/Group.h>
#include <fltk/Input.h>

#include <sbm/GenericViewer.h>

#include <sbm/mcontrol_util.h>
#include "GlChartView.hpp"

class ChannelItem
{
public:
	int index;
	SrString* name;
	SrString* label;
	SkChannel::Type type;
	bool monitored;
	bool channel_filtered;
	bool motion_filtered;
	bool not_in_search;
};


class ChannelBufferWindow : public GenericViewer, public fltk::Window 
{
public:
	ChannelBufferWindow(int x, int y, int w, int h, char* name);
	~ChannelBufferWindow();

	virtual void label_viewer(std::string name);
	virtual void show_viewer();
	virtual void hide_viewer();
	void draw();
	int handle(int event) { return Window::handle(event); }   
	void updateGUI();
	void show();  

	void generateBML(fltk::Widget* widget, void* data);

public:
	GlChartView* chartview;
	fltk::Choice* character;
	fltk::Choice* controller;
	fltk::Choice* motion;
	fltk::Choice* quat;
	fltk::CheckButton* show_x;
	fltk::CheckButton* show_y;
	fltk::CheckButton* show_z;
	fltk::CheckButton* show_w;
	fltk::CheckButton* check_hide_other_channels;
	fltk::Button* refresh;
	fltk::Button* freeze;
	fltk::Button* reset_camera;
	fltk::Browser* channel_list;
	fltk::Browser* channel_monitor;
	fltk::Button* channel_add;
	fltk::Button* channel_remove;
	fltk::Input* frame_num;
	fltk::Input* channel_filter;
	fltk::Input* channel_monitored_filter;

	SrArray<ChannelItem> Channel_item_list;

	int num_of_frames;
	bool is_freezed;
	bool hide_other_channels;

	int mode; //0: character; 1: controller; 2: motion

protected:
	SrString no_motion;

public:
	const char* getSelectedCharacterName();
	void update();

protected:
	static void clearChannelItem(ChannelBufferWindow* window);
	static void initChannelItem(ChannelBufferWindow* window, int num);
	static void loadCharacters(fltk::Choice* character);
	static void loadControllers(fltk::Choice* controller, fltk::Choice* character);
	static void loadChannels(ChannelBufferWindow* window);
	static void loadMotions(ChannelBufferWindow* window);

	static void refreshMotionChannels(fltk::Widget* widget, void* data);
	static void refreshChannelsWidget(ChannelBufferWindow* window);
	static void refreshMonitoredChannelsWidget(ChannelBufferWindow* window);
	static void refreshMaxSize(ChannelBufferWindow* window, int num);

	static void refreshBold(fltk::Widget* widget, void* data);
	static void refreshCharacters(fltk::Widget* widget, void* data);
	static void refreshControllers(fltk::Widget* widget, void* data);
	static void refreshControllerChannels(fltk::Widget* widget, void* data);
	static void refreshHideOtherChannels(fltk::Widget* widget, void* data);
	static void refreshChannels(fltk::Widget* widget, void* data);
	static void refreshMaxSize(fltk::Widget* widget, void* data);
	static void refreshQuat(fltk::Widget* widget, void* data);
	static void resetCamera(fltk::Widget* widget, void* data);
	static void freezeView(fltk::Widget* widget, void* data);
	static void FilterChannelItem(fltk::Widget* widget, void* data);
	static void FilterMonitoredChannelItem(fltk::Widget* widget, void* data);
	static void refreshControllerVisibilities(ChannelBufferWindow* window);

	static void refreshShowX(fltk::Widget* widget, void* data);
	static void refreshShowY(fltk::Widget* widget, void* data);
	static void refreshShowZ(fltk::Widget* widget, void* data);
	static void refreshShowW(fltk::Widget* widget, void* data);

	static void setXYZVisibility(ChannelBufferWindow* window);

protected:
	void initQuat();
	void set_default_values();
	static void clearMonitoredChannel(ChannelBufferWindow* window);
	static void addMonitoredChannel(fltk::Widget* widget, void* data);
	static void FilterItem(ChannelBufferWindow* window, fltk::Browser* list, fltk::Input* filter, bool monitored);
	static void removeMonitoredChannel(fltk::Widget* widget, void* data);
	static int get_size(const char* title);
	static void fillSeriesWithMotionData(ChannelBufferWindow* window, SkMotion* motion, GlChartViewSeries* series, ChannelItem& item);
};

class ChannelBufferViewerFactory : public GenericViewerFactory
{
public:
	ChannelBufferViewerFactory();

	virtual GenericViewer* create(int x, int y, int w, int h);
	virtual void destroy(GenericViewer* viewer);
};
#endif