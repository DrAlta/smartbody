#include "BehaviorWindow.h"
#include <fltk/Group.h>
#include <fltk/ScrollGroup.h>
#include <iostream>
#include <vector>
#include <sbm/mcontrol_util.h>
#include <me/me_ct_blend.hpp>
#include <me/me_ct_time_shift_warp.hpp>
#include "BehaviorBlock.h"
#include <sbm/bml_types.hpp>
#include <sbm/text_speech.h>
#include <sbm/remote_speech.h>
#include <sbm/behavior_scheduler_fixed.hpp>
#include <math.h>

using namespace fltk;

BehaviorWindow::BehaviorWindow(int x, int y, int w, int h, char* name) : Window(w, h, name), BMLViewer(x, y, w, h)
{
	this->begin();

	Group* topGroup = new Group(10, 0, w - 10, 30, "Behaviors");
	topGroup->begin();

		choiceContexts = new fltk::Choice(100, 0, 200, 20, "Contexts");
		choiceContexts->callback(ContextCB, this);
		buttonClear = new fltk::Button(350, 0, 100, 20, "Clear");
		buttonClear->callback(ClearCB, this);
		Group* topSizer = new Group(470, 0, 10, 0);
		
	topGroup->end();
	topGroup->resizable(topSizer);
 
	Group* bottomGroup = new Group(0, 30, w - 10, h - 40);
	bottomGroup->begin();

		ScrollGroup* leftGroup = new ScrollGroup(0, 0, w - 210, h - 30);
		leftGroup->type(ScrollGroup::VERTICAL);
		leftGroup->begin();

			nleWidget = new BehaviorEditorWidget(0, 0, w - 230, h - 40, "");
			nleWidget->box(fltk::BORDER_BOX);

		leftGroup->end();
		leftGroup->resizable(nleWidget);

		Group* rightGroup = new Group(w - 210, 0, 200, h - 40, "Behavior Info");
		rightGroup->box(fltk::BORDER_BOX);
		rightGroup->begin();
			textXML = new fltk::TextDisplay(10, 10, 180, h - 50);
			textXML->color(fltk::WHITE);
			textXML->textcolor(BLACK);
			bufferXML = new fltk::TextBuffer();
			textXML->buffer(bufferXML);
			textXML->wrap_mode(true, 0);

            Group* rightSizer = new Group(80, 110, 90, 20);
      
		rightGroup->end();
		rightGroup->resizable(rightSizer);

	bottomGroup->end();
	bottomGroup->resizable(leftGroup);

	this->end();

	this->resizable(bottomGroup);

	this->x(x);
	this->y(y);

	contextCounter = 0;
	selectedContext = "";


	nleModel = new NonLinearEditorModel();
	EditorWidget* behaviorEditorWidget = this->getEditorWidget();
	behaviorEditorWidget->setModel(nleModel);

	updateGUI();
}


BehaviorWindow::~BehaviorWindow()
{
	delete nleModel;
}

void BehaviorWindow::label_viewer(std::string name)
{
	this->label(strdup(name.c_str()));
}

void BehaviorWindow::show_bml_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(OnRequest, this);
	this->show();
}

void BehaviorWindow::hide_bml_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(NULL, NULL);
	this->hide();
}

int BehaviorWindow::handle(int event)
{
	return  Window::handle(event);
}

void BehaviorWindow::show()
{
    updateGUI();
    
    Window::show();   
}
      
void BehaviorWindow::draw()
{
    NonLinearEditorModel* model = nleWidget->getModel();
    if (model)
    {
        if (model->isModelChanged())
        {
            updateGUI();
            model->setModelChanged(false);
        }
    }
    
    Window::draw();   
}

EditorWidget* BehaviorWindow::getEditorWidget()
{
	return nleWidget;
}

void BehaviorWindow::updateGUI()
{
	NonLinearEditorModel* model = nleWidget->getModel();
	if (!model)
		return;

	choiceContexts->clear();

	int count = 1;
	choiceContexts->add("-------");
	std::vector<std::pair<std::string, std::vector<Track*> > >& contexts = model->getContexts();
	for (std::vector<std::pair<std::string, std::vector<Track*> > >::iterator iter = contexts.begin();
		iter != contexts.end(); 
		iter++)
	{
		std::string contextName = (*iter).first;
		choiceContexts->add(contextName.c_str());
		if (contextName == selectedContext)
			choiceContexts->value(count);
		count++;

	}
	if (selectedContext == "" || selectedContext == "-------")
	{
		choiceContexts->value(0);
	}

	std::string selectedTrackName = "";
	for (int t = 0; t < model->getNumTracks(); t++)
	{
		Track* track = model->getTrack(t);
		if (track->isSelected())
			selectedTrackName = track->getName();
	}

 
	bool found = false;
	// activate the inputs based on the selected block
    Track* track = this->getSelectedTrack();
    if (track)
    {
        std::string trackName = track->getName();
    }
    else
    {
    }
        
	textXML->buffer()->remove(0, textXML->buffer()->length());
	Block* block = this->getSelectedBlock();
	if (block && block->isSelected())
	{     
		std::string name = block->getName();
		double startTime = block->getStartTime();
		double endTime = block->getEndTime();		
		textXML->insert(block->getInfo().c_str());

	}
	else
	{
	}
 

}


Block* BehaviorWindow::getSelectedBlock()
{
	NonLinearEditorModel* model = nleWidget->getModel();
	if (!model)
		return NULL;

	// activate the inputs based on the selected block
	for (int t = 0; t < model->getNumTracks(); t++)
	{
		Track* track = model->getTrack(t);
		for (int b = 0; b < track->getNumBlocks(); b++)
		{
			Block* curBlock = track->getBlock(b);
			if (curBlock->isSelected())
			{
				return curBlock;
			}
		}
	}
	return NULL;
}

