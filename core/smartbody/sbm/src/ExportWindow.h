#ifndef _EXPORTWINDOW_H_
#define _EXPORTWINDOW_H_

#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Double_Window.H>
#include <vector>

class ExportWindow : public Fl_Double_Window
{
	public:
		ExportWindow(int x, int y, int w, int h, char* name);
		~ExportWindow();

		static void ExportCB(Fl_Widget* widget, void* data);
		static void FileCB(Fl_Widget* widget, void* data);

		std::vector<Fl_Check_Button*> checkExport;
		Fl_Input* inputFile;
		Fl_Button* buttonFile;
		Fl_Button* buttonExport;

};
#endif