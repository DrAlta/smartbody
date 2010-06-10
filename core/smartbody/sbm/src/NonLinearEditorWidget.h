#ifndef _NONLINEAREDITORWIDGET_
#define _NONLINEAREDITORWIDGET_

#include <fltk/events.h>
#include <fltk/Window.h>
#include <fltk/Group.h>
#include <fltk/run.h>
#include <fltk/Color.h>
#include <string>
#include <vector>
#include <cmath>
#include <fltk/draw.h>

#include "NonLinearEditor.h"

using namespace fltk;

class EditorWidget : public fltk::Group
{
	public:
		EditorWidget(int x, int y, int w, int h, char* name);
		~EditorWidget();

		virtual void setModel(NonLinearEditorModel* model);
		virtual NonLinearEditorModel* getModel();

		virtual void setWindowStart(double time);
		virtual void setWindowEnd(double time);

		virtual void setViewableTimeStart(double time);
		virtual double getViewableTimeStart();
		virtual void setViewableTimeEnd(double time);
		virtual double getViewableTimeEnd();

		virtual void draw();
		virtual int handle(int event);
		virtual void setup();
		virtual void resize(int x, int y, int w, int h);

		virtual void setTimeWindowBounds(int x, int y, int w, int h);
		virtual void getTimeWindowBounds(int& x, int& y, int& w, int& h);

		virtual void setTimeSliderBounds(int x, int y, int w, int h);
		virtual void getTimeSliderBounds(int& x, int& y, int& w, int& h);

		virtual int convertTimeToPosition(double time);
		virtual double convertPositionToTime(int position);

		virtual int convertTimeToViewablePosition(double time);
		virtual double convertViewablePositionToTime(int position);

		virtual bool isTimeWindowSelected();
		virtual void setTimeWindowSelected(bool val);

		virtual Block* getBlockCandidate(bool& beginning);
		virtual void setBlockCandidate(Block* block, bool beginning);

protected:
		void initialize();

		virtual void drawBackground();
		virtual void drawTicks();
		virtual void drawTimeWindow();
		virtual void drawTrack(Track* track, int trackNum);
		virtual void drawBlock(Block* block, int trackNum, int blockNum);
		virtual void drawMark(Block* block, Mark* mark, int trackNum, int blockNum, int markNum);

		int padding;
		int trackHeight;
        int activationSize;
		int left;
		int right;
		int top;
		int bottom;
		int width;
		int height;
		int labelWidth;
		int trackStart;
		int timeWindowHeight;
		int timeWindowBounds[4];
		int timeSliderBounds[4];
		bool timeWindowSelected;

		int selectState;
		int cameraState;
        int clickPositionX;
        int clickPositionY;
		double clickStartTime;
		double clickViewableTimeStart;
		double clickViewableTimeEnd;

		static const int STATE_NORMAL = 0;
		static const int STATE_TIMEWINDOWSTART = 1;
		static const int STATE_TIMEWINDOWEND = 2;
		static const int STATE_TIMEWINDOWMIDDLE = 3;
		static const int CAMERASTATE_NORMAL = 4;
		static const int CAMERASTATE_ZOOM = 5;
		static const int CAMERASTATE_PAN = 6;

		NonLinearEditorModel* model;
		double windowStartTime;
		double windowEndTime;
		double viewableTimeStart;
		double viewableTimeEnd;
		Block* blockCandidate;
		bool candidateBeginning;
};

#endif
