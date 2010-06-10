#ifndef _BEHAVIOREDITORWIDGET_
#define _BEHAVIOREDITORWIDGET_

#include "NonLinearEditorWidget.h"

class BehaviorEditorWidget : public EditorWidget
{
	public:
		BehaviorEditorWidget(int x, int y, int w, int h, char* name);


protected:
		virtual void drawBlock(Block* block, int trackNum, int blockNum);
		virtual void BehaviorEditorWidget::drawMark(Block* block, Mark* mark, int trackNum, int blockNum, int markNum);

};

#endif