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

#ifndef COMMANDWINDOW
#define COMMANDWINDOW


#include <fltk/Window.h>
#include <fltk/Input.h>
#include <fltk/FloatInput.h>
#include <fltk/MultiLineInput.h>
#include <fltk/Output.h>
#include <fltk/MultiLineOutput.h>
#include <fltk/Box.h>
#include <fltk/Button.h>
#include <fltk/MenuBar.h>
#include <fltk/Choice.h>
#include <fltk/TextBuffer.h>
#include <fltk/TextEditor.h>
#include <fltk/TextDisplay.h>
#include <fltk/ValueSlider.h>
#include <fltk/MenuBar.h>
#include <fltk/draw.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "vhcl_log.h"

#define MAXHISTORYSIZE 10


class CommandWindow : public fltk::Window, public vhcl::Log::Listener
{
public:
	CommandWindow(int, int, int, int, const char*);
	~CommandWindow();

	static CommandWindow* getCommandWindow();

	int width;
	int height;
	char printout[1024];
	void UpdateOutput(char *text, bool origCommand = false);
	void addHistoryItem(const char* item);
	const char* getHistoryItem(int location);
	void clearHistory();

	virtual void OnMessage( const std::string & message );

	fltk::TextBuffer *EditorTextBuffer;
	fltk::TextEditor *textEditor;
	fltk::TextBuffer *DisplayTextBuffer;
	fltk::TextDisplay *textDisplay;
	fltk::MenuBar* menubar;

	static void testCB();
	static void upcb(int key, fltk::TextEditor* te);
	static void entercb(int key, fltk::TextEditor* te);
	static void downcb(int key, fltk::TextEditor* te);
	static void tabcb(int key, fltk::TextEditor* te);
	static void ctrlUcb(int key, fltk::TextEditor* te);

	static void clearcb(fltk::Widget* widget, void* data);
	static void clearhistorycb(fltk::Widget* widget, void* data);

	static void FindFiles(char*, char*);

private:
	void freeHistorySpace();

	int historyCounter;	
	int historyLocation;
	std::vector<std::string> historyItems;
	int when;
	char curDir[256];
	static CommandWindow* s_window;

}
;

#endif

