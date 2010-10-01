#ifndef _PANIMATION_WINDOW_H_
#define _PANIMATION_WINDOW_H_

#include <fltk/Window.h>
#include <fltk/Choice.h>
#include <fltk/LightButton.h>
#include <fltk/Group.h>
#include <fltk/Input.h>
#include <fltk/ValueSlider.h>
#include <fltk/TextDisplay.h>
#include <fltk/MultiLineOutput.h>
#include <sbm/BMLViewer.h>
#include "ParamAnimEditorWidget.h"

class PanimationWindow : public fltk::Window, public BMLViewer
{
	public:
		PanimationWindow(int x, int y, int w, int h, char* name);
		~PanimationWindow();

		virtual void label_viewer(std::string name);
		virtual void show_bml_viewer();
		virtual void hide_bml_viewer();
		void draw();
		void updateGUI();
		void updateCorrespondenceMarks();


        void show();  
		static void generateBML(fltk::Widget* widget, void* data);
		static void weightChanged(fltk::Widget* widget, void* data);
		static void ChangeAnimation1(fltk::Widget* widget, void* data);
		static void ChangeAnimation2(fltk::Widget* widget, void* data);
		static void ChangeCharacter(fltk::Widget* widget, void* data);
		static void addCorrespondenceMarkCb(fltk::Widget* widget, void* data);
		static void delCorrespondenceMarkCb(fltk::Widget* widget, void* data);
		static void updateCorrespondenceCb(fltk::Widget* widget, void* data);
			
	public:
		fltk::Input* anim1;
		fltk::Input* anim2;
		fltk::Input* character;
		fltk::ValueSlider* blendingSlider;
		fltk::Button* runAnimation;
		fltk::Button* addCorrespondenceMark;
		fltk::Button* delCorrespondenceMark;
		fltk::Button* updateCorrespondence;

		fltk::TextDisplay* textXML;
		fltk::TextBuffer* bufferXML;

		ParamAnimEditorWidget* widgetParamAnim;
		nle::NonLinearEditorModel* nleModel;
};

 class PanimationViewerFactory : public BMLViewerFactory
 {
	public:
		PanimationViewerFactory();

		virtual BMLViewer* create(int x, int y, int w, int h);
		virtual void destroy(BMLViewer* viewer);
 };
#endif