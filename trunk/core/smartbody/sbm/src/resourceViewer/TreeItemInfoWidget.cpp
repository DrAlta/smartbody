#include <sbm/mcontrol_util.h>
#include <FL/Fl_Float_Input.H>
#include "TreeItemInfoWidget.h"
#include "ResourceWindow.h"
#include <channelbufferviewer/GlChartViewArchive.hpp>
#include <sbm/Event.h>

const int Pad = 10;

/************************************************************************/
/* Tree Item Info Widget                                                */
/************************************************************************/

TreeItemInfoWidget::TreeItemInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type ) : Fl_Group(x,y,w,h), treeItem(inputItem), itemType(type)
{	
	this->begin();
// 	Fl_Input* info = new Fl_Input(Pad*5+x,Pad+y,120,20,"Type");
// 	info->value(name);
// 	info->type(FL_NORMAL_OUTPUT);
// 	Fl_Input* itemInfo = new Fl_Input(Pad*5+x,Pad*4+y,120,20,"Name");
// 	itemInfo->value(inputItem->label());
// 	itemInfo->type(FL_NORMAL_OUTPUT);
	this->end();
}

/************************************************************************/
/* Skeleton Info                                                        */
/************************************************************************/

SkeletonItemInfoWidget::SkeletonItemInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type ) : TreeItemInfoWidget(x,y,w,h,name,inputItem,type)
{
	jointInfoObject = new TreeInfoObject();
	this->begin();
	skeletonTree = new Fl_TreeHorizontal(Pad*2+x,Pad*2+y,w-30,h-400);//new Fl_Tree(10,10,w - 300, h - 30);			
	skeletonTree->callback(treeCallBack,this);	
	attrWindow = new AttributeWindow(jointInfoObject,Pad*2+x,Pad*2+y+h-380,w-30,360,"Joint Attributes");
	attrWindow->begin();
	attrWindow->end();
	this->end();	
	this->resizable(skeletonTree);
	itemSkeleton = NULL;
	skeletonName = inputItem->label();
	updateWidget();
}

void SkeletonItemInfoWidget::updateWidget()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	skeletonTree->clear_children(skeletonTree->root());
	if (!treeItem->label())
		return;

	std::string skelName = skeletonName;	
	if (mcu.skeleton_map.find(skelName) == mcu.skeleton_map.end())
		return; // skeleton is lost, no update

	//LOG("skelName = %s\n",skelName.c_str());
	itemSkeleton = mcu.skeleton_map[skelName];
	updateSkeletonTree(skeletonTree->root(),itemSkeleton);	
}

void SkeletonItemInfoWidget::updateJointAttributes(std::string jointName)
{
	SkJoint* curJoint = itemSkeleton->search_joint(jointName.c_str());		
	if (!curJoint)
		return;
	std::string posName[3] = { "pos X", "pos Y", "pos Z" };
	std::string offsetName[3] = { "offset X", "offset Y", "offset Z" };
	std::string quatName[4] = { "quat X", "quat Y", "quat Z", "quat W"};
	jointInfoObject->clearAttributes();	
	attrWindow->cleanUpWidgets();

	SrVec offset = curJoint->offset();
	//jointInfoObject->createVec3Attribute("offset",offset.x,offset.y,offset.z,true, "Basic", 20, false, false, false, "?");
	//jointInfoObject->setVec3Attribute("offset",offset.x,offset.y,offset.z);
	for (int i=0;i<3;i++)
	{
		jointInfoObject->createDoubleAttribute(offsetName[i],offset[i], true, "Basic",10+i , true, false, false, "?");		
		//attrWindow->notify(jointInfoObject->getAttribute(offsetName[i]));
	}
	//attrWindow->notify(jointInfoObject);

	for (int i=0;i<3;i++)
	{
		if (!curJoint->pos()->frozen(i))
		{			
			jointInfoObject->createDoubleAttribute(posName[i],curJoint->pos()->value(i), true, "Basic", 20+i, true, false, false, "?");
			//attrWindow->notify(jointInfoObject->getAttribute(posName[i]));
		}
	}

	SrQuat q = curJoint->quat()->value();	
	jointInfoObject->createDoubleAttribute(quatName[0],q.x,true, "Basic", 31, true, false, false, "?");
	jointInfoObject->createDoubleAttribute(quatName[1],q.y,true, "Basic", 32, true, false, false, "?");
	jointInfoObject->createDoubleAttribute(quatName[2],q.z,true, "Basic", 33, true, false, false, "?");
	jointInfoObject->createDoubleAttribute(quatName[3],q.w,true, "Basic", 34, true, false, false, "?");
	//for (int i=0;i<4;i++)
	//	attrWindow->notify(jointInfoObject->getAttribute(quatName[i]));

	//attrWindow->reorderAttributes();	
	this->attrWindow->setDirty(true);	
	this->attrWindow->redraw();
	this->redraw();
}

