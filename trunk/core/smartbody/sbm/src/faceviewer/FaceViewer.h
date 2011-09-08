#ifndef _FACEVIEWER_H_
#define _FACEVIEWER_H_

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <sbm/GenericViewer.h>

class FaceViewer : public GenericViewer, Fl_Double_Window
{
	public:
		FaceViewer(int x, int y, int w, int h, char* name);
		~FaceViewer();

		virtual void show_viewer();
		virtual void hide_viewer();

		static void CharacterCB(Fl_Widget* widget, void* data);
		static void RefreshCB(Fl_Widget* widget, void* data);
		static void ResetCB(Fl_Widget* widget, void* data);
		static void ShowCommandsCB(Fl_Widget* widget, void* data);
		static void FaceCB(Fl_Widget* widget, void* data);
		static void FaceWeightCB(Fl_Widget* widget, void* data);
		
		Fl_Choice* choiceCharacters;
		Fl_Button* buttonRefresh;
		Fl_Button* buttonReset;
		Fl_Group* topGroup;
		Fl_Scroll* bottomGroup;
		Fl_Button* buttonCommands;


};

class FaceViewerFactory : public GenericViewerFactory
{
	public:
		FaceViewerFactory();

		virtual GenericViewer* create(int x, int y, int w, int h);
		virtual void destroy(GenericViewer* viewer);
};


#endif