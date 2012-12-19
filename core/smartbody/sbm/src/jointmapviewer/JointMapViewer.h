#ifndef _JOINTMAPVIEWER_
#define _JOINTMAPVIEWER_

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Button.H>
#include <string>

class JointMapViewer : public Fl_Double_Window
{
	public:
		JointMapViewer(int x, int y, int w, int h, char* name);
		~JointMapViewer();

		void setCharacterName(std::string charName);
		void setJointMapName(std::string jointMapName);
		
		static void ApplyMapCB(Fl_Widget* widget, void* data);
		static void CancelCB(Fl_Widget* widget, void* data);
		static void SelectMapCB(Fl_Widget* widget, void* data);
		static void SelectCharacterCB(Fl_Widget* widget, void* data);
		static void JointNameChange(Fl_Widget* widget, void* data);

		void applyJointMap();
		void updateSelectMap();
		void updateCharacter();
		void updateUI();
		void updateJointName(Fl_Input_Choice* jointChoice);		
		void hideButtons();

	protected:
		std::string _jointMapName;
		std::string _skelName;
		std::string _charName;
		std::vector<std::string> standardJointNames;
		std::vector<std::string> skeletonJointNames;

		Fl_Choice* _choiceJointMaps;
		Fl_Choice* _choiceCharacters;
		Fl_Scroll* _scrollGroup;
		Fl_Button* _buttonApply;
		Fl_Button* _buttonCancel;
		std::vector<Fl_Input_Choice*> _jointChoiceList;
};
#endif