void SkeletonItemInfoWidget::updateSkeletonTree( Fl_Tree_Item* root, SkSkeleton* skel )
{
	SkJoint* skelRoot = skel->root();	
	root->label(skelRoot->name().c_str());	
	for (int i=0;i<skelRoot->num_children();i++)
	{
		updateJointTree(root,skelRoot->child(i));
	}
}

void SkeletonItemInfoWidget::updateJointTree( Fl_Tree_Item* root, SkJoint* node )
{	
	//skeletonTree->sortorder(FL_TREE_SORT_ASCENDING);	

	Fl_Tree_Item* treeItem = skeletonTree->add(root,node->name().c_str());

	std::string posName[3] = { "pos X", "pos Y", "pos Z" };
	skeletonTree->sortorder(FL_TREE_SORT_NONE);
// 	for (int i=0;i<3;i++)
// 	{
// 		if (!node->pos()->frozen(i))
// 		{			
// 			skeletonTree->add(treeItem,posName[i].c_str());
// 		}
// 	}	
// 	if (node->rot_type() == SkJoint::TypeQuat)
// 		skeletonTree->add(treeItem,"quat");
	
	for (int i=0;i<node->num_children();i++)
	{
		SkJoint* child = node->child(i);
		updateJointTree(treeItem,child);
	}
}

void SkeletonItemInfoWidget::treeCallBack( Fl_Widget* widget, void* data )
{
	Fl_Tree      *tree = (Fl_Tree*)widget;
	Fl_Tree_Item *item = (Fl_Tree_Item*)tree->callback_item();	// get selected item	
	SkeletonItemInfoWidget* itemInfoWidget = (SkeletonItemInfoWidget*)data;

	if (tree->callback_reason() == FL_TREE_REASON_SELECTED && item && itemInfoWidget)
	{
		itemInfoWidget->updateJointAttributes(item->label());
	}
}

/************************************************************************/
/*  Motion Item Info Widget                                             */
/************************************************************************/

MotionItemInfoWidget::MotionItemInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type ) : TreeItemInfoWidget(x,y,w,h,name,inputItem,type)
{
	channelInfoObject = new TreeInfoObject();
	this->begin();
	channelBrowser = new Fl_Hold_Browser(Pad*2+x,Pad*2+y,w-30,h-400,"Channels");//new Fl_Tree(10,10,w - 300, h - 30);			
	channelBrowser->callback(browserCallBack,this);	
	frameSlider = new Fl_Value_Slider(Pad*2+x,Pad*2+y+h-360,w-30,20,"Frames");
	frameSlider->type(FL_HORIZONTAL);
	frameSlider->callback(sliderCallBack,this);
	attrWindow = new AttributeWindow(channelInfoObject,Pad*2+x,Pad*2+y+h-310,w-30,280,"");
	attrWindow->begin();
	attrWindow->end();
	this->end();	
	this->resizable(channelBrowser);	
	motionName = inputItem->label();
	updateWidget();
}

