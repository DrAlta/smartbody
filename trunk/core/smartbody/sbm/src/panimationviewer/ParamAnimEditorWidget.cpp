/*
 *  ParamAnimEditorWidget.cpp - part of SmartBody-lib's Test Suite
 *  Copyright (C) 2009  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Yuyu Xu, USC
 */

#include "ParamAnimEditorWidget.h"
#include "ParamAnimBlock.h"

ParamAnimEditorWidget::ParamAnimEditorWidget(int x, int y, int w, int h, char* name) : EditorWidget(x, y, w, h, name)
{
	blockSelectionChanged = false;
	trackSelectionChanged = false;
	this->lockBlockFunc(true);
}
	
void ParamAnimEditorWidget::drawBlock(nle::Block* block, int trackNum, int blockNum)
{
	EditorWidget::drawBlock(block, trackNum, blockNum);
}

void ParamAnimEditorWidget::drawMark(nle::Block* block, nle::Mark* mark, int trackNum, int blockNum, int markNum)
{
	CorrespondenceMark* cMark = dynamic_cast<CorrespondenceMark*>(mark);
	if (cMark)
	{
		int viewableStart = this->convertTimeToPosition(getViewableTimeStart());
		int viewableEnd = this->convertTimeToPosition(getViewableTimeEnd());	
		
		int trackTop = padding + timeWindowHeight + trackNum * trackHeight;// + yOffset;
		
		int bounds[4];
		mark->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
		//bounds[1] += yOffset;

		if (viewableStart > bounds[0] + bounds[2])
			return;
		if (viewableEnd < bounds[0])
			return;

		if (mark->isSelected())
			fl_color(FL_GREEN);
		else
			fl_color(mark->getColor());

		bool drawText = true;
		if (viewableStart > bounds[0])
		{
			int diff = viewableStart - bounds[0];
			bounds[0] += diff;
			bounds[2] -= diff;
			drawText = false;
		}
		//fltk::Rectangle rec(bounds[0] - 2, bounds[1], bounds[2] + 4, bounds[3]);
		fl_rectf(bounds[0] - 2, bounds[1], bounds[2] + 4, bounds[3]);

		
		if (mark->isShowName())
		{
			if (drawText)
			{
				int width, height;
				std::string markName = mark->getName();
				fl_measure(markName.c_str(), width, height);

				int startTextX = bounds[0];
				int startTextY = bounds[1];
				if (startTextX + width <= viewableEnd)
				{
					fl_color(FL_BLACK);
					fl_draw(markName.c_str(), startTextX, startTextY);
				}		
			}
		}

		// connect to the attached correspondance mark
		// get the position of the other mark
		std::vector<CorrespondenceMark*>& attached = cMark->getAttachedMark();
		for (size_t i = 0; i < attached.size(); i++)
		{
			if (attached[i])
			{
				int abounds[4];
				attached[i]->getBounds(abounds[0], abounds[1], abounds[2], abounds[3]);
				fl_color(FL_BLUE);
				fl_line(bounds[0], bounds[1], abounds[0], abounds[1]);
			}
		}
	}
}

void ParamAnimEditorWidget::changeBlockSelectionEvent(nle::Block* block)
{
	blockSelectionChanged = true;
}

void ParamAnimEditorWidget::changeTrackSelectionEvent(nle::Track* track)
{
	trackSelectionChanged = true;
}

void ParamAnimEditorWidget::changeMarkSelectionEvent(nle::Mark* mark)
{
	markSelectionChanged = true;
}

void ParamAnimEditorWidget::setBlockSelectionChanged(bool val)
{
	blockSelectionChanged = val;
}

bool ParamAnimEditorWidget::getBlockSelectionChanged()
{
	return blockSelectionChanged;
}

void ParamAnimEditorWidget::setup()
{
	EditorWidget::setup();
	
	double maxEndTime = 0.0f;
	// adjust the viewable end time
	for (int t = 0; t < model->getNumTracks(); t++)
	{
		nle::Track* track = model->getTrack(t);
		for (int b = 0; b < track->getNumBlocks(); b++)
		{
			nle::Block* block = track->getBlock(b);
			double endTime = block->getEndTime();
			if (maxEndTime < endTime)
				maxEndTime = endTime;
		}
	}
	this->setViewableTimeEnd(maxEndTime + 1.0);
}

void ParamAnimEditorWidget::setTrackSelectionChanged(bool val)
{
	trackSelectionChanged = val;
}

bool ParamAnimEditorWidget::getTrackSelectionChanged()
{
	return trackSelectionChanged;
}

void ParamAnimEditorWidget::setMarkSelectionChanged(bool val)
{
	markSelectionChanged = val;
}

bool ParamAnimEditorWidget::getMarkSelectionChanged()
{
	return markSelectionChanged;
}
