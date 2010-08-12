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

using namespace fltk;
using namespace std;

CommandWindow* CommandWindow::s_window = NULL;

CommandWindow* CommandWindow::getCommandWindow()
{
	return s_window;
}

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
	textEditor = new TextEditor(10, yDis, w - 20, h - yDis);  
	textEditor->textfont(COURIER);
#ifdef _MACOSX_
    textEditor->textsize(10);
#else
    textEditor->textsize(13);
#endif
	textEditor->box(fltk::UP_BOX);
	textEditor->textcolor(fltk::BLACK);
	EditorTextBuffer = new TextBuffer();
	textEditor->buffer(EditorTextBuffer); 
	textEditor->add_key_binding(fltk::UpKey, fltk::event_state(), (TextEditor::Key_Func) upcb);
	textEditor->add_key_binding(fltk::DownKey, fltk::event_state(), (TextEditor::Key_Func) downcb);

	textEditor->add_key_binding(fltk::TabKey,fltk::event_state(), (TextEditor::Key_Func) tabcb);
	textEditor->add_key_binding(fltk::ReturnKey,fltk::event_state(), (TextEditor::Key_Func) entercb);
	textEditor->add_key_binding('u', CTRL | SHIFT, (TextEditor::Key_Func) ctrlUcb);
	textEditor->add_key_binding('u', CTRL, (TextEditor::Key_Func) ctrlUcb);

	this->end();

	this->resizable(textEditor);

	s_window = this;

}

void CommandWindow::OnMessage( const std::string & message )
{
	UpdateOutput((char*) message.c_str());
}

CommandWindow::~CommandWindow()
{
	delete DisplayTextBuffer;
	//delete EditorTextBuffer;
	delete textEditor;
	/*fltk::TextBuffer *DisplayTextBuffer;
	fltk::TextDisplay *textDisplay;*/
}


void CommandWindow::entercb(int key, fltk::TextEditor* textEditor) 
{
	TextBuffer* tempBuffer = textEditor->buffer();
	char* c = (char*) tempBuffer->text();
	int len = (int) strlen(c);

	string str = "-> ";
	str.append(c);

	CommandWindow::getCommandWindow()->UpdateOutput((char*) str.c_str(), true);

	if (len == 0)
		return;

	mcuCBHandle::singleton().execute(c);
	CommandWindow::getCommandWindow()->addHistoryItem(c);


	tempBuffer->remove(0,len + 1);
}

void CommandWindow::upcb(int key, fltk::TextEditor* texteditor) 
{
	CommandWindow::getCommandWindow()->historyLocation--;
	if (CommandWindow::getCommandWindow()->historyLocation < 0)
		CommandWindow::getCommandWindow()->historyLocation = 0;
	const char* command = CommandWindow::getCommandWindow()->getHistoryItem(CommandWindow::getCommandWindow()->historyLocation);
	texteditor->buffer()->remove(0, texteditor->buffer()->length());
	texteditor->buffer()->insert(texteditor->buffer()->length(), command);
}

void CommandWindow::downcb(int key, fltk::TextEditor* texteditor)
{
	CommandWindow::getCommandWindow()->historyLocation++;
	if (CommandWindow::getCommandWindow()->historyLocation > int(CommandWindow::getCommandWindow()->historyItems.size()))
		CommandWindow::getCommandWindow()->historyLocation = CommandWindow::getCommandWindow()->historyItems.size();

	const char* command = CommandWindow::getCommandWindow()->getHistoryItem(CommandWindow::getCommandWindow()->historyLocation);
	texteditor->buffer()->remove(0, texteditor->buffer()->length());
	texteditor->buffer()->insert(texteditor->buffer()->length(), command);
}

void CommandWindow::clearcb(fltk::Widget* widget, void* data)
{
	TextDisplay* display = CommandWindow::getCommandWindow()->textDisplay;
	display->buffer()->remove(0, display->buffer()->length());
}

void CommandWindow::clearhistorycb(fltk::Widget* widget, void* data)
{
	CommandWindow::getCommandWindow()->clearHistory();
}
	