void MotionItemInfoWidget::updateWidget()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	channelBrowser->clear();	
	
	if (mcu.motion_map.find(motionName) == mcu.motion_map.end())
		return; 	
	SkMotion* motion = mcu.motion_map[motionName];
	frameSlider->bounds(1,motion->frames());
	frameSlider->step(1.0);
	motionFrame = 0;
	channelIndex = -1;
	SkChannelArray& channels = motion->channels();
	for (int i = 0; i < channels.size(); i++)
	{
		std::string chanName = channels.name(i).c_str();
		std::string typeName = channels[i].type_name();
		channelBrowser->add((chanName+"."+typeName).c_str());
	}
}

void MotionItemInfoWidget::browserCallBack( Fl_Widget* widget, void* data )
{
	Fl_Hold_Browser* browser = dynamic_cast<Fl_Hold_Browser*>(widget);
	MotionItemInfoWidget* itemInfoWidget = (MotionItemInfoWidget*)data;
	if (browser && itemInfoWidget)
	{
		itemInfoWidget->setChannelIndex(browser->value()-1);		
		itemInfoWidget->updateChannelAttributes();
	}
}

void MotionItemInfoWidget::sliderCallBack( Fl_Widget* widget, void* data )
{
	Fl_Value_Slider* slider = dynamic_cast<Fl_Value_Slider*>(widget);
	MotionItemInfoWidget* itemInfoWidget = (MotionItemInfoWidget*)data;
	if (slider && itemInfoWidget)
	{
		itemInfoWidget->setMotionFrame((int)slider->value()-1);	
		itemInfoWidget->updateChannelAttributes();
	}
}

void MotionItemInfoWidget::updateChannelAttributes()
{		
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.motion_map.find(motionName) == mcu.motion_map.end())
		return; 	
	SkMotion* motion = mcu.motion_map[motionName];
	SkChannelArray& channels = motion->channels();
	if (motionFrame < 0 || motionFrame >= motion->frames() || channelIndex < 0 || channelIndex >= channels.size())
		return;

	channelInfoObject->clearAttributes();	
	attrWindow->cleanUpWidgets();
	SkChannel& chan = channels[channelIndex];
	std::string chanName = channels.name(channelIndex);
	std::string typeName = channels[channelIndex].type_name();	
	int floatIdx = channels.float_position(channelIndex);
	float* buffer = motion->posture(motionFrame);
	std::string tag[4] = {"W", "X", "Y", "Z"};
	for (int i=0;i<chan.size();i++)
	{
		std::string attrName = (chanName+"."+typeName);
		if (chan.size() > 1)
			attrName += tag[i];			
		DoubleAttribute* attr = channelInfoObject->createDoubleAttribute(attrName.c_str(),buffer[floatIdx+i],true,"Basic",10+i,true,false,false,"?");
		//attrWindow->notify(attr);
	}
	if (chan.type == SkChannel::Quat)
	{
		SrQuat quat = SrQuat(&buffer[floatIdx]);
		SrVec euler = GlChartViewSeries::GetEulerFromQuaternion(quat);
		std::string attrName = (chanName+"."+"euler");
		for (int k=0;k<3;k++)
		{			
			DoubleAttribute* attr = channelInfoObject->createDoubleAttribute((attrName+tag[k+1]).c_str(),euler[k],true,"Basic",20+k,true,false,false,"?");
		}
	}

	attrWindow->reorderAttributes();	
	this->attrWindow->setDirty(true);	
	this->attrWindow->redraw();
	this->redraw();
}
/************************************************************************/
/* Path Info Widget                                                     */
/************************************************************************/
PathItemInfoWidget::PathItemInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type ) : TreeItemInfoWidget(x,y,w,h,name,inputItem,type)
{
	this->begin();
	std::string pathChooserTitle = "Add Path for ";
	pathChooserTitle = pathChooserTitle + name;
	pathChooser = new Fl_File_Chooser(".", "*", Fl_File_Chooser::DIRECTORY, pathChooserTitle.c_str());
	Fl_Button* dirButton = new Fl_Button( x + 20 , y + 10 , 100, 20, "Add Path");
	dirButton->callback(addDirectoryCallback, this);
	this->end();	
}

