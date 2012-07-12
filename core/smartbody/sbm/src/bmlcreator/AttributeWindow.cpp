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

#include "AttributeWindow.h"

#include "sb/SBObject.h"
#include <FL/Fl_Color_Chooser.H>
//#include <FL/Fl_Color.H>
//#include <FL/ask.h>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Float_Input.H>
#include <limits>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <sb/SBAttribute.h>
#include <sb/SBAttributeManager.h>
#include "TextEditor.h"

#ifndef WIN32
#define _strdup strdup
#endif


AttributeWindow::AttributeWindow(SmartBody::SBObject* obj, int x, int y, int w, int h, const char *s, bool upDownBox) : Fl_Group(x, y, w, h, s)
{
	//this->type(VERTICAL);

	setOffset(100);
	setObject(obj);
	this->begin();		
		mainGroup = new Fl_Scroll( x + 10,  y + 10, w -20, h -20);	
		//mainGroup->label(s);
		mainGroup->type(Fl_Scroll::VERTICAL_ALWAYS);
		if (upDownBox)
			mainGroup->box(FL_UP_BOX);
		mainGroup->begin();
		mainGroup->end();
	if (upDownBox)
		this->box(FL_DOWN_BOX);
	this->end();

	dirty = true;
}


AttributeWindow::~AttributeWindow()
{
	cleanUpAttributesInfo();
		
}

void AttributeWindow::setOffset(int pixels)
{
	_offset = pixels;
}

int AttributeWindow::getOffset()
{
	return _offset;
}

void AttributeWindow::cleanUpAttributesInfo()
{
	//LOG("AttributeWindow Destructor\n");
	// unregister the observer for each object attributes before destruction
	SmartBody::SBObject* obj = getObject();	
	//obj->getAttributeManager()->unregisterObserver(this);
	std::map<std::string, SmartBody::SBAttribute*>& attrList = obj->getAttributeList();
	std::map<std::string, SmartBody::SBAttribute*>::iterator ai;
	for ( ai  = attrList.begin();
		ai != attrList.end();
		ai++)
	{
		SmartBody::SBAttribute* attr = dynamic_cast<SmartBody::SBAttribute*>(ai->second);
		if (attr)
		{
			attr->unregisterObserver(this);
		}
	}
}

void AttributeWindow::setDirty(bool val)
{
	dirty = val;
	//relayout();
}
/*
void AttributeWindow::show()
{
	Fl_Pack::show();
}
*/

void AttributeWindow::setAttributeInfo(Fl_Widget* widget, SmartBody::SBAttributeInfo* attrInfo)
{
	// this is NOT an efficient way to change the status of the widget...
	// since it requires one check for each widget
	Fl_Group* group = dynamic_cast<Fl_Group*>(widget);
	if (group)
	{
		for (int c = 0; c < group->children(); c++)
			setAttributeInfo(group->child(c), attrInfo);
		//group->redraw();
		//return;
	}

	if (attrInfo->getReadOnly())
	{
		if (widget->active())
			widget->deactivate();
	}
	else
	{
		if (!widget->active())
			widget->activate();
	}

	if (attrInfo->getHidden())
	{
		if (widget->visible())
			widget->hide();
	}
	else
	{
		if (!widget->visible())
			widget->show();
	}

	if (attrInfo->getLocked())
	{
		if (widget->color() != FL_WHITE)
		{
			widget->color(FL_WHITE);
			widget->redraw();
		}
	}
	else
	{
		if (widget->color() != FL_GRAY)
		{
			widget->color(FL_GRAY);
			widget->redraw();
		}
	}
	
	if (attrInfo->getPriority())
	{
		// reorder widget
	}
	else
	{
		// reorder widget
	}
}

bool AttributePriorityPredicate(const SmartBody::SBAttribute* d1, const SmartBody::SBAttribute* d2)
{
	SmartBody::SBAttribute* a = const_cast<SmartBody::SBAttribute*>(d1);
	SmartBody::SBAttribute* b = const_cast<SmartBody::SBAttribute*>(d2);
	
	if (a->getAttributeInfo()->getGroup() == b->getAttributeInfo()->getGroup())
	{
		return a->getAttributeInfo()->getPriority() < b->getAttributeInfo()->getPriority();
	}
	else 
	{
		if (a->getAttributeInfo()->getGroup() && b->getAttributeInfo()->getGroup())
			return a->getAttributeInfo()->getGroup()->getPriority() < b->getAttributeInfo()->getGroup()->getPriority();
		else if (a->getAttributeInfo()->getGroup())
			return true;
		else
			return false;
	}
}

