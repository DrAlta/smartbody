#ifndef _CHARACTERCREATERWINDOW_H_
#define _CHARACTERCREATERWINDOW_H_

#include <fltk/Window.h>
#include <fltk/Browser.h>
#include <fltk/Input.h>
#include <fltk/Button.h>
#include <vector>
#include <sk/sk_skeleton.h>

class CharacterCreatorWindow : public fltk::Window
{
	public:
		CharacterCreatorWindow(int x, int y, int w, int h, char* name);
		~CharacterCreatorWindow();

		void setSkeletons(std::vector<std::string>& skeletonNames);
		static void CreateCB(fltk::Widget* w, void* data);

		fltk::Browser* browserSkeletons;
		fltk::Input* inputName;
		fltk::Button* buttonCreate;
};


#endif