void PathItemInfoWidget::addDirectoryCallback( Fl_Widget* widget, void* data )
{
	PathItemInfoWidget* pathInfoWidget = (PathItemInfoWidget*)data;
	pathInfoWidget->addDirectory(NULL);	
}
void PathItemInfoWidget::addDirectory( const char* dirName )
{
	pathChooser->show();
	while (pathChooser->visible()) {
		Fl::wait();
	}	
	char relativePath[256];
	int count = pathChooser->count();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (count > 0)
	{		
		for (int i = 1; i <= count; i ++)
		{
			if (!pathChooser->value(i))
				break;
			fl_filename_relative(relativePath, sizeof(relativePath), pathChooser->value(i));					
			
			std::string paraType = getTypeParameter(itemType);
			std::string pathCmd = "path ";
			pathCmd = pathCmd + paraType + relativePath;
			mcu.execute(const_cast<char*>(pathCmd.c_str()));					
		}		
	}
}

std::string PathItemInfoWidget::getTypeParameter( int type )
{
	std::string paraType = "seq ";
	switch (type)
	{
	case ResourceWindow::ITEM_SEQ_PATH :
		paraType = "seq ";
		break;
	case ResourceWindow::ITEM_AUDIO_PATH :
		paraType = "audio ";
		break;
	case ResourceWindow::ITEM_ME_PATH :
		paraType = "me ";
		break;
	case ResourceWindow::ITEM_MESH_PATH :
		paraType = "mesh ";
		break;
	}
	return paraType;
}

/************************************************************************/
/* Seq Item Info Widget                                                 */
/************************************************************************/
SeqItemInfoWidget::SeqItemInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type ) : TreeItemInfoWidget(x,y,w,h,name,inputItem,type)
{
	seqFilename = inputItem->label();
	this->begin();
	//runSeqButton = new Fl_Button()
	runSeqButton = new Fl_Button( Pad*2+x,Pad*2+y, 100, 20, "Run Seq");
	runSeqButton->callback(runSeqCallback,this);
	editSeqButton = new Fl_Button( Pad*2+x + 130,Pad*2+y, 100, 20, "Edit Seq");
	editSeqButton->callback(editSeqCallback,this);
	textDisplay = new Fl_Text_Display(Pad*2+x,Pad*2+y + 30,w-40,h-70);		
	textDisplay->color(FL_GRAY);
	textDisplay->textcolor(FL_BLACK);
	textBuffer = new Fl_Text_Buffer();
	textDisplay->buffer(textBuffer);
	this->end();
	updateWidget();
}

void SeqItemInfoWidget::updateWidget()
{
	// clean up text buffer
	textBuffer->select(0, textBuffer->length());
	textBuffer->remove_selection();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string fullSeqPath;
	FILE* fp = mcu.open_sequence_file(seqFilename.c_str(),fullSeqPath);
	//LOG("seq file name = %s, full path = %s\n",seqFilename.c_str(),fullSeqPath.c_str());
	if (fp)
	{
		textBuffer->loadfile(fullSeqPath.c_str());
		seqFullPathName = fullSeqPath;
		fclose(fp);
	}
	textDisplay->redraw();
	//textBuffer->loadfile()
}

void SeqItemInfoWidget::runSeqCallback( Fl_Widget* widget, void* data )
{
	SeqItemInfoWidget* seqInfoWidget = (SeqItemInfoWidget*)data;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (seqInfoWidget)
	{
		std::string seqName = seqInfoWidget->getSeqFile();
		std::string seqCmd = "seq ";
		seqCmd += seqName;
		LOG("seq cmd = %s\n",seqCmd.c_str());
		mcu.execute(const_cast<char*>(seqCmd.c_str()));
	}
}