void AttributeWindow::reorderAttributes()
{
	std::vector<SmartBody::SBAttribute*> list;
	std::map<std::string, SmartBody::SBAttribute*>& attributes = object->getAttributeList();
	for (std::map<std::string, SmartBody::SBAttribute*>::iterator iter = attributes.begin();
		iter != attributes.end();
		iter++)
	{
		if (!iter->second->getAttributeInfo()->getHidden())
			list.push_back(iter->second);
	}

	std::sort(list.begin(), list.end(), AttributePriorityPredicate);
	for (std::vector<SmartBody::SBAttribute*>::iterator iter = list.begin();
		 iter != list.end();
		 iter++)
	{
		SmartBody::SBAttributeInfo* info = (*iter)->getAttributeInfo();
		int priority = info->getPriority();
		std::string name = (*iter)->getName();
	}

	int widgetHeight = 25;
	int startY = 10;

	int numOutOfOrder = 0;
	int childNum = 0;
	int numChildren = mainGroup->children();
	for (std::vector<SmartBody::SBAttribute*>::iterator sortedIter = list.begin();
		sortedIter != list.end();
		sortedIter++)
	{
		//LOG("Now writing attribute %s", (*sortedIter)->getName().c_str());
		// get the widget
		std::map<std::string, Fl_Widget*>::iterator mapIter = widgetMap.find((*sortedIter)->getName());
		if (mapIter != widgetMap.end())
		{
			Fl_Widget* widget = mapIter->second;
			if (mainGroup->child(childNum) != widget)
			{
				mainGroup->insert(*widget, childNum);
				numOutOfOrder++;
			}
			childNum++;
			/*
			int curY = widget->y();
			if (curY != startY) // make sure the widget is in the right place
			{
				widget->y(startY);
				numOutOfOrder++;
			}
			startY += widgetHeight;
			*/
		}
	}

	if (numOutOfOrder > 0)
	{
		this->dirty = true;
		this->damage(1);
	}
}