/*****************************************************************************
if using UNIX, this may give you errors because of differences between direct.h
and dirent.h
******************************************************************************/
void CommandWindow::FindFiles(char* curdir, char* c)
{
	TextEditor* texteditor = CommandWindow::getCommandWindow()->textEditor;
	TextBuffer* tempBuffer = texteditor->buffer();

	char lastword[256];
	bool doublequote = false;
	bool singlequote = false;
	int j = 0;

	// put a backslash before spaces and remove quotes as needed
	for (int i = 0; i < (int) strlen(c); i++)
	{
		switch(c[i])
		{
		case ' ':
			lastword[j++] = '\\';
			lastword[j++] = ' ';
			break;
		case '\\':
			if (singlequote || (doublequote && c[i+1] != '\"'))
			{
				lastword[j++] = c[i];
				lastword[j++] = c[i];
			}
			else
			{
				lastword[j++] = c[i++];
				lastword[j++] = c[i];
			}
			break;
		case '\'':
			if (!doublequote) 
				singlequote = !singlequote;
			else
				lastword[j++] = c[i];
			break;
		case '\"':
			if (!singlequote) 
				doublequote = !doublequote;
			else
				lastword[j++] = c[i];
			break;
		default:
			lastword[j++] = c[i];
			break;
		}
	}
	lastword[j] = '\0';

	char temp[8192];
	
	//FIX! 
	//danceInterp::getDirectoryListing(temp, 8192, lastword);
	
	std::vector<std::string> names2;
	char* oldname;
	oldname = strtok (temp," ");
	while (oldname != NULL)
	{
		if (oldname[0] != '{')
		{
			names2.push_back(string(oldname));
			oldname = strtok (NULL, " ");
		}
		else
		{
			std::string longname = string(oldname);
			oldname = strtok (NULL, "}");

			longname[0] = '\"';
			longname += " " + string(oldname) + "\"";
			names2.push_back(longname);
			oldname = strtok (NULL, " ");
		}
	}
	if (names2.size() == 1)
	{
		tempBuffer->remove(texteditor->insert_position() - strlen(c), texteditor->insert_position());
		CommandWindow::getCommandWindow()->textEditor->insert((names2[0] + " ").c_str());
	}
	if (names2.size() > 1)
	{
		char tempstring[8192] = "";
		std::string match;
		int minlen = (int) names2[0].length();
		for (unsigned int i = 0; i < names2.size(); i++)
		{
			strcat(tempstring, names2[i].c_str());
			strcat(tempstring, "  ");

			if (i > 0)
			{
				int m;
				for (m = 0; names2[i-1][m] == names2[i][m]; m++)
				{
				}
				if (m < minlen) 
					minlen = m;
			}
		}
		if (minlen > 0)
		{
			match.append(names2[0], 0, minlen);
			tempBuffer->remove(texteditor->insert_position() - strlen(c), texteditor->insert_position());
			CommandWindow::getCommandWindow()->textEditor->insert(match.c_str());
		}
		LOG(tempstring);
	}
}

void CommandWindow::tabcb(int key, fltk::TextEditor* textEditor)
{
	CommandWindow::getCommandWindow()->textEditor->insert("\t");

	if (true) return;

	TextBuffer* tempBuffer = CommandWindow::getCommandWindow()->textEditor->buffer();
	const char* c = tempBuffer->text();
	char directory[256];
	// FIX!
	//danceInterp::getCurrentDirectory(directory) ;
	//get the last word
	char lastword[256] = "";
	bool singlequote = false;
	bool doublequote = false;
	bool word = false;
	int cursorPos = textEditor->insert_position();
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


	FindFiles(directory, lastword);
}

void CommandWindow::ctrlUcb(int key, fltk::TextEditor* te)
{
	TextEditor* texteditor = CommandWindow::getCommandWindow()->textEditor;
	TextBuffer* tempBuffer = texteditor->buffer();

	tempBuffer->remove(0, texteditor->insert_position());
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
	CommandWindow::getCommandWindow()->textDisplay->show_insert_position();
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
	CommandWindow::getCommandWindow()->historyItems.clear();
	CommandWindow::getCommandWindow()->historyCounter = 0;
	CommandWindow::getCommandWindow()->historyLocation = 0;
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

