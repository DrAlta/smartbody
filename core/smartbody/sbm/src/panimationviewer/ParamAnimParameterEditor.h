#ifndef _PARAM_ANIM_PARAMETER_EDITOR_H_
#define _PARAM_ANIM_PARAMETER_EDITOR_H_

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>

class PAStateEditor;
class PAParameterEditor : public Fl_Window
{
public:
	PAParameterEditor(PAStateEditor* editor, int x, int y, int w, int h);
	~PAParameterEditor();

	static void changeParameter(Fl_Widget* widget, void* data);	
	static void confirmEditting(Fl_Widget* widget, void* data);
	static void cancelEditting(Fl_Widget* widget, void* data);


	Fl_Choice*		choiceParameter;
	Fl_Choice*		choiceDimension;
	Fl_Choice*		inputJoint;
	Fl_Choice*		choiceAxis;
	Fl_Input*		stateName;
	PAStateEditor*	stateEditor;
	Fl_Button*		buttonConfirm;
	Fl_Button*		buttonCancel;

	int xDis;
	int yDis;
};
#endif