void AttributeWindow::draw()
{
	if (dirty) 
	{
		std::vector<SmartBody::SBAttributeGroup*>& attributeGroups = object->getAttributeManager()->getAttributeGroups();
		for (size_t g = 0; g < attributeGroups.size(); g++)
		{
		}


		int widgetHeight = 25;
		int startY = 10 + widgetHeight * widgetMap.size(); // start position for the next widget to be added
		std::map<std::string, SmartBody::SBAttribute*>& attributes = object->getAttributeList();
		
		// sort the attribute list by priority
		std::vector<SmartBody::SBAttribute*> sortedAttributes;
		for (std::map<std::string, SmartBody::SBAttribute*>::iterator iter = attributes.begin();
			iter != attributes.end();
			iter++)
		{
			if (!iter->second->getAttributeInfo()->getHidden())
				sortedAttributes.push_back(iter->second);
		}

		std::sort(sortedAttributes.begin(), sortedAttributes.end(), AttributePriorityPredicate);

		// compare the existing sorted attribute list to the widget list
		std::map<std::string, int> attributeStatus;
		for (std::map<std::string, Fl_Widget*>::iterator mapIter = widgetMap.begin();
			 mapIter != widgetMap.end();
			 mapIter++)
		{
			attributeStatus[mapIter->first] = 0;
		}
		
		for (std::vector<SmartBody::SBAttribute*>::iterator iter = sortedAttributes.begin();
			iter != sortedAttributes.end();
			iter++)
		{
			SmartBody::SBAttribute* attr = (*iter);
			std::string name = (*iter)->getName();
			std::map<std::string, Fl_Widget*>::iterator mapIter = widgetMap.find(name);
								
			if (mapIter == widgetMap.end()) // widget not found, create it
			{
				attributeStatus[name] = 2; // attribute widget created

				SmartBody::SBAttributeInfo* attrInfo = attr->getAttributeInfo();

				// action -> button
				SmartBody::ActionAttribute* actionAttr = dynamic_cast<SmartBody::ActionAttribute*>(attr);
				if (actionAttr) 
				{
					actionAttr->registerObserver(this);
					Fl_Button* button = new Fl_Button(mainGroup->x() + _offset, mainGroup->y() + startY, 40, 20, _strdup(name.c_str()));
					if (attrInfo->getDescription() != "")
						button->tooltip(_strdup(attrInfo->getDescription().c_str()));
					button->align(FL_ALIGN_LEFT);
					startY += widgetHeight; // make sure the next widget starts lower
					button->callback(ActionCB, this);
					setAttributeInfo(button, attrInfo);
					mainGroup->add(button);
					widgetMap[name] = button;
					reverseWidgetMap[button] = name;
					continue;
				}
				// bool -> checkbox
				SmartBody::BoolAttribute* boolAttr = dynamic_cast<SmartBody::BoolAttribute*>(attr);
				if (boolAttr) 
				{
					boolAttr->registerObserver(this);
					Fl_Check_Button* check = new Fl_Check_Button(mainGroup->x() + _offset, mainGroup->y() + startY, 150, 20, _strdup(name.c_str()));
					if (attrInfo->getDescription() != "")
						check->tooltip(_strdup(attrInfo->getDescription().c_str()));
					check->align(FL_ALIGN_LEFT);
					check->value(boolAttr->getValue());
					startY += widgetHeight; // make sure the next widget starts lower
					check->callback(BoolCB, this);
					setAttributeInfo(check, attrInfo);
					mainGroup->add(check);
					widgetMap[name] = check;
					reverseWidgetMap[check] = name;
					continue;
				}
				// int -> input
				SmartBody::IntAttribute* intAttr = dynamic_cast<SmartBody::IntAttribute*>(attr);
				if (intAttr) 
				{
					intAttr->registerObserver(this);
					Fl_Group* intGroup = new Fl_Group(mainGroup->x() + 0, mainGroup->y() + startY, 300, 20);
					intGroup->end();
					intGroup->align(FL_ALIGN_LEFT);
					Fl_Float_Input* input = new Fl_Float_Input(mainGroup->x() + _offset + 0, mainGroup->y() + startY, 60, 20, _strdup(name.c_str()));
					if (attrInfo->getDescription() != "")
						input->tooltip(_strdup(attrInfo->getDescription().c_str()));
					std::stringstream str;
					str << intAttr->getValue();
					input->value(str.str().c_str());
					input->callback(IntCB, this);
					intGroup->add(input);
					// show a wheel is there is no limit
					// show a slider if there is a limit
					if (intAttr->getMin() == -std::numeric_limits<int>::max() || 
						intAttr->getMax() == std::numeric_limits<int>::max())
					{
						Fl_Roller* wheel = new Fl_Roller(mainGroup->x() + _offset + 70, mainGroup->y() + startY, 100, 20);
						if (attrInfo->getDescription() != "")
							wheel->tooltip(_strdup(attrInfo->getDescription().c_str()));
						wheel->type(FL_HORIZONTAL);
						wheel->step(1);
						wheel->range(intAttr->getMin(), intAttr->getMax());
						wheel->value(intAttr->getValue());
						wheel->callback(IntCB, this);
						intGroup->add(wheel);
					}
					else
					{
						Fl_Slider* slider = new Fl_Slider(mainGroup->x() + 70, mainGroup->y() + startY, 100, 20);
						if (attrInfo->getDescription() != "")
							slider->tooltip(_strdup(attrInfo->getDescription().c_str()));
						//slider->type(Fl_Slider::Fl_Slider::LINEAR);
						slider->step(1.0);
						slider->range(intAttr->getMin(), intAttr->getMax());
						slider->value(intAttr->getValue());
						slider->callback(IntCB, this);
						intGroup->add(slider);
					}
					
					startY += widgetHeight; // make sure the next widget starts lower
					
					setAttributeInfo(input, attrInfo);
					mainGroup->add(intGroup);
					widgetMap[name] = intGroup;
					reverseWidgetMap[intGroup] = name;
					continue;
				}
				// double -> floatinput
				SmartBody::DoubleAttribute* doubleAttr = dynamic_cast<SmartBody::DoubleAttribute*>(attr);
				if (doubleAttr) 
				{
					doubleAttr->registerObserver(this);
					Fl_Group* doubleGroup = new Fl_Group(mainGroup->x() + 0, mainGroup->y() + startY, 300, 20);
					doubleGroup->end();
					//doubleGroup->flags(FL_ALIGN_LEFT);
					Fl_Float_Input* input = new Fl_Float_Input(mainGroup->x() +  _offset, mainGroup->y() + startY, 60, 20, _strdup(name.c_str()));
					if (attrInfo->getDescription() != "")
						input->tooltip(_strdup(attrInfo->getDescription().c_str()));
					std::stringstream strstr;
					strstr << doubleAttr->getValue();
					input->value(strstr.str().c_str());
					input->callback(DoubleCB, this);
					doubleGroup->add(input);
					// show a wheel is there is no limit
					// show a slider if there is a limit
					if (doubleAttr->getMin() == -std::numeric_limits<double>::max() ||
						doubleAttr->getMax() == std::numeric_limits<double>::max())
					{
						Fl_Roller* wheel = new Fl_Roller(mainGroup->x() + _offset + 70, mainGroup->y() + startY, 100, 20);
						if (attrInfo->getDescription() != "")
							wheel->tooltip(_strdup(attrInfo->getDescription().c_str()));
						wheel->type(FL_HORIZONTAL);
						wheel->step(.01);
						wheel->range(doubleAttr->getMin(), doubleAttr->getMax());
						wheel->value(doubleAttr->getValue());
						wheel->callback(DoubleCB, this);
						doubleGroup->add(wheel);
					}
					else
					{
						Fl_Slider* slider = new Fl_Slider(mainGroup->x() + 70, mainGroup->y() + startY, 100, 20);
						if (attrInfo->getDescription() != "")
							slider->tooltip(_strdup(attrInfo->getDescription().c_str()));
						//slider->type(FL_LINEAR);
						slider->step(.01);
						slider->range(doubleAttr->getMin(), doubleAttr->getMax());
						slider->value(doubleAttr->getValue());
						slider->callback(DoubleCB, this);
						doubleGroup->add(slider);
					}
					
					startY += widgetHeight; // make sure the next widget starts lower
					
					setAttributeInfo(doubleGroup, attrInfo);
					mainGroup->add(doubleGroup);
					widgetMap[name] = doubleGroup;
					reverseWidgetMap[doubleGroup] = name;
					continue;
				}
				// string -> input, or string -> choice
				SmartBody::StringAttribute* stringAttr = dynamic_cast<SmartBody::StringAttribute*>(attr);
				if (stringAttr) 
				{
					stringAttr->registerObserver(this);
					if (stringAttr->getValidValues().size() == 0)
					{
						Fl_Input* input = new Fl_Input(mainGroup->x() + _offset, mainGroup->y() + startY, 150, 20, _strdup(name.c_str()));
						Fl_Button* edit = new Fl_Button(mainGroup->x() + _offset + 160, mainGroup->y() + startY, 20, 20, ".");
						edit->tooltip(stringAttr->getName().c_str());
						edit->callback(EditStringCB, stringAttr);
						if (attrInfo->getDescription() != "")
							input->tooltip(_strdup(attrInfo->getDescription().c_str()));
						input->value(stringAttr->getValue().c_str());
						startY += widgetHeight; // make sure the next widget starts lower
						input->callback(StringCB, this);
						mainGroup->add(input);
						mainGroup->add(edit);
						widgetMap[name] = input;
						reverseWidgetMap[input] = name;
					}
					else
					{
						Fl_Choice* choice = new Fl_Choice(mainGroup->x() + _offset, mainGroup->y() + startY, 150, 20, _strdup(name.c_str()));
						if (attrInfo->getDescription() != "")
							choice->tooltip(_strdup(attrInfo->getDescription().c_str()));
						// add all the options
						choice->add("-----");
						const std::vector<std::string>& values = stringAttr->getValidValues();
						int selected = -1;
						for (size_t i = 0; i < values.size(); i++)
						{
							addChoice(choice, values[i]);
							if (stringAttr->getValue() == values[i])
								choice->value(i + 1);
						}
						if (stringAttr->getValue() == "")
							choice->value(0);
						startY += widgetHeight; // make sure the next widget starts lower
						choice->callback(StringCB, this);
						mainGroup->add(choice);
						widgetMap[name] = choice;
						reverseWidgetMap[choice] = name;
					}
					continue;
				}
				// vec3 -> 3 float inputs
				SmartBody::Vec3Attribute* vec3Attr = dynamic_cast<SmartBody::Vec3Attribute*>(attr);
				if (vec3Attr) 
				{
					vec3Attr->registerObserver(this);
					Fl_Group* vec3Group = new Fl_Group(mainGroup->x() + _offset, mainGroup->y() + startY, 200, 20, _strdup(name.c_str()));
					vec3Group->align(FL_ALIGN_LEFT);
					vec3Group->end();
					if (attrInfo->getDescription() != "")
						vec3Group->tooltip(_strdup(attrInfo->getDescription().c_str()));
					//vec3Group->flags(FL_ALIGN_LEFT);
					SrVec val = vec3Attr->getValue();
					for (int c = 0; c < 3; c++)
					{
						Fl_Float_Input* finput = new Fl_Float_Input(mainGroup->x() + _offset + c * 50, mainGroup->y() +  startY, 60, 20);
						if (attrInfo->getDescription() != "")
							finput->tooltip(_strdup(attrInfo->getDescription().c_str()));
						std::stringstream strstr;
						strstr << val[c];
						finput->value(strstr.str().c_str());
						finput->callback(Vec3CB, this);
						vec3Group->add(finput);
					}						
					startY += widgetHeight; // make sure the next widget starts lower
					setAttributeInfo(vec3Group, attrInfo);
					mainGroup->add(vec3Group);
					widgetMap[name] = vec3Group;
					reverseWidgetMap[vec3Group] = name;
					continue;
				}
			}
			else
			{
				attributeStatus[mapIter->first] = 1; // attribute widget found

				// check the string widgets to make sure that the valid values haven't changes
				SmartBody::StringAttribute* stringAttr = dynamic_cast<SmartBody::StringAttribute*>(attr);
				if (stringAttr) 
				{
					const std::vector<std::string>& validValues = stringAttr->getValidValues();
					if (validValues.size() > 0)
					{
						bool resetChoiceList = false;
						Fl_Choice* choice = dynamic_cast<Fl_Choice*>(mapIter->second);
						if (choice)
						{
							if (validValues.size() != choice->size())
							{
								resetChoiceList = true;
							}
							else 
							{
								for (int c = 0; c < choice->size(); c++)
								{
									if (validValues[c] != choice->menu()[c].label())
									{
										resetChoiceList = true;
										break;
									}
								}
							}
							if (resetChoiceList)
							{
								for (int c = choice->size() - 1; c >= 0; c--)
								{
									choice->remove(c);
								}
								choice->add("-----");
								for (size_t i = 0; i < validValues.size(); i++)
								{
									addChoice(choice, validValues[i]);
									if (stringAttr->getValue() == validValues[i])
										choice->value(i + 1);
								}
								if (stringAttr->getValue() == "")
									choice->value(0);
								choice->callback(StringCB, this); // is this necessary?
							}
						}
					}
				}
			}
		}

		// remove any unused attributes
		for (std::map<std::string, Fl_Widget*>::iterator mapIter = widgetMap.begin();
			 mapIter != widgetMap.end();
			 mapIter++)
		{
			if (attributeStatus[mapIter->first] == 0) 
			{
				mainGroup->remove(mapIter->second);
			}
		}

		reorderAttributes();
		dirty = false;
	}

	Fl_Group::draw();
}

