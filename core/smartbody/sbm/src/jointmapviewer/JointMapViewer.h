#ifndef _JOINTMAPVIEWER_
#define _JOINTMAPVIEWER_

#include <FL/Fl_Check_Button.H>
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

class MouseViewer : public Fl_Gl_Window
{
public:
	MouseViewer(int x, int y, int w, int h, char* name);
	~MouseViewer();

public:
	virtual int handle( int event );
	void translate_event(SrEvent& e, SrEvent::EventType t, int w, int h, MouseViewer* viewer);
	void mouse_event(SrEvent& e);	
protected:
	SrVec rotate_point(SrVec point, SrVec origin, SrVec direction, float angle);
protected:
	SrEvent e;
	SrCamera cam;
};

class SkeletonViewer : public MouseViewer
{
public:
	SkeletonViewer(int x, int y, int w, int h, char* name);
	~SkeletonViewer();

public:
	void setShowJointLabels(int showLabel);
	void setSkeleton(std::string skelName);
	void setJointMap(std::string mapName);
	void setFocusJointName(std::string focusName);
	std::string getFocusJointName();
	void focusOnSkeleton();
	virtual int handle( int event );
	virtual void draw();
	
protected:
	void updateLights();
	void init_opengl();
	void drawJointMapLabels(std::string jointMapName);
	std::vector<int> process_hit(unsigned int *pickbuffer,int nhits);
	std::string pickJointName(float x, float y);
	
protected:	
	SrSaGlRender renderFunction;
	int showJointLabels;
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
		
		static void ResetMapCB(Fl_Widget* widget, void* data);
		static void ApplyMapCB(Fl_Widget* widget, void* data);
		static void CancelCB(Fl_Widget* widget, void* data);
		static void SelectMapCB(Fl_Widget* widget, void* data);
		static void SelectCharacterCB(Fl_Widget* widget, void* data);
		static void JointNameChange(Fl_Widget* widget, void* data);
		static void AddJointMapCB(Fl_Widget* widget, void* data);
		static void CheckShowJointLabelCB(Fl_Widget* widget, void* data);

		void resetJointMap(bool restore = false);
		void addFocusJointMap();
		void applyJointMap();
		void updateSelectMap();
		void updateCharacter();
		void updateUI();
		void updateJointName(Fl_Input_Choice* jointChoice);		
		void hideButtons();
		void showJointLabels(int showLabel);
		virtual void draw();
	protected:
		void updateJointLists();

	
		std::string _jointMapName;
		std::string _skelName;
		std::string _charName;
		std::vector<std::string> standardJointNames;
		std::vector<std::string> skeletonJointNames;
	public:
		Fl_Choice* _buttonJointLabel;
		Fl_Choice* _choiceJointMaps;
		Fl_Choice* _choiceCharacters;
		Fl_Scroll* _scrollGroup;
		Fl_Button* _buttonApply;
		Fl_Button* _buttonCancel;
		Fl_Button* _buttonReset;
		Fl_Button* _buttonRestore;
		Fl_Button* _buttonAddMapping;
		std::vector<JointMapInputChoice*> _jointChoiceList;
		SkeletonViewer* targetSkeletonViewer;
		SkeletonViewer* standardSkeletonViewer;
		int scrollY;
};
#endif
