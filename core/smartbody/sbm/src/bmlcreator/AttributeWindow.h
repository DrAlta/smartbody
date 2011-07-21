/**************************************************
Copyright 2005 by Ari Shapiro and Petros Faloutsos

DANCE
Dynamic ANimation and Control Environment

 ***************************************************************
 ******General License Agreement and Lack of Warranty ***********
 ****************************************************************

This software is distributed for noncommercial use in the hope that it will 
be useful but WITHOUT ANY WARRANTY. The author(s) do not accept responsibility
to anyone for the consequences	of using it or for whether it serves any 
particular purpose or works at all. No warranty is made about the software 
or its performance. Commercial use is prohibited. 

Any plugin code written for DANCE belongs to the developer of that plugin,
who is free to license that code in any manner desired.

Content and code development by third parties (such as FLTK, Python, 
ImageMagick, ODE) may be governed by different licenses.
You may modify and distribute this software as long as you give credit 
to the original authors by including the following text in every file 
that is distributed: */

/*********************************************************
	Copyright 2005 by Ari Shapiro and Petros Faloutsos

	DANCE
	Dynamic ANimation and Control Environment
	-----------------------------------------
	AUTHOR:
		Ari Shapiro (ashapiro@cs.ucla.edu)
	ORIGINAL AUTHORS: 
		Victor Ng (victorng@dgp.toronto.edu)
		Petros Faloutsos (pfal@cs.ucla.edu)
	CONTRIBUTORS:
		Yong Cao (abingcao@cs.ucla.edu)
		Paco Abad (fjabad@dsic.upv.es)
**********************************************************/

#ifndef _ATTRIBUTEWINDOW_
#define _ATTRIBUTEWINDOW_

#include <FL/Fl_Scroll.H>
#include "vhcl.h"
#include "sbm/DObserver.h"
#include "sbm/DAttribute.h"
#include <map>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>


class DObject;

class AttributeWindow : public Fl_Group, public DObserver
{
public:
	AttributeWindow(DObject*, int,int,int,int,const char*);
	~AttributeWindow();

	void setDirty(bool val);
	void draw();
	//void show();
	//void layout();

	void setObject(DObject* g);
	DObject* getObject();
	
	void setAttributeInfo(Fl_Widget* widget, DAttributeInfo* attrInfo);
	void reorderAttributes();
	void cleanUpAttributesInfo();
	void cleanUpWidgets();
	virtual void notify(DSubject* subject);

	static void BoolCB(Fl_Widget* w, void *data);
	static void IntCB(Fl_Widget* w, void *data);
	static void DoubleCB(Fl_Widget* w, void *data);
	static void StringCB(Fl_Widget* w, void *data);
	static void Vec3CB(Fl_Widget* w, void *data);

	static const uchar ATTRIBUTEWINDOWTYPE = (uchar)240;

	DObject* object;

	bool dirty;
	std::map<std::string, Fl_Widget*> widgetMap;
	std::map<Fl_Widget*, std::string> reverseWidgetMap;
	Fl_Scroll* mainGroup;
};

#endif
