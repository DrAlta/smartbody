#pragma once
#include "Fl_Tree_Horizontal.h"
#include "TreeInfoObject.h"
#include <bmlcreator/AttributeWindow.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Tabs.H>
#include <sk/sk_skeleton.h>

class TreeItemInfoWidget : public Fl_Group
{
public:
	TreeItemInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type);
protected:
	Fl_Tree_Item* treeItem;
	int           itemType;
public:
	virtual void updateWidget() {};	
};


class SkeletonItemInfoWidget : public TreeItemInfoWidget
{
protected:
	Fl_Tree*    skeletonTree;
	SkSkeleton* itemSkeleton;
	AttributeWindow* attrWindow;
	TreeInfoObject*  jointInfoObject;
	std::string      curJointName;
	std::string      skeletonName;
	std::string      charName;
public:
	SkeletonItemInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type);	
	virtual void updateWidget();
	void updateJointAttributes(std::string jointName);
	static void treeCallBack(Fl_Widget* widget, void* data);
protected:
	void updateSkeletonTree(Fl_Tree_Item* root, SkSkeleton* skel);
	void updateJointTree(Fl_Tree_Item* root, SkJoint* node);
};

class MotionItemInfoWidget : public TreeItemInfoWidget
{
protected:
	Fl_Value_Slider* frameSlider;
	Fl_Hold_Browser* channelBrowser;
	AttributeWindow* attrWindow;
	TreeInfoObject*  channelInfoObject;
	std::string      curChannelName;
	std::string      motionName;
	int              motionFrame;
	int              channelIndex;
public:
	MotionItemInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type);	
	virtual void updateWidget();
	void updateChannelAttributes();
	void setMotionFrame(int frame) { motionFrame = frame; }
	void setChannelIndex(int index) { channelIndex = index; }
	static void browserCallBack(Fl_Widget* widget, void* data);
	static void sliderCallBack(Fl_Widget* widget, void* data);
};

class PathItemInfoWidget : public TreeItemInfoWidget
{
protected:
	Fl_File_Chooser* pathChooser;
public:
	PathItemInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type);	
	static void addDirectoryCallback(Fl_Widget* widget, void* data);
	void addDirectory(const char* dirName );
protected:
	std::string getTypeParameter(int type);
};

class SeqItemInfoWidget : public TreeItemInfoWidget
{
protected:	
	Fl_Text_Display*	textDisplay;
	Fl_Text_Buffer*	    textBuffer;
	std::string         seqFilename;
	std::string         seqFullPathName;
	Fl_Button*          runSeqButton;
	Fl_Button*          editSeqButton;
public:
	SeqItemInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type);
	static void runSeqCallback(Fl_Widget* widget, void* data);
	static void editSeqCallback(Fl_Widget* widget, void* data);
	virtual void updateWidget();
	std::string getSeqFile() { return seqFilename; }
	std::string getSeqFullPathName() { return seqFullPathName; }
};


class EventItemInfoWidget : public TreeItemInfoWidget, public SmartBody::SBObserver
{
protected:	
	AttributeWindow* attrWindow;
	TreeInfoObject* eventInfoObject;
	std::string     eventName;
public:
	EventItemInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type);	
	static void addEventCallback(Fl_Widget* widget, void* data);
	void addNewEvent();
	void removeEvent();
	virtual void updateWidget();
	virtual void notify(SmartBody::SBSubject* subject);
};



class PawnItemInfoWidget : public TreeItemInfoWidget, public SmartBody::SBObserver
{
public:
	PawnItemInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type, SBObserver* observerWindow = NULL);	
	virtual void updateWidget();
	virtual void notify(SmartBody::SBSubject* subject);
protected:
	AttributeWindow* attrWindow;
	TreeInfoObject* pawnInfoObject;
	std::string pawnName;	
};

class AttributeItemWidget : public TreeItemInfoWidget
{
protected:
	AttributeWindow* attrWindow;
	SmartBody::SBObject*         infoObject;
public:
	AttributeItemWidget(SmartBody::SBObject* object, int x, int y, int w, int h, const char* name, Fl_Tree_Item* inputItem, int type, SmartBody::SBObserver* observerWindow = NULL);
	~AttributeItemWidget();

	virtual void updateWidget();
};

class DoubleAttributeItemWidget : public TreeItemInfoWidget
{
protected:
	AttributeWindow* attrWindow1, *attrWindow2;
	SmartBody::SBObject*         infoObject1;
	SmartBody::SBObject*         infoObject2;
public:
	DoubleAttributeItemWidget(SmartBody::SBObject* object1, SmartBody::SBObject* object2, int x, int y, int w, int h, int ySep, const char* name1, const char* name2, Fl_Tree_Item* inputItem, int type, SmartBody::SBObserver* observerWindow = NULL);
	~DoubleAttributeItemWidget();

	virtual void updateWidget();
};

class MultiAttributeItemWidget : public TreeItemInfoWidget
{
protected:
	//AttributeWindow* attrWindow1, *attrWindow2;
	Fl_Tabs* mainGroup;
	std::vector<AttributeWindow*> attrWindowList;
	std::vector<SmartBody::SBObject*> infoObjectList;	
	std::vector<std::string> attrNameList;
public:
	MultiAttributeItemWidget(std::vector<SmartBody::SBObject*>& objectList, int x, int y, int w, int h, int yStep, const char* name, std::vector<std::string>& objectNameList, Fl_Tree_Item* inputItem, int type, SmartBody::SBObserver* observerWindow /*= NULL*/ );
	~MultiAttributeItemWidget();

	virtual void updateWidget();
};