Track* BehaviorWindow::getSelectedTrack()
{
    NonLinearEditorModel* model = nleWidget->getModel();
    if (!model)
        return NULL;

    // activate the inputs based on the selected block
    for (int t = 0; t < model->getNumTracks(); t++)
    {
        Track* track = model->getTrack(t);
        if (track->isSelected())
            return track;
    }
    return NULL;
}

void BehaviorWindow::ClearCB(fltk::Widget* widget, void* data)
{
	BehaviorWindow* window = (BehaviorWindow*) data;
	window->nleWidget->getModel()->clearContexts();

	window->updateGUI();
	window->redraw();
}

void BehaviorWindow::ContextCB(fltk::Widget* widget, void* data)
{
	BehaviorWindow* window = (BehaviorWindow*) data;
	fltk::Choice* choice = (fltk::Choice*) widget;

	int val = choice->value();
	if (val < 0)
		return;

	if (val == 0)
	{
		window->nleWidget->getModel()->clear(true);
		window->updateGUI();
		window->selectedContext = "";
		return;
	}
	fltk::Widget* choiceWidget = choice->child(val);

	std::string contextName = choiceWidget->label();
	window->nleWidget->getModel()->setContext(contextName);

	// set the viewable time to the minimum and maximum block times
	double minTime = 9999999;
	double maxTime = -999999;
	for (int t = 0; t < window->nleWidget->getModel()->getNumTracks(); t++)
	{
		Track* track = window->nleWidget->getModel()->getTrack(t);
		for (int b = 0; b < track->getNumBlocks(); b++)
		{
			Block* block = track->getBlock(b);
			if (block->getStartTime() < minTime)
				minTime = block->getStartTime();
			if (block->getEndTime() > maxTime)
				maxTime = block->getEndTime();
		}
	}
	window->nleWidget->setViewableTimeStart(minTime);
	window->nleWidget->setViewableTimeEnd(maxTime);
	window->selectedContext = contextName;
	window->updateGUI();
}