void SeqItemInfoWidget::editSeqCallback( Fl_Widget* widget, void* data )
{
	SeqItemInfoWidget* seqInfoWidget = (SeqItemInfoWidget*)data;
	if (seqInfoWidget)
	{
		std::string seqFullPath = seqInfoWidget->getSeqFullPathName();
#ifdef WIN32 // forking a process is system specific		
		STARTUPINFOA siStartupInfo; 
		PROCESS_INFORMATION piProcessInfo; 
		memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
		memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
		siStartupInfo.cb = sizeof(siStartupInfo); 		
		std::string cmd = "notepad.exe ";		
		BOOL success = CreateProcess(NULL,const_cast<LPSTR>((cmd+seqFullPath).c_str()),0, 0, false, 
									 CREATE_DEFAULT_ERROR_MODE, 0, 0,&siStartupInfo, &piProcessInfo);		
#else
		pid_t child_pid;
		child_pid = fork();
		if (child_pid == 0)
		{
		  char* pathChar = const_cast<char*>(seqFullPath.c_str());
		  
		  char* guiEditorName = getenv ("VISUAL");		  
		  char* argList[3];
		  if (guiEditorName)
			  argList[0] = guiEditorName;
		  else
			  argList[0] = "gedit";
		  argList[1] = pathChar;
		  argList[2] = NULL;
		  execvp("gedit",argList);
		}		
#endif
	}
}
/************************************************************************/
/* Event Item Info Widget                                                 */
/************************************************************************/

EventItemInfoWidget::EventItemInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type )
: TreeItemInfoWidget(x,y,w,h,name,inputItem,type)
{
	eventInfoObject = new TreeInfoObject();
	eventName = inputItem->label();
	eventInfoObject->setName(eventName);
	eventInfoObject->createStringAttribute("EventType","",true,"Basic",10,false,false,false,"?");
	eventInfoObject->createStringAttribute("Action","",true,"Basic",20,false,false,false,"?");
	updateWidget();

	EventManager* manager = EventManager::getEventManager();
	EventHandlerMap& eventMap = manager->getEventHandlers();		
	eventInfoObject->registerObserver(this);
	this->begin();
		attrWindow = new AttributeWindow(eventInfoObject,x,y,w,h,name);
		attrWindow->begin();
		attrWindow->end();			
		Fl_Button* eventButton = new Fl_Button( x + 20 , y + 80 , 100, 20, "Remove Event");
		if (eventMap.find(eventName) == eventMap.end())		
			eventButton->label("Add New Event");		
		eventButton->callback(addEventCallback, this);		
	this->end();

}

void EventItemInfoWidget::updateWidget()
{	
	EventManager* manager = EventManager::getEventManager();
	EventHandlerMap& eventMap = manager->getEventHandlers();	
	EventHandlerMap::iterator mi = eventMap.find(eventName);
	if (mi != eventMap.end())
	{
		EventHandler* handler = mi->second;
		eventInfoObject->setStringAttribute("EventType", mi->first);
		BasicHandler* basicHandler = dynamic_cast<BasicHandler*>(handler);
		if (basicHandler)
		{
			eventInfoObject->setStringAttribute("Action",basicHandler->getAction());	
		}
	}		
}

void EventItemInfoWidget::notify( DSubject* subject )
{
	EventManager* manager = EventManager::getEventManager();
	EventHandlerMap& eventMap = manager->getEventHandlers();
	EventHandlerMap::iterator mi = eventMap.find(eventName);
	if (mi != eventMap.end())
	{
		EventHandler* handler = mi->second;
		BasicHandler* basicHandler = dynamic_cast<BasicHandler*>(handler);
		if (basicHandler)
			basicHandler->setAction(eventInfoObject->getStringAttribute("Action"));		
	}		
}