void AttributeWindow::EditStringCB(Fl_Widget* w, void *data)
{
	SmartBody::StringAttribute* sa = (SmartBody::StringAttribute*) data;
	TextEditorWindow* textEditor = new TextEditorWindow(w->window()->x() + 10, w->window()->y() + 10, 400, 360, "Edit Text");
	textEditor->setSBStringAttribute(sa);
	textEditor->show();
}

SmartBody::SBObject* AttributeWindow::getObject()
{
	return object;
}

void AttributeWindow::setObject(SmartBody::SBObject* g)
{
	object = g;
	object->getAttributeManager()->registerObserver(this);
}

void AttributeWindow::ActionCB(Fl_Widget *w, void *data)
{
	AttributeWindow *cw = (AttributeWindow*) data;
	
	Fl_Button* check = (Fl_Button*) w;
	SmartBody::SBObject* obj = cw->getObject();
	
	std::string name = "";
	std::map<Fl_Widget*, std::string>::iterator iter = cw->reverseWidgetMap.find(w);
	if (iter !=  cw->reverseWidgetMap.end())
		name = iter->second;
		
	if (name != "")
	{
		SmartBody::SBAttribute* attr = obj->getAttribute(name);
		SmartBody::ActionAttribute* aattr = dynamic_cast<SmartBody::ActionAttribute*>(attr);
		if (aattr)
		{
			aattr->setValue();
		}
		else
		{
			LOG("Attribute with name %s.%s is not an action attribute. Please check code.", obj->getName().c_str(), check->label());
		}
	}
	else
	{
		LOG("Attribute with name %s.%s is no longer present.", obj->getName().c_str(), check->label());
	}
}