void BehaviorWindow::updateBehaviors(BML::BmlRequest* request)
{
	NonLinearEditorModel* model = nleWidget->getModel();
	if (!model)
		return;

	model->clear(true);

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	double curTime = mcu.time;

	// first, display the bml request in an intuitive way
	RequestTrack* requestTrack = new RequestTrack();
	requestTrack->setName(request->actorId);
	model->addTrack(requestTrack);
	RequestBlock* requestBlock = new RequestBlock();
	requestBlock->setName(request->msgId);
	requestBlock->setShowName(false);
	requestTrack->addBlock(requestBlock);
	requestBlock->setStartTime(curTime);
	requestBlock->setEndTime(curTime + 1);
	requestBlock->setColor(fltk::GREEN);

	// dump the xml
	XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
    DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	XMLCh* xmlOutput = theSerializer->writeToString(request->doc);
	theSerializer->release();
	std::wstring xmlStrWide = xmlOutput;
	std::string xmlStr(xmlStrWide.begin(), xmlStrWide.end());
	requestBlock->setInfo(xmlStr);

	std::vector<std::pair<RequestMark*, std::string> > untimedMarks; // marks that have no associated time
	std::map<std::string, double> syncMap; // keeps track of sync points and their times

	int counter = 0;
	for (BML::MapOfSyncPoint::iterator spIter = request->idToSync.begin();
		spIter != request->idToSync.end();
		spIter++)
	{
		RequestMark* requestMark = new RequestMark();
		std::string syncPointName((*spIter).first.begin(), (*spIter).first.end()); 
		requestMark->setName(syncPointName);
		BML::SyncPointPtr syncPointPtr = (*spIter).second;
		if (syncPointPtr.get()->is_set())
		{
			double syncPointTime = syncPointPtr.get()->time;
			requestMark->setStartTime(syncPointTime);
			requestMark->setEndTime(syncPointTime);
		}
		else if (syncPointName == "bml:start") // why doesn't this sync point get set?
		{
			syncPointPtr.get()->time = request->bml_start->time; // careful! we are setting the time of this sync point
			double syncPointTime = syncPointPtr.get()->time;
			if (syncPointTime != syncPointTime) 
				syncPointTime = curTime; // why is this sync point time unset?
			requestMark->setStartTime(syncPointTime);
			requestMark->setEndTime(syncPointTime);
			requestMark->setColor(fltk::RED);
		}
		else
		{
			// save this mark for later since it contains sync point information
			// that we haven't obtained the proper time for yet
			untimedMarks.push_back(std::pair<RequestMark*, std::string>(requestMark, syncPointName));
		}
		//if (requestMark)
			requestBlock->addMark(requestMark);
		
		counter++;
	}

	double triggerTime = 0;
	for (BML::VecOfTriggerEvent::iterator triggerIter = request->triggers.begin();
		 triggerIter != request->triggers.end();
		 triggerIter++)
	{
		BML::TriggerEvent* trigger = (*triggerIter).get();
		std::wstring name = trigger->name;
		BML::MapOfSyncPoint& syncMap = request->idToSync;
		BML::MapOfSyncPoint::iterator syncIter = syncMap.find(name);
		if (syncIter != syncMap.end())
		{
			BML::SyncPoint* syncPoint = (*syncIter).second.get();
			triggerTime = syncPoint->time;
			if (triggerTime != triggerTime)
				triggerTime = curTime; // why is this sync point time unset?
		}
	}

	BML::VecOfBehaviorRequest b = request->behaviors;
	for (BML::VecOfBehaviorRequest::iterator iter = b.begin();
		iter != b.end();
		iter++)
	{
		BML::BehaviorRequestPtr requestPtr = (*iter);
		BML::BehaviorRequest* behavior = requestPtr.get();

		BML::BehaviorSyncPoints::iterator startIter = behavior->behav_syncs.sync_start();
		if (startIter != behavior->behav_syncs.end())
		{
			triggerTime =(*startIter).sync()->time;
			if (triggerTime != triggerTime)
				triggerTime = curTime; // why is this sync point time unset?
		}

		BML::BehaviorScheduler* scheduler = behavior->scheduler.get();
		double readyTime = 0;
		double relaxTime = 0;
		double startTime = 0;
		double strokeTime = 0;
		double endTime = 0;
		double speed = 1;
		BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler = dynamic_cast<BML::BehaviorSchedulerConstantSpeed*>(scheduler);
		if (constantSpeedScheduler)
		{
			readyTime = constantSpeedScheduler->readyTime;
			relaxTime = constantSpeedScheduler->relaxTime;
			startTime = constantSpeedScheduler->startTime;
			strokeTime = constantSpeedScheduler->strokeTime;
			endTime = constantSpeedScheduler->endTime;
			speed = constantSpeedScheduler->speed;
		}
		bool processed = false;

		for (BML::BehaviorSyncPoints::iterator bhIter = behavior->behav_syncs.begin(); 
				bhIter !=  behavior->behav_syncs.end();
				bhIter++)
		{
			BML::SyncPointPtr syncPtr = (*bhIter).sync();
			BML::SyncPoint* sp = syncPtr.get();
			double time = (*bhIter).time();
			if (time != time)
			{
				 BML::SyncPointPtr parentPtr = sp->parent;
				 if (parentPtr)
				 {
					 BML::SyncPoint* parentSp = parentPtr.get();
					 double parentTime = parentSp->time;
					 if (parentTime == parentTime)
					 {
						 float offset = sp->offset;
					 }
				 }
			}

		}

		//BML::BehaviorSpan span = behavior->behav_syncs.getBehaviorSpan();
		std::cout << behavior->unique_id << std::endl;
		behavior->behav_syncs.printSyncIds();
		behavior->behav_syncs.printSyncTimes();





		BML::MotionRequest* motionRequest = dynamic_cast<BML::MotionRequest*>(behavior);
		if (motionRequest)
		{
			processMotionRequest(motionRequest, model, behavior, triggerTime, constantSpeedScheduler, syncMap, untimedMarks);
			processed = true;
		}

		BML::MeControllerRequest* controllerRequest = dynamic_cast<BML::MeControllerRequest*>(behavior);
		if (controllerRequest)
		{
			processControllerRequest(controllerRequest, model, behavior, triggerTime, constantSpeedScheduler, syncMap, untimedMarks);
			processed = true;
		}

		BML::SpeechRequest* speechRequest = dynamic_cast<BML::SpeechRequest*>(behavior);
		if (speechRequest)
		{
			processSpeechRequest(speechRequest, model, behavior, triggerTime, constantSpeedScheduler, syncMap, untimedMarks);
			processed = true;
		}

		BML::EventRequest* eventRequest = dynamic_cast<BML::EventRequest*>(behavior);
		if (eventRequest)
		{
			processEventRequest(eventRequest, model, behavior, triggerTime, constantSpeedScheduler, syncMap, untimedMarks);
			processed = true;
		}

		if (!processed)
		{
			// need to handle additional request types here
			processed = true;
		}
		
	}

	std::vector<std::pair<BML::BehaviorRequest*, Block*> > behaviorBlocks;
	// now match any marks against their proper times
	for (std::vector<std::pair<RequestMark*, std::string> >::iterator iter = untimedMarks.begin();
		iter != untimedMarks.end();
		iter++)
	{
		std::string spName = (*iter).second;
		std::map<std::string, double>::iterator mapIter = syncMap.find(spName);
		if (mapIter != syncMap.end())
		{
			bool processed = false;
			// make sure that event blocks only last for the time of the event
			EventBlock* eventBlock = dynamic_cast<EventBlock*>((*iter).first->getBlock());
			if (eventBlock)
			{
				eventBlock->setStartTime((*mapIter).second);
				eventBlock->setEndTime((*mapIter).second + .5);
				(*iter).first->setStartTime((*mapIter).second);
				(*iter).first->setEndTime((*mapIter).second);
				processed = true;
			}
			// adjust motion blocks
			MotionBlock* motionBlock = dynamic_cast<MotionBlock*>((*iter).first->getBlock());
			if (motionBlock)
			{
				bool found = false;
				for (BML::VecOfBehaviorRequest::iterator behavIter = request->behaviors.begin();
					behavIter != request->behaviors.end();
					behavIter++)
				{
					if (motionBlock->getName() == (*behavIter)->unique_id)
					{
						for (unsigned int b = 0; b < behaviorBlocks.size(); b++)
						{
							if (behaviorBlocks[b].first == (*behavIter).get())
							{
								found = true;
								break;
							}
						}
						if (!found)
							behaviorBlocks.push_back(std::pair<BML::BehaviorRequest*, Block*>((*behavIter).get(), motionBlock));
					}
					
				}
				
				processed = true;
			}
			// adjust nod blocks
			NodBlock* nodBlock = dynamic_cast<NodBlock*>((*iter).first->getBlock());
			if (nodBlock)
			{
				bool found = false;
				for (BML::VecOfBehaviorRequest::iterator behavIter = request->behaviors.begin();
					behavIter != request->behaviors.end();
					behavIter++)
				{
					if (nodBlock->getName() == (*behavIter).get()->unique_id)
					{
						
						for (unsigned int b = 0; b < behaviorBlocks.size(); b++)
						{
							if (behaviorBlocks[b].first == (*behavIter).get())
							{
								found = true;
								break;
							}
						}
						if (!found)
							behaviorBlocks.push_back(std::pair<BML::BehaviorRequest*, Block*>((*behavIter).get(), nodBlock));
					}
				}
				
				processed = true;
				
			}
			if (!processed)
			{
				(*iter).first->setStartTime((*mapIter).second);
				(*iter).first->setEndTime((*mapIter).second);
			}
		}
		else
		{
			// remove this mark
			Block* b = (*iter).first->getBlock();
			b->removeMark((*iter).first);
		}
	}

	//for (int b = 0; b < behaviorBlocks.size(); b++)
	//{
//		adjustSyncPoints(behaviorBlocks[b].first, behaviorBlocks[b].second, syncMap);
//	}
	
	double startTime = 99999;
	double endTime = -99999;
	for (int t = 0; t < model->getNumTracks(); t++)
	{
		Track* track = model->getTrack(t);
		for (int b = 0; b < track->getNumBlocks(); b++)
		{
			Block* block = track->getBlock(b);
			if (block->getEndTime() > endTime)
				endTime = block->getEndTime();
			if (block->getStartTime() < startTime)
				startTime = block->getStartTime();
			for (int m = 0; m < block->getNumMarks(); m++)
			{
				Mark* mark = block->getMark(m);
				if (mark->getEndTime() > endTime)
					endTime = mark->getEndTime();
				if (mark->getStartTime() > startTime)
					startTime = mark->getStartTime();
			}
		}
	}
	// set the main block on the first track to span the max times
	requestBlock->setEndTime(endTime);

	// set the time for the widget
	getEditorWidget()->setViewableTimeStart(startTime);
	getEditorWidget()->setViewableTimeEnd(endTime);
	model->setEndTime(endTime);
	if (model->getNumTracks() > 0)
		contextCounter++;
	std::stringstream strstr;
	strstr << contextCounter;
	model->saveContext(strstr.str());
	updateGUI();
	redraw();
}



