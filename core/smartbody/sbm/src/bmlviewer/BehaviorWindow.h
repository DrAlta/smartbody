#ifndef _BEHAVIORWINDOW_
#define _BEHAVIORWINDOW_

#include <fltk/Window.h>
#include <fltk/FloatInput.h>
#include <fltk/TextDisplay.h>
#include <fltk/Choice.h>
#include <fltk/LightButton.h>
#include "nle/NonLinearEditor.h"
#include "nle/NonLinearEditorWidget.h"
#include "BehaviorEditorWidget.h"
#include "BehaviorBlock.h"
#include <sbm/bml_speech.hpp>
#include <sbm/bml_event.hpp>
#include <sbm/GenericViewer.h>

class BehaviorWindow : public fltk::Window, public GenericViewer
{
	public:
		BehaviorWindow(int x, int y, int w, int h, char* name);
		~BehaviorWindow();

		static void OnRequest(BML::BmlRequest* request, void* data);

		virtual void label_viewer(std::string name);
		virtual void show_viewer();
		virtual void hide_viewer();
		virtual	void update_viewer();
		int handle(int event);
        void show();      
        void draw();
        
		void updateGUI();
        
		nle::EditorWidget* getEditorWidget();
		nle::Block* getSelectedBlock();
        nle::Track* getSelectedTrack();
		void updateBehaviors(BML::BmlRequest* request);

		void processMotionRequest(BML::MotionRequest* motionRequest, nle::NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
								  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
								  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
		void processControllerRequest(BML::MeControllerRequest* contrlllerRequest, nle::NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
									  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
									  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
		void processSpeechRequest(BML::SpeechRequest* speechRequest, nle::NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
									double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
									std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
		void processEventRequest(BML::EventRequest* eventRequest, nle::NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
									double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
									std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
		void processVisemeRequest(BML::VisemeRequest* eventRequest, nle::NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
									double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
									std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks);
	
		void adjustSyncPoints(BML::BehaviorRequest* behavior, nle::Block* block, std::map<std::string, double>& syncMap);
		
		static void ContextCB(fltk::Widget* widget, void* data);
		static void ClearCB(fltk::Widget* widget, void* data);
		static void ReplayCB(fltk::Widget* widget, void* data);

		BehaviorEditorWidget* nleWidget;


		fltk::Choice* choiceContexts;
		fltk::Button* buttonClear;
		fltk::Button* buttonReplay;
		fltk::TextDisplay* textXML;
		fltk::TextBuffer* bufferXML;

		int contextCounter;
		std::string selectedContext;

		nle::NonLinearEditorModel* nleModel;
};

 class BehaviorViewerFactory : public GenericViewerFactory
 {
	public:
		BehaviorViewerFactory();

		virtual GenericViewer* create(int x, int y, int w, int h);
		virtual void destroy(GenericViewer* viewer);
 };
#endif
