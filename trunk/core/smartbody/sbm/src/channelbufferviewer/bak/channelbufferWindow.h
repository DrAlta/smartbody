#ifndef _CHANNEL_BUFFER_WINDOW_H_
#define _CHANNEL_BUFFER_WINDOW_H_

#include <fltk/Window.h>
#include <fltk/GlWindow.H>
#include <fltk/Choice.h>
#include <fltk/Browser.h>
#include <fltk/Button.h>
#include <fltk/LightButton.h>
#include <fltk/Group.h>
#include <fltk/Input.h>
#include <fltk/ValueSlider.h>
#include <fltk/TextDisplay.h>
#include <fltk/MultiLineOutput.h>
#include <sbm/BMLViewer.h>

#include "GlChartView.h"


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
		fltk::GlWindow* chartview;
		fltk::Choice* character;
		fltk::Button* refresh;
		fltk::Browser* channel_list;
		fltk::Browser* channel_monitor;
		fltk::Button* channel_add;
		fltk::Button* channel_remove;

	public:
		static void loadCharacters(fltk::Choice* character);
		static void loadChannels(fltk::Choice* character, fltk::Browser* channel_list);
		static void refreshCharacters(fltk::Widget* widget, void* data);
		static void refreshChannels(fltk::Widget* widget, void* data);


};

 class ChannelBufferViewerFactory : public BMLViewerFactory
 {
	public:
		ChannelBufferViewerFactory();

		virtual BMLViewer* create(int x, int y, int w, int h);
		virtual void destroy(BMLViewer* viewer);
 };
#endif