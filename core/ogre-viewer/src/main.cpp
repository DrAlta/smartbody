/*
*  main.cpp - part of SmartBody Project's OgreViewer
*  Copyright (C) 2009  University of Southern California
*
*  SmartBody is free software: you can redistribute it and/or
*  modify it under the terms of the Lesser GNU General Public License
*  as published by the Free Software Foundation, version 3 of the
*  license.
*s
*  SmartBody is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  Lesser GNU General Public License for more details.
*
*  You should have received a copy of the Lesser GNU General Public
*  License along with SmartBody.  If not, see:
*      http://www.gnu.org/licenses/lgpl-3.0.txt
*
*  CONTRIBUTORS:
*      Ed Fast, USC
*      Andrew n marshall, USC
*	   Deepali Mendhekar, USC
*	   Shridhar Ravikumar, USC
*      Arno Hartholt, USC
*/

#include "OgreRenderer.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
		// Create application object
	OgreRenderer app;

	app.setUseBoneBus(true);

	// Processing command line arguments, if there are, then use dll mode
	std::string commandLine = lpCmdLine;
	std::stringstream strstr(commandLine);
	std::istream_iterator<std::string> it(strstr);
	std::istream_iterator<std::string> end;
	std::vector<std::string> tokenzied(it, end);
	int numTokens = tokenzied.size();
	int tokenCounter = 0;
	while (tokenCounter < numTokens)
	{
		std::string op = tokenzied[tokenCounter];
		if (op == "-seqpath" || op == "-scriptpath")
		{
			tokenCounter++;
			if (tokenCounter < numTokens)
			{
				std::string command = "path seq " + tokenzied[tokenCounter];
				app.m_initialCommands.push_back(command);
			}
		}
		if (op == "-seq")
		{
			tokenCounter++;
			if (tokenCounter < numTokens)
			{
				std::string command = "seq " + tokenzied[tokenCounter];
				app.m_initialCommands.push_back(command);
			}
		}
		if (op == "-script")
		{
			tokenCounter++;
			if (tokenCounter < numTokens)
			{
				std::string command = "pythonscript " + tokenzied[tokenCounter];
				app.m_initialCommands.push_back(command);
			}
		}
		tokenCounter++;
	}

	if (app.m_initialCommands.size() > 0)
		app.setUseBoneBus(false);

	try
	{
		app.go();
	}
	catch ( Exception & e )
	{
		MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}

	return 0;
}