void BehaviorWindow::processMotionRequest(BML::MotionRequest* motionRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
										  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
										  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks)
{
	RequestTrack* track = new RequestTrack();
	track->setName("motion");
	model->addTrack(track);
	MotionBlock* block = new MotionBlock();
	track->addBlock(block);
	block->setStartTime(triggerTime);
	block->setEndTime(triggerTime + 1);
	block->setName(behavior->unique_id);
	block->setShowName(false);
	int counter = 0;
	BML::BehaviorSyncPoints syncPoints = behavior->behav_syncs;

	const std::map<std::wstring, std::wstring>& behaviorToNameMap = syncPoints.getBehaviorToSyncNames();
	// start
	{
		BML::BehaviorSyncPoints::iterator syncPointIter = syncPoints.sync_start();
		std::wstring spName = syncPointIter->name();
		std::string spMarkName(spName.begin(), spName.end());
		RequestMark* spMark = new RequestMark();
		spMark->setName(spMarkName);
		if (constantSpeedScheduler)
			spMark->setStartTime(triggerTime + constantSpeedScheduler->startTime / constantSpeedScheduler->speed);
		spMark->setEndTime(spMark->getStartTime());
		block->addMark(spMark);

		const std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.find(spName);
		if (iter != behaviorToNameMap.end())		
		{
			std::string name((*iter).second.begin(), (*iter).second.end());
			untimedMarks.push_back( std::pair<RequestMark*, std::string>(spMark, name));
		}
		
	}
	// ready
	{
		BML::BehaviorSyncPoints::iterator syncPointIter = syncPoints.sync_ready();
		std::wstring spName = syncPointIter->name();
		std::string spMarkName(spName.begin(), spName.end());
		RequestMark* spMark = new RequestMark();
		spMark->setName(spMarkName);
		if (constantSpeedScheduler)
			spMark->setStartTime(triggerTime + constantSpeedScheduler->readyTime / constantSpeedScheduler->speed);
		spMark->setEndTime(spMark->getStartTime());
		block->addMark(spMark);

		const std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.find(spName);
		if (iter != behaviorToNameMap.end())		
		{
			std::string name((*iter).second.begin(), (*iter).second.end());
			untimedMarks.push_back( std::pair<RequestMark*, std::string>(spMark, name));
		}
			
	}
	// stroke start
	{
		BML::BehaviorSyncPoints::iterator syncPointIter = syncPoints.sync_stroke_start();
		std::wstring spName = syncPointIter->name();
		std::string spMarkName(spName.begin(), spName.end());
		RequestMark* spMark = new RequestMark();
		spMark->setName(spMarkName);
		if (constantSpeedScheduler)
			spMark->setStartTime(triggerTime + constantSpeedScheduler->strokeTime / constantSpeedScheduler->speed);
		spMark->setEndTime(spMark->getStartTime());
		block->addMark(spMark);

		const std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.find(spName);
		if (iter != behaviorToNameMap.end())		
		{
			std::string name((*iter).second.begin(), (*iter).second.end());
			untimedMarks.push_back( std::pair<RequestMark*, std::string>(spMark, name));
		}
	}
	// stroke 
	{
		BML::BehaviorSyncPoints::iterator syncPointIter = syncPoints.sync_stroke();
		std::wstring spName = syncPointIter->name();
		std::string spMarkName(spName.begin(), spName.end());
		RequestMark* spMark = new RequestMark();
		spMark->setName(spMarkName);
		if (constantSpeedScheduler)
			spMark->setStartTime(triggerTime + constantSpeedScheduler->strokeTime / constantSpeedScheduler->speed);
		spMark->setEndTime(spMark->getStartTime());
		block->addMark(spMark);

		const std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.find(spName);
		if (iter != behaviorToNameMap.end())		
		{
			std::string name((*iter).second.begin(), (*iter).second.end());
			untimedMarks.push_back( std::pair<RequestMark*, std::string>(spMark, name));
		}
	}
	// stroke end
	{
		BML::BehaviorSyncPoints::iterator syncPointIter = syncPoints.sync_stroke_end();
		std::wstring spName = syncPointIter->name();
		std::string spMarkName(spName.begin(), spName.end());
		RequestMark* spMark = new RequestMark();
		spMark->setName(spMarkName);
		if (constantSpeedScheduler)
			spMark->setStartTime(triggerTime + constantSpeedScheduler->strokeTime / constantSpeedScheduler->speed);
		spMark->setEndTime(spMark->getStartTime());
		block->addMark(spMark);

		const std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.find(spName);
		if (iter != behaviorToNameMap.end())		
		{
			std::string name((*iter).second.begin(), (*iter).second.end());
			untimedMarks.push_back( std::pair<RequestMark*, std::string>(spMark, name));
		}
	}
	// relax
	{
		BML::BehaviorSyncPoints::iterator syncPointIter = syncPoints.sync_relax();
		std::wstring spName = syncPointIter->name();
		std::string spMarkName(spName.begin(), spName.end());
		RequestMark* spMark = new RequestMark();
		spMark->setName(spMarkName);
		if (constantSpeedScheduler)
			spMark->setStartTime(triggerTime + constantSpeedScheduler->relaxTime / constantSpeedScheduler->speed);
		spMark->setEndTime(spMark->getStartTime());
		block->addMark(spMark);

		const std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.find(spName);
		if (iter != behaviorToNameMap.end())		
		{
			std::string name((*iter).second.begin(), (*iter).second.end());
			untimedMarks.push_back( std::pair<RequestMark*, std::string>(spMark, name));
		}
	}
	// end
	{
		BML::BehaviorSyncPoints::iterator syncPointIter = syncPoints.sync_end();
		std::wstring spName = syncPointIter->name();
		std::string spMarkName(spName.begin(), spName.end());
		RequestMark* spMark = new RequestMark();
		spMark->setName(spMarkName);
		if (constantSpeedScheduler)
			spMark->setStartTime(triggerTime + constantSpeedScheduler->endTime / constantSpeedScheduler->speed);
		spMark->setEndTime(spMark->getStartTime());
		block->addMark(spMark);

		const std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.find(spName);
		if (iter != behaviorToNameMap.end())		
		{
			std::string name((*iter).second.begin(), (*iter).second.end());
			untimedMarks.push_back( std::pair<RequestMark*, std::string>(spMark, name));
		}
	}

	MeController* animController = motionRequest->anim_ct;
	MeCtMotion* motion = dynamic_cast<MeCtMotion*>(animController);
	if (motion)
	{ 
		RequestTrack* track = new RequestTrack();
		track->setName("Motion");
		model->addTrack(track);
		MotionBlock* motionBlock = new MotionBlock();
		motionBlock->setName(motion->name());
		motionBlock->setShowName(false);
		motionBlock->setInfo(motion->name());
		track->addBlock(motionBlock);
		double duration = motion->controller_duration();
		double speed = constantSpeedScheduler->speed;
		duration /= speed;
		// TODO
		// speed needs to change the timing of the motion
		// set the duration on the original block on the preceding track as well
		block->setEndTime(block->getStartTime() + duration);
		motionBlock->setStartTime(triggerTime);
		motionBlock->setEndTime(triggerTime + duration);
		RequestMark* inMark = new RequestMark();
		inMark->setName("in");
		inMark->setStartTime(motion->indt() / speed + triggerTime);
		inMark->setEndTime(motion->indt() / speed + triggerTime);
		motionBlock->addMark(inMark);
		RequestMark* outMark = new RequestMark();
		outMark->setName("out");
		motionBlock->addMark(outMark);
		if (motion->outdt() >= motion->indt())
		{
			outMark->setStartTime(motion->outdt() / speed + triggerTime);
			outMark->setEndTime(motion->outdt()/ speed + triggerTime);
		}
		RequestMark* emphasisMark = new RequestMark();
		emphasisMark->setName("emphasis");
		motionBlock->addMark(emphasisMark);
		emphasisMark->setStartTime(motion->emphasist() / speed + triggerTime);
		emphasisMark->setEndTime(motion->emphasist() / speed + triggerTime);
	
	}
}


