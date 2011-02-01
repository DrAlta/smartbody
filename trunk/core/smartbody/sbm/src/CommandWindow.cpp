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

#include "CommandWindow.h"
#include <vector>

#include <sys/stat.h> 
#include <sys/types.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <fltk/events.h>
#include <sbm/mcontrol_util.h>
#include <boost/algorithm/string/replace.hpp>

using namespace fltk;
using namespace std;

void updateDisplay(int pos, int nInserted, int nDeleted,int b, const char* d, void* v);

//creating FLTK multiline editor to connect to the interpreter
CommandWindow::CommandWindow(int x,int y,int w,int h, const char* s) : Window(x, y, w, h, s)
{
       curDir[0] = '\0';

	this->begin();

	menubar = new MenuBar(0, 0, w, 25); 
	menubar->add("&Edit/&Clear",NULL, clearcb, 0);
	menubar->add("&Edit/C&lear History",NULL, clearhistorycb, 0);

	historyCounter = 0;
	historyLocation = 0;
	this->color(fltk::GRAY75);
	strcpy(printout,"");

	int yDis = 25;
	
	// output
	textDisplay = new TextDisplay(10, yDis, w - 20, 250, ""); yDis += 250;
	textDisplay->box(fltk::UP_BOX);
	textDisplay->textfont(COURIER);
#ifdef _MACOSX_
	textDisplay->textsize(10);
#else
	textDisplay->textsize(11);
#endif
	textDisplay->color(fltk::GRAY75);
	textDisplay->textcolor(BLACK);
	textDisplay->redraw() ;

	DisplayTextBuffer = new TextBuffer();
	DisplayTextBuffer->add_modify_callback(updateDisplay, this);
	textDisplay->buffer(DisplayTextBuffer);


	yDis += 25;

	// command input
	tabGroup = new TabGroup(10, yDis, w - 10, h - yDis);
	tabGroup->begin();
	for (int i = 0; i < 2; i++)
	{
		textEditor[i] = new TextEditor(0, 0, w - 20, h - yDis - 30);  
		textEditor[i]->textfont(COURIER);
	#ifdef _MACOSX_
		textEditor[i]->textsize(10);
	#else
		textEditor[i]->textsize(13);
	#endif
		textEditor[i]->box(fltk::UP_BOX);
		textEditor[i]->textcolor(fltk::BLACK);
		textBuffer[i] = new TextBuffer();
		textEditor[i]->buffer(textBuffer[i]); 
		textEditor[i]->add_key_binding(fltk::UpKey, fltk::event_state(), (TextEditor::Key_Func) upcb);
		textEditor[i]->add_key_binding(fltk::DownKey, fltk::event_state(), (TextEditor::Key_Func) downcb);

		textEditor[i]->add_key_binding(fltk::TabKey,fltk::event_state(), (TextEditor::Key_Func) tabcb);
		textEditor[i]->add_key_binding(fltk::ReturnKey,fltk::event_state(), (TextEditor::Key_Func) entercb);
		textEditor[i]->add_key_binding('u', CTRL | SHIFT, (TextEditor::Key_Func) ctrlUcb);
		textEditor[i]->add_key_binding('u', CTRL, (TextEditor::Key_Func) ctrlUcb);
	}

	textEditor[0]->label("Sbm");
	textEditor[1]->label("Python");

	tabGroup->end();
	//tabGroup->selected_child(textEditor[1]); // Python command interface by default
	tabGroup->selected_child(textEditor[0]);   // Sbm command interface by default

	this->end();

	this->resizable(tabGroup);
}

void CommandWindow::OnMessage( const std::string & message )
{
	UpdateOutput((char*) message.c_str());
}

CommandWindow::~CommandWindow()
{
	delete DisplayTextBuffer;
	//delete EditorTextBuffer;
	delete textEditor[0];
	delete textEditor[1];
	/*fltk::TextBuffer *DisplayTextBuffer;
	fltk::TextDisplay *textDisplay;*/
}

CommandWindow* CommandWindow::getCommandWindow(Widget* w)
{
	CommandWindow* commandWindow = NULL;
	Widget* widget = w;
	while (widget)
	{
		commandWindow = dynamic_cast<CommandWindow*>(widget);
		if (commandWindow)
			break;
		else
			widget = widget->parent();
	}

	return commandWindow;
}

void CommandWindow::entercb(int key, fltk::TextEditor* editor) 
{
	CommandWindow* commandWindow =  CommandWindow::getCommandWindow(editor);

	TextBuffer* tempBuffer = editor->buffer();
	char* c = (char*) tempBuffer->text();
	int len = (int) strlen(c);

	string str = "-> ";
	if (editor == commandWindow->textEditor[1])
		str = "~ ";
	str.append(c);

	commandWindow->UpdateOutput((char*) str.c_str(), true);

	if (len == 0)
		return;

	if (editor == commandWindow->textEditor[1])
	{
		//mcuCBHandle::singleton().executePython(c);
	}
	else
	{
		mcuCBHandle::singleton().execute(c);
	}
	commandWindow->addHistoryItem(c);

	tempBuffer->remove(0,len + 1);
}

void CommandWindow::upcb(int key, fltk::TextEditor* editor) 
{
	CommandWindow* commandWindow = CommandWindow::getCommandWindow(editor);
	commandWindow->historyLocation--;
	if (commandWindow->historyLocation < 0)
		commandWindow->historyLocation = 0;
	const char* command = commandWindow->getHistoryItem(commandWindow->historyLocation);
	editor->buffer()->remove(0, editor->buffer()->length());
	editor->buffer()->insert(editor->buffer()->length(), command);
}