void AttributeWindow::BoolCB(Fl_Widget *w, void *data)
{
	AttributeWindow *cw = (AttributeWindow*) data;
	
	Fl_Check_Button* check = (Fl_Check_Button*) w;
	SmartBody::SBObject* obj = cw->getObject();
	
	std::string name = "";
	std::map<Fl_Widget*, std::string>::iterator iter = cw->reverseWidgetMap.find(w);
	if (iter !=  cw->reverseWidgetMap.end())
		name = iter->second;
		
	if (name != "")
	{
		SmartBody::SBAttribute* attr = obj->getAttribute(name);
		SmartBody::BoolAttribute* battr = dynamic_cast<SmartBody::BoolAttribute*>(attr);
		if (battr)
		{
			battr->setValue(check->value()? true : false);
		}
		else
		{
			LOG("Attribute with name %s.%s is not a boolean attribute. Please check code.", obj->getName().c_str(), check->label());
		}
	}
	else
	{
		LOG("Attribute with name %s.%s is no longer present.", obj->getName().c_str(), check->label());
	}
}

void AttributeWindow::IntCB(Fl_Widget *w, void *data)
{
	AttributeWindow *attrWin = (AttributeWindow*) data;
	
	Fl_Group* intGroup = w->parent();

	Fl_Float_Input* input = dynamic_cast<Fl_Float_Input*>(intGroup->child(0));
	Fl_Roller* wheel = dynamic_cast<Fl_Roller*>(intGroup->child(1));
	Fl_Slider* slider = dynamic_cast<Fl_Slider*>(intGroup->child(1));
				
	// get the name of the attribute
	std::string name = "";
	std::map<Fl_Widget*, std::string>::iterator iter = attrWin->reverseWidgetMap.find(intGroup);
	if (iter !=  attrWin->reverseWidgetMap.end())
		name = iter->second;

	SmartBody::SBObject* obj = attrWin->getObject();
	
	if (name != "")
	{
		SmartBody::SBAttribute* attr = obj->getAttribute(name);
		SmartBody::IntAttribute* iattr = dynamic_cast<SmartBody::IntAttribute*>(attr);
		if (iattr)
		{
			// where did the input come from? The input, the wheel or the slider?
			int val = 0;
			if (w == input)
			{
				val = atoi(input->value());
			}
			else if (w == wheel)
			{
				val = int(wheel->value());
			}
			else if (w == slider)
			{
				val = int(slider->value());
			}
			iattr->setValue(val);
		}
		else
		{
		}
	}
	else
	{
		LOG("Attribute with name %s.%s is no longer present.", obj->getName().c_str(), input->label());
	}
}

