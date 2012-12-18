#ifndef _JOINTMAPVIEWER_
#define _JOINTMAPVIEWER_

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>
#include <string>

class JointMapViewer : public Fl_Double_Window
{
	public:
		JointMapViewer(int x, int y, int w, int h, char* name);
		~JointMapViewer();
		
		static void ApplyMapCB(Fl_Widget* widget, void* data);
		static void CancelCB(Fl_Widget* widget, void* data);
		static void SelectMapCB(Fl_Widget* widget, void* data);

		void applyJointMap();
		void updateSelectMap();
		void updateUI();
		

	protected:
		std::string _jointMapName;
		std::string _skelName;
		std::vector<std::string> standardJointNames;

		Fl_Choice* _choiceJointMaps;
		Fl_Choice* _choiceCharacters;
		Fl_Scroll* _scrollGroup;

};
#endif