void CommandWindow::downcb(int key, fltk::TextEditor* editor)
{
	CommandWindow* commandWindow = CommandWindow::getCommandWindow(editor);

	commandWindow->historyLocation++;
	if (commandWindow->historyLocation > int(commandWindow->historyItems.size()))
		commandWindow->historyLocation = commandWindow->historyItems.size();

	const char* command = commandWindow->getHistoryItem(commandWindow->historyLocation);
	editor->buffer()->remove(0, editor->buffer()->length());
	editor->buffer()->insert(editor->buffer()->length(), command);
}

void CommandWindow::clearcb(fltk::Widget* w, void* data)
{
	CommandWindow* commandWindow = CommandWindow::getCommandWindow(w);

	if (!commandWindow)
		return;
	TextDisplay* display = commandWindow->textDisplay;
	display->buffer()->remove(0, display->buffer()->length());
}

void CommandWindow::clearhistorycb(fltk::Widget* w, void* data)
{
	CommandWindow* commandWindow = CommandWindow::getCommandWindow(w);
	commandWindow->clearHistory();
}

void CommandWindow::tabcb(int key, fltk::TextEditor* editor)
{
	editor->insert("\t");

	if (true) return;

	TextBuffer* tempBuffer = editor->buffer();
	const char* c = tempBuffer->text();

	//get the last word
	char lastword[256] = "";
	bool singlequote = false;
	bool doublequote = false;
	bool word = false;
	int cursorPos = editor->insert_position();
	int j = 0;

	for (int i = 0; i < cursorPos; i++)
	{
		switch(c[i])
		{
		case ' ':
			if (doublequote || singlequote)
			{
				lastword[j++] = c[i];
			}
			else
			{
				word = true;
			}
			break;
		case '\\':
			if (singlequote)
			{
				lastword[j++] = c[i];
			}
			else
			{
				lastword[j++] = c[i++];
				lastword[j++] = c[i];
			}
			break;
		case '\'':
			lastword[j++] = c[i];
			if (!doublequote) 
				singlequote = !singlequote;
			break;
		case '\"':
			lastword[j++] = c[i];
			if (!singlequote) 
				doublequote = !doublequote;
			break;
		default:
			lastword[j++] = c[i];
			break;
		}

		if (word)
		{
			j = 0;
			word = false;
		}
	}
	lastword[j] = '\0';
}

void CommandWindow::ctrlUcb(int key, fltk::TextEditor* editor)
{
	TextBuffer* tempBuffer = editor->buffer();

	tempBuffer->remove(0, editor->insert_position());
}


void CommandWindow::UpdateOutput(char *text, bool origCommand)
{
	if(textDisplay == NULL) return;

/*	textDisplay->highlightdata(StyleBuffer, styletable,
								sizeof(styletable) / sizeof(styletable[0]),
								'A', styleunfinishedcb, 0);
	char *style = new char[strlen(text) + 1];
	memset(style, 'A', strlen(text));

*/


	std::string textString = text;
	// substitute any final \r\n for \n
	boost::replace_all(textString, "\r\n", "\n");

	if (true) //length <= 79)
	{
		// find the end of the buffer
		textDisplay->insert_position(9999999); // move to the last position

		if (origCommand)
		{
		    textDisplay->insert(textString.c_str());
			textDisplay->insert("\n");
		}
		else
		{
			// text is less than the width of the command window, just insert it
			textDisplay->insert(textString.c_str());
		}
	}
	else
	{
		// text size is greater than width of window - scroll to fit window
		int size =  (int) textString.length();
		while (size > 0)
		{
			// check for \n 
			int loc = (int) textString.find('\n');
			if (loc == -1 || loc > 79)
			{
				int max = 79;
				if (size < max)
					max = size;
				string g  = textString.substr(0, max).c_str() ;
				textDisplay->insert(textString.substr(0, max).c_str());
				textDisplay->insert("\n");
				if (size > max)
					textString = textString.substr(80);
				else
					textString = "";
			}
			else if (loc == 0)
			{
				textString = textString.substr(1);
			}
			else
			{
				textDisplay->insert(textString.substr(0, loc).c_str());
				textDisplay->insert("\n");
				textString = textString.substr(loc + 1);
			}
			size =  (int) textString.length();
		}

	}

}

void updateDisplay(int pos, int nInserted, int nDeleted, int b, const char* d, void* v)
{
	CommandWindow* commandWindow = static_cast<CommandWindow*>(v);
	
	commandWindow->textDisplay->show_insert_position();
}

void CommandWindow::addHistoryItem(const char* item)
{
	if (this->historyItems.size() > MAXHISTORYSIZE - 1)
	{
		freeHistorySpace();
	}
	this->historyItems.push_back(item);
	this->historyLocation = this->historyItems.size();
}

const char* CommandWindow::getHistoryItem(int location)
{
	if ((unsigned int) location < this->historyItems.size() && location >= 0)
		return (char*) this->historyItems[location].c_str();
	else
		return "";
}

void CommandWindow::clearHistory()
{
	historyItems.clear();
	historyCounter = 0;
	historyLocation = 0;
}

void CommandWindow::freeHistorySpace()
{
	int halfHistorySize = MAXHISTORYSIZE / 2;
	// shift history items down

	// delete the old values
	int count = 0;
	while (count < halfHistorySize)
	{
		historyItems.erase(historyItems.begin());
		count++;
	}

	if (historyLocation < count)
	{
		historyLocation = 0;
	}
	else
	{
		historyLocation -= count;
	}
}