void AttributeWindow::DoubleCB(Fl_Widget *w, void *data)
{
	AttributeWindow *attrWin = (AttributeWindow*) data;
	
	Fl_Group* doubleGroup = w->parent();

	Fl_Float_Input* input = dynamic_cast<Fl_Float_Input*>(doubleGroup->child(0));
	Fl_Roller* wheel = dynamic_cast<Fl_Roller*>(doubleGroup->child(1));
	Fl_Slider* slider = dynamic_cast<Fl_Slider*>(doubleGroup->child(1));
				
	// get the name of the attribute
	std::string name = "";
	std::map<Fl_Widget*, std::string>::iterator iter = attrWin->reverseWidgetMap.find(doubleGroup);
	if (iter !=  attrWin->reverseWidgetMap.end())
		name = iter->second;

	SmartBody::SBObject* obj = attrWin->getObject();
	
	if (name != "")
	{
		SmartBody::SBAttribute* attr = obj->getAttribute(name);
		SmartBody::DoubleAttribute* dattr = dynamic_cast<SmartBody::DoubleAttribute*>(attr);
		if (dattr)
		{
			// where did the input come from? The input, the wheel or the slider?
			double val = 0;
			if (w == input)
			{
				val = atof(input->value());
			}
			else if (w == wheel)
			{
				val = wheel->value();
			}
			else if (w == slider)
			{
				val = slider->value();
			}
			dattr->setValue(val);
		}
		else
		{
			LOG("Attribute with name %s.%s is not a double attribute. Please check code.", obj->getName().c_str(), input->label());
		}
	}
	else
	{
		LOG("Attribute with name %s.%s is no longer present.", obj->getName().c_str(), input->label());
	}
}

void AttributeWindow::StringCB(Fl_Widget *w, void *data)
{
	AttributeWindow *cw = (AttributeWindow*) data;
	
	Fl_Input* input = dynamic_cast<Fl_Input*>(w);
	Fl_Choice* choice = dynamic_cast<Fl_Choice*>(w);

	SmartBody::SBObject* obj = cw->getObject();
	
	// get the name of the attribute
	std::string name = "";
	std::map<Fl_Widget*, std::string>::iterator iter = cw->reverseWidgetMap.find(w);
	if (iter !=  cw->reverseWidgetMap.end())
		name = iter->second;

	if (name != "")
	{
		SmartBody::SBAttribute* attr = obj->getAttribute(name);
		SmartBody::StringAttribute* strattr = dynamic_cast<SmartBody::StringAttribute*>(attr);
		if (strattr)
		{
			if (input)
			{
				strattr->setValue(input->value());
			}
			else
			{
				std::string choiceValue = choice->menu()[choice->value()].label();
				if (choiceValue == "-----")
					choiceValue = "";
				strattr->setValue(choiceValue);
			}
		}
		else
		{
			LOG("Attribute with name %s.%s is not a string attribute. Please check code.", obj->getName().c_str(), input->label());
		}
	}
	else
	{
		LOG("Attribute with name %s.%s is no longer present.", obj->getName().c_str(), input->label());
	}
}

void AttributeWindow::Vec3CB(Fl_Widget *w, void *data)
{
	AttributeWindow *attrWin = (AttributeWindow*) data;
	
	Fl_Float_Input* finput = (Fl_Float_Input*) w;
	Fl_Group* group = finput->parent();
	SmartBody::SBObject* obj = attrWin->getObject();
	
	// get the name of the attribute
	std::string name = "";
	std::map<Fl_Widget*, std::string>::iterator iter = attrWin->reverseWidgetMap.find(group);
	if (iter !=  attrWin->reverseWidgetMap.end())
		name = iter->second;

	if (name != "")
	{
		SmartBody::SBAttribute* attr = obj->getAttribute(name);
		SmartBody::Vec3Attribute* vec3attr = dynamic_cast<SmartBody::Vec3Attribute*>(attr);
		if (vec3attr)
		{
			// get all the children of the group
			SrVec val;
			Fl_Float_Input* finput[3];
			for (int x = 0; x < 3; x++)
			{
				finput[x] = (Fl_Float_Input*) group->child(x);
				val[x] = (float) atof(finput[x]->value());
			}
			vec3attr->setValue(val);
		}
		else
		{
			LOG("Attribute with name %s.%s is not a vec3 attribute. Please check code.", obj->getName().c_str(), group->label());
		}
	}
	else
	{
		LOG("Attribute with name %s.%s is no longer present.", obj->getName().c_str(), group->label());
	}
}

