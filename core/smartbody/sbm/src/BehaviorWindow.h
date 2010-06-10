#ifndef _BEHAVIORWINDOW_
#define _BEHAVIORWINDOW_

#include <fltk/Window.h>
#include <fltk/FloatInput.h>
#include <fltk/TextDisplay.h>
#include <fltk/Choice.h>
#include <fltk/LightButton.h>
#include "NonLinearEditor.h"
#include "NonLinearEditorWidget.h"
#include "BehaviorEditorWidget.h"
#include "BehaviorBlock.h"
#include <sbm/bml_speech.hpp>
#include <sbm/bml_event.hpp>
#include <sbm/BMLViewer.h>

class BehaviorWindow : public fltk::Window, public BMLViewer
{
	public:
		BehaviorWindow(int x, int y, int w, int h, char* name);
		~BehaviorWindow();

		static void OnRequest(BML::BmlRequest* request, void* data);

		virtual void label_viewer(std::string name);
		virtual void show_bml_viewer();
		virtual void hide_bml_viewer();
		
		int handle(int event);
        void show();      
        void draw();
        
		void updateGUI();
        
		EditorWidget* getEditorWidget();
		Block* getSelectedBlock();
        Track* getSelectedTrack();
		void updateBehaviors(BML::BmlRequest* request);

		void processMotionRequest(BML::MotionRequest* motionRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
								  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
								  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
		void processControllerRequest(BML::MeControllerRequest* contrlllerRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
									  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
									  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
		void processSpeechRequest(BML::SpeechRequest* speechRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
									double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
									std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
		void processEventRequest(BML::EventRequest* eventRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
									double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
									std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
	
		void adjustSyncPoints(BML::BehaviorRequest* behavior, Block* block, std::map<std::string, double>& syncMap);
		
		static void ContextCB(fltk::Widget* widget, void* data);
		static void ClearCB(fltk::Widget* widget, void* data);

		BehaviorEditorWidget* nleWidget;


		fltk::Choice* choiceContexts;
		fltk::Button* buttonClear;
		fltk::TextDisplay* textXML;
		fltk::TextBuffer* bufferXML;

		int contextCounter;
		std::string selectedContext;

		NonLinearEditorModel* nleModel; 
};

 class BehaviorViewerFactory : public BMLViewerFactory
 {
	public:
		BehaviorViewerFactory();

		virtual BMLViewer* create(int x, int y, int w, int h);
		virtual void destroy(BMLViewer* viewer);
 };
#endif