void BehaviorWindow::processControllerRequest(BML::MeControllerRequest* controllerRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
											  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
											  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks)
{
	MeController* controller = controllerRequest->anim_ct;
	
	MeCtGaze* gazeController = dynamic_cast<MeCtGaze*>(controller);
	if (gazeController)
	{
		RequestTrack* track = new RequestTrack();
		track->setName(controller->controller_type());
		model->addTrack(track);
		RequestBlock* block = new RequestBlock();
		block->setName("gazer");
		track->addBlock(block);
		block->setStartTime(triggerTime);
		block->setEndTime(triggerTime + 1);
	//	block->setColor(fltk::GREEN);
		// find the gaze target
		float x, y, z;
		SkJoint* joint = gazeController->get_target_joint(x, y, z);
		std::stringstream strstr;
		if (joint)
		{
			strstr << joint->skeleton()->name() << "(" << x << ", " << y << ", " << z << ")";
			block->setName(strstr.str());
		}
		// gaze parameters
		std::stringstream gazestr;
		//gazestr << gazeController->print

	}

	BML::NodRequest* nodRequest = dynamic_cast<BML::NodRequest*>(behavior);
	if (nodRequest)
	{
		RequestTrack* nodTrack = new RequestTrack();
		nodTrack->setName("nod");
		model->addTrack(nodTrack);
		NodBlock* nodBlock = new NodBlock();
		nodBlock->setName(behavior->unique_id);
		nodBlock->setShowName(false);
		nodTrack->addBlock(nodBlock);

		nodBlock->setStartTime(triggerTime);
		nodBlock->setEndTime(triggerTime + constantSpeedScheduler->endTime);

		BML::BehaviorSyncPoints syncPoints = behavior->behav_syncs;
		const std::map<std::wstring, std::wstring>& behaviorToNameMap = syncPoints.getBehaviorToSyncNames();
		
		{
			RequestMark* readyMark = new RequestMark();
			readyMark->setName("ready");
			readyMark->setStartTime(triggerTime + constantSpeedScheduler->readyTime);
			readyMark->setEndTime(readyMark->getStartTime());
			nodBlock->addMark(readyMark);
			const std::map<std::wstring, std::wstring>::const_iterator biter = behaviorToNameMap.find(L"ready");
			if (biter != behaviorToNameMap.end())		
			{
				std::string spName((*biter).second.begin(), (*biter).second.end());
				untimedMarks.push_back(std::pair<RequestMark*, std::string>(readyMark, spName));
			}
		}

		{
			RequestMark* strokeMark = new RequestMark();
			strokeMark->setName("stroke");
			strokeMark->setStartTime(triggerTime + constantSpeedScheduler->strokeTime);
			strokeMark->setEndTime(strokeMark->getStartTime());
			nodBlock->addMark(strokeMark);
			const std::map<std::wstring, std::wstring>::const_iterator biter = behaviorToNameMap.find(L"stroke");	
			if (biter != behaviorToNameMap.end())		
			{
				std::string spName((*biter).second.begin(), (*biter).second.end());
				untimedMarks.push_back(std::pair<RequestMark*, std::string>(strokeMark, spName));
			}
		}

		{
			RequestMark* relaxMark = new RequestMark();
			relaxMark->setName("relax");
			relaxMark->setStartTime(triggerTime + constantSpeedScheduler->relaxTime);
			relaxMark->setEndTime(relaxMark->getStartTime());
			nodBlock->addMark(relaxMark);
			const std::map<std::wstring, std::wstring>::const_iterator biter = behaviorToNameMap.find(L"relax");
			if (biter != behaviorToNameMap.end())		
			{
				std::string spName((*biter).second.begin(), (*biter).second.end());
				untimedMarks.push_back(std::pair<RequestMark*, std::string>(relaxMark, spName));
			}
		}

		{
			RequestMark* endMark = new RequestMark();
			endMark->setName("end");
			endMark->setStartTime(triggerTime + constantSpeedScheduler->endTime);
			endMark->setEndTime(endMark->getStartTime());
			nodBlock->addMark(endMark);
			const std::map<std::wstring, std::wstring>::const_iterator biter = behaviorToNameMap.find(L"end");
			if (biter != behaviorToNameMap.end())		
			{
				std::string spName((*biter).second.begin(), (*biter).second.end());
				untimedMarks.push_back(std::pair<RequestMark*, std::string>(endMark, spName));
			}
		}
	}

	BML::PostureRequest* postureRequest = dynamic_cast<BML::PostureRequest*>(behavior);
	if (postureRequest)
	{
		RequestTrack* track = new RequestTrack();
		track->setName(controller->controller_type());
		model->addTrack(track);
		MeController* animController = postureRequest->anim_ct;
		MeCtMotion* motion = dynamic_cast<MeCtMotion*>(animController);
		if (motion)
		{ 
			RequestTrack* track = new RequestTrack();
			track->setName("Posture");
			model->addTrack(track);
			MotionBlock* motionBlock = new MotionBlock();
			motionBlock->setName(behavior->unique_id);
			motionBlock->setInfo(motion->name());
			track->addBlock(motionBlock);
			double duration = motion->controller_duration();
			if (duration < 0)
			{
				// if duration < 0, this motion is looped,
				// but I'd like to see the duration of the original motion
				// regardless
				SkMotion* skMotion = motion->motion();
				duration = skMotion->duration();
				motionBlock->setColor(fltk::GRAY25);

			}
			motionBlock->setStartTime(triggerTime);
			motionBlock->setEndTime(triggerTime + duration);
			RequestMark* inMark = new RequestMark();
			inMark->setName("in");
			inMark->setStartTime(motion->indt() + triggerTime);
			inMark->setEndTime(motion->indt() + triggerTime);
			motionBlock->addMark(inMark);
			RequestMark* outMark = new RequestMark();
			outMark->setName("out");
			if (motion->outdt() >= motion->indt())
			{
				outMark->setStartTime(motion->outdt() + triggerTime);
				outMark->setEndTime(motion->outdt() + triggerTime);
			}
			motionBlock->addMark(outMark);
		}
	}
}

