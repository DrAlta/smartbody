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

#include <sbm/BMLViewer.h>

#include <sbm/mcontrol_util.h>
#include "GlChartView.hpp"


class ChannelBufferWindow : public BMLViewer, public fltk::Window 
{
public:
	ChannelBufferWindow(int x, int y, int w, int h, char* name);
	~ChannelBufferWindow();

	virtual void label_viewer(std::string name);
	virtual void show_bml_viewer();
	virtual void hide_bml_viewer();
	void draw();
	void updateGUI();
	void updateCorrespondenceMarks();
	void show();  

	void generateBML(fltk::Widget* widget, void* data);

public:
	GlChartView* chartview;
	fltk::Choice* character;
	fltk::Choice* controller;
	fltk::Choice* quat;
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

	SrArray<int> buffer_index;
	int num_of_frames;
	bool is_freezed;

public:
	const char* getSelectedCharacterName();
	static void loadCharacters(fltk::Choice* character);
	static void loadControllers(fltk::Choice* controller, fltk::Choice* character);
	static void loadChannels(ChannelBufferWindow* window);
	static void refreshCharacters(fltk::Widget* widget, void* data);
	static void refreshControllers(fltk::Widget* widget, void* data);
	static void refreshControllerChannels(fltk::Widget* widget, void* data);
	static void refreshChannels(fltk::Widget* widget, void* data);
	static void refreshMaxSize(fltk::Widget* widget, void* data);
	static void refreshQuat(fltk::Widget* widget, void* data);
	static void resetCamera(fltk::Widget* widget, void* data);
	static void freezeView(fltk::Widget* widget, void* data);
	static void FilterChannelItem(fltk::Widget* widget, void* data);
	static void FilterMonitoredChannelItem(fltk::Widget* widget, void* data);

protected:
	void initQuat();
	void set_default_values();
	static void clearMonitoredChannel(ChannelBufferWindow* window);
	static void addMonitoredChannel(fltk::Widget* widget, void* data);
	static void FilterItem(fltk::Browser* list, fltk::Input* filter);
	static void removeMonitoredChannel(fltk::Widget* widget, void* data);
	static void moveChannels(ChannelBufferWindow* cbufwindow, fltk::Browser* from, fltk::Browser* to, bool add_series, SrArray<int>& buffer_index);
	static int get_size(const char* title);
};

class ChannelBufferViewerFactory : public BMLViewerFactory
{
public:
	ChannelBufferViewerFactory();

	virtual BMLViewer* create(int x, int y, int w, int h);
	virtual void destroy(BMLViewer* viewer);
};
#endif