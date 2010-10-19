#ifndef _PARAMANIMEDITORWIDGET_
#define _PARAMANIMEDITORWIDGET_

#include "nle/NonLinearEditorWidget.h"

class ParamAnimEditorWidget : public nle::EditorWidget
{
	public:
		ParamAnimEditorWidget(int x, int y, int w, int h, char* name);

		virtual void changeBlockSelectionEvent(nle::Block* block);
		virtual void changeTrackSelectionEvent(nle::Track* track);
		virtual void changeMarkSelectionEvent(nle::Mark* mark);

		void setBlockSelectionChanged(bool val);
		bool getBlockSelectionChanged();
		void setTrackSelectionChanged(bool val);
		bool getTrackSelectionChanged();
		void setMarkSelectionChanged(bool val);
		bool getMarkSelectionChanged();

protected:
		virtual void drawBlock(nle::Block* block, int trackNum, int blockNum);
		virtual void drawMark(nle::Block* block, nle::Mark* mark, int trackNum, int blockNum, int markNum);

		bool blockSelectionChanged;
		bool trackSelectionChanged;
		bool markSelectionChanged;

};

#endif