void BehaviorWindow::processSpeechRequest(BML::SpeechRequest* speechRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
										  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
										  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks)
{
	RequestTrack* track = new RequestTrack();
	track->setName("speech");
	model->addTrack(track);
	RequestBlock* block = new RequestBlock();
	block->setName(speechRequest->unique_id);
	block->setShowName(false);
	block->setInfo(speechRequest->unique_id);
	track->addBlock(block);
	block->setStartTime(triggerTime);
	block->setEndTime(triggerTime + 1); // this should be based on sound duration
	block->setColor(fltk::GREEN);

	SmartBody::SpeechInterface* speechInterface = speechRequest->get_speech_interface();
	char* audioFilename = speechInterface->getSpeechAudioFilename(speechRequest->get_speech_request_id());
	SmartBody::AudioFileSpeech* audioSpeechInterface = dynamic_cast<SmartBody::AudioFileSpeech*>(speechInterface);
	char* playCommand = speechInterface->getSpeechPlayCommand(speechRequest->get_speech_request_id());
	if (audioSpeechInterface)
	{
		RequestTrack* visemeTrack = new RequestTrack();
		visemeTrack->setName("viseme");
		model->addTrack(visemeTrack);
		RequestBlock* visemeBlock = new RequestBlock();
		visemeBlock->setName(speechRequest->unique_id);
		visemeBlock->setStartTime(triggerTime);
		visemeBlock->setShowName(false);
		visemeTrack->addBlock(visemeBlock);
		float lastTime = 0;
		const std::vector<SmartBody::VisemeData *>* visemes = audioSpeechInterface->getVisemes(speechRequest->get_speech_request_id());
		for (unsigned int v = 0; v < visemes->size(); v++)
		{
			SmartBody::VisemeData* viseme = (*visemes)[v];
			const char* id =  viseme->id();
			float visemeTime = viseme->time();
			float weight = viseme->weight();
			float blendDuration = viseme->duration();
			RequestMark* visemeMark = new RequestMark();
			visemeMark->setName(id);
			visemeMark->setStartTime(triggerTime + visemeTime);
			visemeMark->setEndTime(visemeMark->getStartTime());
			std::stringstream strstr;
			strstr << "Id = " << id << std::endl << "Time = " << visemeTime << std::endl << "Weight = " << weight << std::endl << "Duration = " << blendDuration;
			visemeMark->setInfo(strstr.str());
			if (weight == 0.0)
				visemeMark->setColor(fltk::RED);
			else if (weight == 1.0)
				visemeMark->setColor(fltk::GREEN);

			visemeBlock->addMark(visemeMark);
			if (visemeTime > lastTime)
				lastTime = visemeTime;
		}
		visemeBlock->setEndTime(triggerTime + lastTime);
		block->setEndTime(triggerTime + lastTime);


		stdext::hash_map< SmartBody::RequestId, SmartBody::AudioFileSpeech::SpeechRequestInfo >& speechRequestInfo = audioSpeechInterface->getSpeechRequestInfo();
		for (stdext::hash_map< SmartBody::RequestId, SmartBody::AudioFileSpeech::SpeechRequestInfo >::iterator iter = speechRequestInfo.begin();
			iter != speechRequestInfo.end();
			iter++)
		{
			RequestTrack* timeMarkerTrack = new RequestTrack();
			timeMarkerTrack->setName("speechtimemarkers");
			model->addTrack(timeMarkerTrack);
			RequestBlock* timeMarkerBlock = new RequestBlock();
			timeMarkerBlock->setName(speechRequest->unique_id);
			timeMarkerTrack->addBlock(timeMarkerBlock);
			timeMarkerBlock->setStartTime(triggerTime);
			timeMarkerBlock->setEndTime(triggerTime + lastTime);
			timeMarkerBlock->setShowName(false);

			SmartBody::RequestId reqId = (*iter).first;
			SmartBody::AudioFileSpeech::SpeechRequestInfo& info = (*iter).second;
			for(stdext::hash_map< std::string, float >::iterator markerIter = info.timeMarkers.begin();
				markerIter != info.timeMarkers.end();
				markerIter++)
			{
				std::string markerName = (*markerIter).first;
				float markerTime = (*markerIter).second;
				RequestMark* timeMark = new RequestMark();
				timeMark->setName(markerName);
				timeMark->setStartTime(triggerTime + markerTime);
				timeMark->setEndTime(timeMark->getStartTime());
				timeMarkerBlock->addMark(timeMark);
				// add these times to the syncMap so that we can use them 
				// to calculate other timings
				std::string syncMapName = speechRequest->local_id;
				syncMapName.append(":");
				syncMapName.append(markerName);
				syncMap.insert(std::pair<std::string, double>(syncMapName, triggerTime + markerTime));
			}
		}
	}
	else
	{
		text_speech* textSpeech = dynamic_cast<text_speech*>(speechInterface);
		if (textSpeech)
		{
				int x = 2;
		}
		else
		{
			remote_speech* remoteSpeech = dynamic_cast<remote_speech*>(speechInterface);
			if (remoteSpeech)
			{
				int x = 2;
			}

		}
	}

	std::stringstream speechStr;
	speechStr << block->getInfo() << std::endl << audioFilename << std::endl;
	if (playCommand)
		speechStr << playCommand;
	block->setInfo(speechStr.str());

	BML::MapOfSyncPoint& wordBreaks = speechRequest->getWorkBreakSync();
	int counter = 0;
	for (BML::MapOfSyncPoint::iterator wordBreakIter = wordBreaks.begin();
		 wordBreakIter != wordBreaks.end();
		 wordBreakIter++)
	{
		BML::SyncPoint* syncPoint = (*wordBreakIter).second.get();
		RequestMark* mark = new RequestMark();
		std::string name((*wordBreakIter).first.begin(), (*wordBreakIter).first.end());
		mark->setName(name);
		double time = syncPoint->time;
		double offset = syncPoint->offset;
		if (time != time)
		{
			time = 0; // word breaks should have times set
			untimedMarks.push_back(std::pair<RequestMark*, std::string>(mark, mark->getName()));
		}
		//mark->setStartTime(triggerTime + time);
		//mark->setEndTime(triggerTime + time);
		// word breaks use absolute time?
		mark->setStartTime(time);
		mark->setEndTime(time);
		
		block->addMark(mark);
		counter++;
	}
}