void AttributeWindow::notify(SmartBody::SBSubject* subject)
{
	if (subject == this->getObject()->getAttributeManager())
	{
		setDirty(true);
		//relayout();
		return;
	}

	// assume that the notification came from a change in attribute value 
	// directly from the attribute
	SmartBody::SBAttribute* attr = dynamic_cast<SmartBody::SBAttribute*>(subject);
	if (attr)
	{
		SmartBody::SBAttributeInfo* attrInfo = attr->getAttributeInfo();

		Fl_Widget* widget = NULL;
		// make sure that the attribute exists in the attribute map
		std::map<std::string, Fl_Widget*>::iterator iter = widgetMap.find(attr->getName());
		if (iter == widgetMap.end())
		{
			LOG("Widget for attribute %s.%s was not found. Please check code.", this->object->getName().c_str(), attr->getName().c_str());
			return;
		}
		SmartBody::ActionAttribute* aattr = dynamic_cast<SmartBody::ActionAttribute*>(attr);
		if (aattr)
		{
			Fl_Button* button = (Fl_Button*) iter->second;
			button->value();
			setAttributeInfo(button, attrInfo);
			return;
		}

		SmartBody::BoolAttribute* battr = dynamic_cast<SmartBody::BoolAttribute*>(attr);
		if (battr)
		{
			Fl_Check_Button* check = (Fl_Check_Button*) iter->second;
			check->value(battr->getValue());
			setAttributeInfo(check, attrInfo);
			return;
		}
		SmartBody::IntAttribute* iattr = dynamic_cast<SmartBody::IntAttribute*>(attr);
		if (iattr)
		{
			Fl_Group* group = (Fl_Group*) iter->second;
			int val = iattr->getValue();
			Fl_Float_Input* input = (Fl_Float_Input*) group->child(0);
			std::stringstream strstr;
			strstr << val;
			input->value(strstr.str().c_str());
			Fl_Roller* wheel = dynamic_cast<Fl_Roller*>( group->child(1));
			Fl_Slider* slider = dynamic_cast<Fl_Slider*>( group->child(1));
			if (wheel)
			{
				wheel->value(val);
			}
			else if (slider)
			{
				slider->value(val);
			}
			// did the range so as to cause a widget change type?
			if (wheel && 
				(iattr->getMin() != -std::numeric_limits<int>::max() && 
				 iattr->getMax() != std::numeric_limits<int>::max()))
			{
				// change the wheel to a slider
				int xpos = wheel->x();
				int ypos = wheel->y();
				int width = wheel->w();
				int height = wheel->h();
				Fl_Slider* slider = new Fl_Slider(xpos, ypos, width, height);
				if (attrInfo->getDescription() != "")
					slider->tooltip(_strdup(attrInfo->getDescription().c_str()));
				//slider->type(FL_LINEAR);
				slider->range(float(iattr->getMin()), float(iattr->getMax()));
				slider->step(1);
				slider->value(val);
				slider->callback(IntCB, this);
				group->remove(wheel);
				group->add(slider);
				delete wheel;
			}
			else if (wheel && 
					 (wheel->minimum() != iattr->getMin() &&
					  wheel->maximum() != iattr->getMax()))
			{
				wheel->range(float(iattr->getMin()), float(iattr->getMax()));
			}
			else if (slider && 
				(iattr->getMin() == -std::numeric_limits<int>::max() && 
				 iattr->getMax() == std::numeric_limits<int>::max()))
			{
				// change the slider to a wheel
				int xpos = slider->x();
				int ypos = slider->y();
				int width = slider->w();
				int height = slider->h();
				Fl_Roller* wheel = new Fl_Roller(xpos, ypos, width, height);
				if (attrInfo->getDescription() != "")
					wheel->tooltip(_strdup(attrInfo->getDescription().c_str()));
				wheel->type(FL_HORIZONTAL);
				wheel->range(float(iattr->getMin()), float(iattr->getMax()));
				wheel->step(1);
				wheel->value(val);
				wheel->callback(IntCB, this);
				group->remove(slider);
				group->add(wheel);
				delete slider;
			}
			else if (slider && 
					 (slider->minimum() != iattr->getMin() &&
					  slider->maximum() != iattr->getMax()))
			{
				slider->range(float(iattr->getMin()), float(iattr->getMax()));
			}

			setAttributeInfo(group, attrInfo);
			
			return;
		}
		SmartBody::DoubleAttribute* dattr = dynamic_cast<SmartBody::DoubleAttribute*>(attr);
		if (dattr)
		{
			Fl_Group* group = (Fl_Group*) iter->second;
			double val = dattr->getValue();
			Fl_Float_Input* input = (Fl_Float_Input*) group->child(0);
			std::stringstream strstr;
			strstr << val;
			input->value(strstr.str().c_str());
			Fl_Roller* wheel = dynamic_cast<Fl_Roller*>(group->child(1));
			Fl_Slider* slider = dynamic_cast<Fl_Slider*>(group->child(1));
			if (wheel)
			{
				wheel->value(val);
			}
			else if (slider)
			{
				slider->value(val);
			}
			// did the range so as to cause a widget change type?
			if (wheel && 
				(dattr->getMin() != -std::numeric_limits<double>::max() && 
				 dattr->getMax() != std::numeric_limits<double>::max()))
			{
				// change the wheel to a slider
				int xpos = wheel->x();
				int ypos = wheel->y();
				int width = wheel->w();
				int height = wheel->h();
				Fl_Slider* slider = new Fl_Slider(xpos, ypos, width, height);
				if (attrInfo->getDescription() != "")
					slider->tooltip(_strdup(attrInfo->getDescription().c_str()));
				//slider->type(Slider::LINEAR);
				slider->range(dattr->getMin(), dattr->getMax());
				slider->step(.01);
				slider->value(val);
				slider->callback(DoubleCB, this);
				group->remove(wheel);
				group->add(slider);
				delete wheel;
			}
			else if (wheel && 
					 (wheel->minimum() != dattr->getMin() &&
					  wheel->maximum() != dattr->getMax()))
			{
				wheel->range(dattr->getMin(), dattr->getMax());
			}
			else if (slider && 
				(dattr->getMin() == -std::numeric_limits<double>::max() && 
				 dattr->getMax() == std::numeric_limits<double>::max()))
			{
				// change the slider to a wheel
				int xpos = slider->x();
				int ypos = slider->y();
				int width = slider->w();
				int height = slider->h();
				Fl_Roller* wheel = new Fl_Roller(xpos, ypos, width, height);
				if (attrInfo->getDescription() != "")
					wheel->tooltip(_strdup(attrInfo->getDescription().c_str()));
				wheel->type(FL_HORIZONTAL);
				wheel->range(dattr->getMin(), dattr->getMax());
				wheel->step(.01);
				wheel->value(val);
				wheel->callback(DoubleCB, this);
				group->remove(slider);
				group->add(wheel);
				delete slider;
			}
			else if (slider && 
					 (slider->minimum() != dattr->getMin() &&
					  slider->maximum() != dattr->getMax()))	
			{
				slider->range(dattr->getMin(), dattr->getMax());
			}
			
			setAttributeInfo(group, attrInfo);

			return;
		}
		SmartBody::StringAttribute* sattr = dynamic_cast<SmartBody::StringAttribute*>(attr);
		if (sattr)
		{
			const std::vector<std::string>& validValues = sattr->getValidValues();
			if (validValues.size() == 0)
			{

				Fl_Input* input = dynamic_cast<Fl_Input*>(iter->second);
				if (!input)
				{ // widget used to be a list of values, so erase that widget and have it remade automatically
					// remove the widget from the attribute window
					mainGroup->remove(*(iter->second));
					// remove the widget from the attribute map
					widgetMap.erase(iter);
					dirty = true;
					return;
				}
				input->value(sattr->getValue().c_str());
 				setAttributeInfo(input, attrInfo);
			}
			else
			{
				Fl_Choice* choice = dynamic_cast<Fl_Choice*>(iter->second);
				if (!choice)
				{ // widget used to be an input, so erase that widget and have it remade automatically
					// remove the widget from the attribute window
					mainGroup->remove(*(iter->second));
					// remove the widget from the attribute map
					widgetMap.erase(iter);
					dirty = true;
					return;
				}
				// check to see if the choice list needs to be reset
				bool resetChoiceList = false;
				const std::vector<std::string>& validValues = sattr->getValidValues();
				if (validValues.size() != choice->size())
				{
					resetChoiceList = true;
				}
				else
				{
					for (int c = 0; c < choice->size(); c++)
					{
						if (validValues[c] != choice->menu()[c].label())
						{
							resetChoiceList = true;
							break;
						}
					}
				}
				if (resetChoiceList)
				{
					dirty = true;
					choice->clear();
					choice->add("-----");
					const std::vector<std::string>& values = sattr->getValidValues();
					int selected = -1;
					for (size_t i = 0; i < values.size(); i++)
					{
						addChoice(choice, values[i]);
						if (sattr->getValue() == values[i])
							choice->value(i + 1);
					}
					if (sattr->getValue() == "")
						choice->value(0);
				}
				if (sattr->getValue() == "")
				{
					choice->value(0);
				}
				else
				{
					for (int c = 1; c < choice->size(); c++)
					{
						if (sattr->getValue() == choice->menu()[c].label())
						{
							choice->value(c);
							break;
						}
					}
				}
				setAttributeInfo(choice, attrInfo);
			}
			return;
		}
		SmartBody::Vec3Attribute* vec3attr = dynamic_cast<SmartBody::Vec3Attribute*>(attr);
		if (vec3attr)
		{
			Fl_Group* group = (Fl_Group*) iter->second;
			SrVec val = vec3attr->getValue();
			Fl_Float_Input* finput[3];
			for (int x = 0; x < 3; x++)
			{
				finput[x] = (Fl_Float_Input*) group->child(x);
				char buff[64];
				sprintf(buff, "%f", val[x]);
				finput[x]->value(buff);
			}
			setAttributeInfo(group, attrInfo);
			return;
		}
		
	}
	else
	{
		// non-attribute ?
		LOG("AttributeWindow for object %s got a notify() with a non-attribute.", this->object->getName().c_str());
	}
}

void AttributeWindow::cleanUpWidgets()
{
	cleanUpAttributesInfo();
	std::map<std::string, Fl_Widget*>::iterator mi;
	for ( mi  = widgetMap.begin();
		  mi != widgetMap.end();
		  mi++ )
	{
		mainGroup->remove(mi->second);
	}
	widgetMap.clear();
	reverseWidgetMap.clear();	
}

void AttributeWindow::addChoice(Fl_Choice* choice, const std::string& val)
{
	// if a choice includes a slash character, then it will automatically 
	// create a submenu. To get around this, we need to first add the choice item,
	// then later rename it with the slash character.
	int pos = val.find_first_of("/");
	if (pos == std::string::npos)
	{
		choice->add(val.c_str());
		return;
	}

	int i = choice->add("xxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	Fl_Menu_Item* m = (Fl_Menu_Item*) choice->menu();
	m[i].label(val.c_str());
}