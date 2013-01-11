#include "ExportWindow.h"
#include <vector>
#include <string>
#include <sb/SBScene.h>
#include <FL/Fl_File_Chooser.H>
#include <fstream>

ExportWindow::ExportWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(x, y, w, h, name)
{
	std::vector<std::string> aspects;
	aspects.push_back("scene");
	aspects.push_back("assets");
	aspects.push_back("cameras");
	aspects.push_back("lights");
	aspects.push_back("face definitions");
	aspects.push_back("joint maps");
	aspects.push_back("lip syncing");
	aspects.push_back("blends and transitions");
	aspects.push_back("gesture maps");
	aspects.push_back("pawns");
	aspects.push_back("characters");
	aspects.push_back("services");
	aspects.push_back("positions");

	int curY = 10;
	for (std::vector<std::string>::iterator iter = aspects.begin();
		 iter != aspects.end();
		 iter++)
	{
		std::string& str = (*iter);
		Fl_Check_Button* check = new Fl_Check_Button(10, curY, 150, 25, _strdup(str.c_str()));
		curY += 30;	
		checkExport.push_back(check);
	}

	inputFile = new Fl_Input(60, curY, 150, 25, "Export File");
	buttonFile = new Fl_Button(220, curY, 25, 25, "...");
	buttonFile->callback(FileCB, this);

	curY += 30;

	Fl_Button* buttonExport = new Fl_Button(10, curY, 80, 25, "Export");
	buttonExport->callback(ExportCB, this);


	this->end();
}

ExportWindow::~ExportWindow()
{
}

void ExportWindow::FileCB(Fl_Widget* widget, void* data)
{
	ExportWindow* window = (ExportWindow*) data;

	std::string mediaPath = SmartBody::SBScene::getSystemParameter("mediapath");
	const char* fileName = fl_file_chooser("Save to:", "*.py", mediaPath.c_str());

	if (fileName)
		window->inputFile->value(fileName);
}

void ExportWindow::ExportCB(Fl_Widget* widget, void* data)
{
	ExportWindow* window = (ExportWindow*) data;

	std::ofstream file(window->inputFile->value());
	if (!file.good())
	{
		fl_alert("Cannot save to file '%s'. No export done.", window->inputFile->value());
		return;
	}

	std::vector<std::string> aspects;
	for (std::vector<Fl_Check_Button*>::iterator iter = window->checkExport.begin();
		 iter != window->checkExport.end();
		 iter++)
	{
		if ((*iter)->value())
		{
			aspects.push_back((*iter)->label());
		}
	}

	std::string exportData = SmartBody::SBScene::getScene()->exportScene(aspects, false);
	file << exportData;
	file.close();

	window->hide();

}