void BehaviorWindow::processEventRequest(BML::EventRequest* eventRequest, NonLinearEditorModel* model, BML::BehaviorRequest* behavior, 
										  double triggerTime, BML::BehaviorSchedulerConstantSpeed* constantSpeedScheduler, 
										  std::map<std::string, double>& syncMap, std::vector<std::pair<RequestMark*, std::string> >& untimedMarks)
{
	RequestTrack* eventTrack = new RequestTrack();
	eventTrack->setName("event");
	EventBlock* eventBlock = new EventBlock();
	eventBlock->setName(eventRequest->getMessage());
	eventBlock->setShowName(false);
	eventBlock->setInfo(eventRequest->getMessage());
	eventBlock->setStartTime(triggerTime);
	eventBlock->setEndTime(triggerTime + 1);
	eventBlock->setColor(fltk::WHITE);
	eventTrack->addBlock(eventBlock);
	model->addTrack(eventTrack);

	std::string syncPointName = eventRequest->getSyncPointName();
	RequestMark* syncPointMark = new RequestMark();
	syncPointMark->setName(syncPointName);
	eventBlock->addMark(syncPointMark);
	untimedMarks.push_back(std::pair<RequestMark*, std::string> (syncPointMark, syncPointName));
}

void BehaviorWindow::adjustSyncPoints(BML::BehaviorRequest* behavior, Block* block, std::map<std::string, double>& syncMap)
{
	std::string stages[] = { "start", "ready", "stroke_start", "stroke", "stroke_end", "relax", "end" };
	bool hasTiming[] =     { false,   false,   false,          false,    false,        false,   false };
	bool overrides[] =     { false,   false,   false,          false,    false,        false,   false };
	double origTiming[] =   {     0,       0,       0,              0,        0,            0,       0 };
	double newTiming[] =    {     0,       0,       0,              0,        0,            0,       0 };
	
	// determine which sync points are present and have legitimate timings
	for (int x = 0; x < 7; x ++)
	{
		Mark* mark = block->getMark(stages[x]);
		if (mark)
			if (mark->getStartTime() == mark->getStartTime())
			{
				hasTiming[x] = true;
				origTiming[x] = mark->getStartTime();
			}
	}
	
	const std::map<std::wstring, std::wstring>& behaviorToNameMap = behavior->behav_syncs.getBehaviorToSyncNames();

	// first pass, get the proper timings
	for (std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.begin();
		iter != behaviorToNameMap.end();
		iter++)
	{
		std::string syncPointName((*iter).first.begin(), (*iter).first.end());
		std::string syncPointTiming((*iter).second.begin(), (*iter).second.end());

		std::map<std::string, double>::iterator mapIter = syncMap.find(syncPointTiming);
		if (mapIter != syncMap.end())
		{
			// find the mark associated with this sync point
			RequestMark* syncPointMark = dynamic_cast<RequestMark*>(block->getMark(syncPointName));
			if (syncPointMark)
			{
				syncPointMark->setStartTime((*mapIter).second);
				syncPointMark->setEndTime(syncPointMark->getStartTime());
			}
			else
			{
				// did not find mark - this is a problem!
				syncPointMark = new RequestMark();
				syncPointMark->setName(syncPointName);
				syncPointMark->setStartTime((*mapIter).second);
				syncPointMark->setEndTime(syncPointMark->getStartTime());
				block->addMark(syncPointMark);
			}
			for (int x = 0; x < 7; x ++)
			{
				if (syncPointName == stages[x])
				{
					newTiming[x] = (*mapIter).second;
				}
			}
		}
	}

	// second pass, adjust the other timings accordingly
	for (std::map<std::wstring, std::wstring>::const_iterator iter = behaviorToNameMap.begin();
		iter != behaviorToNameMap.end();
		iter++)
	{
		std::string syncPointName((*iter).first.begin(), (*iter).first.end());
		for (int x = 0; x < 7; x++)
			if (syncPointName == stages[x])
				overrides[x] = true;
	}

	for (int x = 0; x < 7; x++)
	{
		if (overrides[x])
		{
			double timingDiff = newTiming[x] - origTiming[x];
			for (int y = x + 1; y < 7; y++)
			{
				if (overrides[y])
					break;

				if (hasTiming[y])
				{
					Mark* mark = block->getMark(stages[y]);
					if (mark)
					{
						if (!hasTiming[x])
						{
							mark->setStartTime(newTiming[x]);
							mark->setEndTime(newTiming[x]);
						}
						else
						{
							double oldTime = mark->getStartTime();
							double newTime = oldTime + timingDiff;
							mark->setStartTime(newTime);
							mark->setEndTime(newTime);
						}
						
					}
			
				}
			}
			
		}
	}

	// adjust the block to min/max of marks
	double minTime = 9999999;
	double maxTime = -999999;
	for (int m = 0; m < block->getNumMarks(); m++)
	{
		Mark* mark = block->getMark(m);
		if (mark->getStartTime() < minTime)
			minTime = mark->getStartTime();
		if (mark->getEndTime() > maxTime)
			maxTime = mark->getEndTime();
	}
	block->setStartTime(minTime);
	block->setEndTime(maxTime);

}

BehaviorViewerFactory::BehaviorViewerFactory()
{
}

BMLViewer* BehaviorViewerFactory::create(int x, int y, int w, int h)
{
	BehaviorWindow* behaviorWindow = new BehaviorWindow(x, y, w, h, "Behavior Requests");
	return behaviorWindow;
}

void BehaviorViewerFactory::destroy(BMLViewer* viewer)
{
	delete viewer;
}


void BehaviorWindow::OnRequest(BML::BmlRequest* request, void* data)
{
	BehaviorWindow* behaviorWindow = (BehaviorWindow*) data;
	behaviorWindow->updateBehaviors(request);
}