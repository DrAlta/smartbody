#ifndef _JOINTMAPVIEWER_
#define _JOINTMAPVIEWER_

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Button.H>
#include <sr/sr_camera.h>
#include <sk/sk_scene.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_light.h>
#include <sb/SBSkeleton.h>
#include <string>

class SkeletonViewer : public Fl_Gl_Window
{
public:
	SkeletonViewer(int x, int y, int w, int h, char* name);
	~SkeletonViewer();

public:
	void setSkeleton(std::string skelName);
	void setJointMap(std::string mapName);
	void setFocusJointName(std::string focusName);
	void focusOnSkeleton();
	virtual void draw();
protected:
	void updateLights();
	void init_opengl();
	void drawJointMapLabels(std::string jointMapName);
protected:	
	SrSaGlRender renderFunction;
	SrCamera camera;
	std::vector<SrLight> lights;
	SkScene* skeletonScene;
	SmartBody::SBSkeleton* skeleton;
	std::string jointMapName;
	std::string focusJointName;
public:
	std::vector<std::string> standardJointNames;	
};

class JointMapInputChoice : public Fl_Input_Choice
{
protected:
	SkeletonViewer* skelViewer;
public:
	JointMapInputChoice(int x, int y, int w, int h, char* name);
	~JointMapInputChoice();
public:
	virtual int handle ( int event );
	void setViewer(SkeletonViewer* viewer);
};


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
		virtual void draw();

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
		SkeletonViewer* skeletonViewer;
};
#endif