void EventItemInfoWidget::addEventCallback( Fl_Widget* widget, void* data )
{
	EventItemInfoWidget* pathInfoWidget = (EventItemInfoWidget*)data;
	std::string buttonLabel = widget->label();
	if (buttonLabel == "Add New Event")
		pathInfoWidget->addNewEvent();	
	else if (buttonLabel == "Remove Event")
		pathInfoWidget->removeEvent();
}

void EventItemInfoWidget::addNewEvent()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string eventType = eventInfoObject->getStringAttribute("EventType");
	std::string eventAction = eventInfoObject->getStringAttribute("Action");	
	if (eventType != "" && eventAction != "")
	{
		std::string eventCmd = "registerevent " + eventType + " " + "\"" + eventAction + "\"";	
		mcu.execute(const_cast<char*>(eventCmd.c_str()));		
	}
}

void EventItemInfoWidget::removeEvent()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::string eventType = eventInfoObject->getStringAttribute("EventType");
	std::string eventAction = eventInfoObject->getStringAttribute("Action");	
	if (eventType != "" && eventAction != "")
	{
		std::string eventCmd = "unregisterevent " + eventType;	
		mcu.execute(const_cast<char*>(eventCmd.c_str()));		
		//LOG("Remove Event %s",eventType.c_str());
	}
}
/************************************************************************/
/* Pawn Item Info Widget                                                */
/************************************************************************/
PawnItemInfoWidget::PawnItemInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type, DObserver* observerWindow ) 
: TreeItemInfoWidget(x,y,w,h,name,inputItem,type)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	updateWidget();
	//if (observerWindow)
	//	pawnInfoObject->registerObserver(observerWindow);
	pawnInfoObject->registerObserver(this);
	this->begin();
		SbmPawn* pawn = mcu.getPawn(pawnName);
		attrWindow = new AttributeWindow(pawn,x,y,w,h,name);
		attrWindow->begin();
		attrWindow->end();
	this->end();
}

void PawnItemInfoWidget::updateWidget()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(pawnName);
	if (!pawn) return;
	float x, y, z, h, p, r;
	pawn->get_world_offset(x,y,z,h,p,r);
	pawnInfoObject->setDoubleAttribute("pos X",x);
	pawnInfoObject->setDoubleAttribute("pos Y",y);
	pawnInfoObject->setDoubleAttribute("pos Z",z);

	pawnInfoObject->setDoubleAttribute("rot X",h);
	pawnInfoObject->setDoubleAttribute("rot Y",p);
	pawnInfoObject->setDoubleAttribute("rot Z",r);
	//pawnInfoObject->getAttribute("pos X")->

}

void PawnItemInfoWidget::notify( DSubject* subject )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(pawnName);
	if (!pawn) return;

	TreeInfoObject* infoObject = dynamic_cast<TreeInfoObject*>(subject);
	if (infoObject && infoObject == pawnInfoObject)
	{
		float x,y,z,h,p,r;
		x = (float)infoObject->getDoubleAttribute("pos X");
		y = (float)infoObject->getDoubleAttribute("pos Y");
		z = (float)infoObject->getDoubleAttribute("pos Z");

		h = (float)infoObject->getDoubleAttribute("rot X");
		p = (float)infoObject->getDoubleAttribute("rot Y");
		r = (float)infoObject->getDoubleAttribute("rot Z");
		pawn->set_world_offset(x,y,z,h,p,r);
		//updateWidget();
	}
}

AttributeItemWidget::AttributeItemWidget( DObject* object, int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type, DObserver* observerWindow /*= NULL*/ )
: TreeItemInfoWidget(x,y,w,h,name,inputItem,type)
{
	attrWindow = NULL;
	infoObject = object;
	if (!object)
		return;
	this->begin();
	attrWindow = new AttributeWindow(infoObject,x,y,w,h,name);
	attrWindow->begin();
	attrWindow->end();
	this->end();	
}

void AttributeItemWidget::updateWidget()
{
	redraw();
}

AttributeItemWidget::~AttributeItemWidget()
{
	
	delete attrWindow;
}
