#ifndef _PARAM_ANIM_AUTO_MARKING_EDITOR_H_
#define _PARAM_ANIM_AUTO_MARKING_EDITOR_H_

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Browser.H>
#include <vector>

class PAStateEditor;
class PAAutoFootStepsEditor : public Fl_Window
{
public:
	PAAutoFootStepsEditor(PAStateEditor* editor, int x, int y, int w, int h);
	~PAAutoFootStepsEditor();

	static void confirmEditting(Fl_Widget* widget, void* data);
	static void cancelEditting(Fl_Widget* widget, void* data);

	// helper function
	bool kMeansClustering1D(int num, std::vector<double>& inputPoints, std::vector<double>& outMeans);
	void calculateMeans(std::vector<double>&inputPoints, std::vector<double>& means, double convergentValue);

	Fl_Input*			inputFloorHeight;
	Fl_Input*			inputHeightThreshold;
	Fl_Input*			inputSpeedThreshold;
	Fl_Input*			inputSpeedDetectWindow;
	Fl_Browser*			browserJoint;
	Fl_Input*			inputMaxSteps;
	Fl_Button*			buttonConfirm;
	Fl_Button*			buttonCancel;

	PAStateEditor*		stateEditor;
	int xDis;
	int yDis;
};
#endif