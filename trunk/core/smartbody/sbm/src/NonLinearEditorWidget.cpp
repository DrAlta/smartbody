#include "NonLinearEditorWidget.h"
#include <iostream>
#include <fltk/Cursor.h>

EditorWidget::EditorWidget(int x, int y, int w, int h, char* name) :
									fltk::Group(x, y, w, h, name)
{
	model = NULL;
	setup();
	setViewableTimeStart(0);
	setViewableTimeEnd(3);
	setTimeWindowSelected(false);

	selectState = STATE_NORMAL;
	cameraState = CAMERASTATE_NORMAL;
	setBlockCandidate(NULL, true);
}

EditorWidget::~EditorWidget()
{
}

void EditorWidget::setup()
{
	padding = 10;
	trackHeight = 30;
    activationSize = 8;
	left = padding;
	right = w() - padding;
	width = right - left;
	top = padding;
	bottom = h() - 10;
	height = top - bottom;
	labelWidth = 100;
	trackStart = left + labelWidth;
	timeWindowHeight = 10;

	// set up the tracks
	if (!model)
		return;

	int currentTrackLocation = padding + timeWindowHeight;
	for (int t = 0; t < model->getNumTracks(); t++)
	{
		Track* track = model->getTrack(t);

		int trackTop = currentTrackLocation;
		int trackLeft = trackStart;
		int trackEnd = right;
		int trackWidth = trackEnd - trackStart;

		for (int b = 0; b < track->getNumBlocks(); b++)
		{
			Block* block = track->getBlock(b);
			// set up the blocks

			double startTime = block->getStartTime();    
			//if (startTime > this->getViewableTimeEnd())
			//	continue;
			if (startTime < 0)
				startTime = 0;

			double endTime = block->getEndTime();
			//if (endTime < this->getViewableTimeStart())
			//	continue;
			if (endTime > this->getViewableTimeEnd())
				endTime = this->getViewableTimeEnd();
			if (endTime < 0)
				endTime = 0;

			int startPos = convertTimeToViewablePosition(startTime);
			int endPos = convertTimeToViewablePosition(endTime);

			std::vector<int> levelHeight;
			levelHeight.push_back(0);

			for (int m = 0; m < block->getNumMarks(); m++)
			{
				Mark* mark = block->getMark(m);

				double markStartTime = mark->getStartTime();    
				//if (markStartTime > this->getViewableTimeEnd())
				//	continue;
				if (markStartTime < 0)
					markStartTime = 0;

				double markEndTime = mark->getEndTime();
				//if (markEndTime < this->getViewableTimeStart())
				//	continue;
				if (markEndTime > this->getViewableTimeEnd())
					markEndTime = this->getViewableTimeEnd();
				if (markEndTime < 0)
					markEndTime = 0;

				int markStartPos = convertTimeToViewablePosition(markStartTime);
				int markEndPos = convertTimeToViewablePosition(markEndTime);

				// set the mark's bounds
				int width = markEndPos - markStartPos;
				if (width == 0)
					width = 1;
				int halfHeight = int(trackHeight / 2);
				mark->setBounds(markStartPos, trackTop + halfHeight, width, halfHeight);
				levelHeight[0] = trackTop + halfHeight;
			}
			// check for mark overlap
			bool hasOverlap = true;
			while (hasOverlap) 
			{
				hasOverlap = false;
				for (int m = 0; m < block->getNumMarks(); m++)
				{
					Mark* mark = block->getMark(m);
					int bounds[4];
					mark->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
					for (int n = m + 1; n < block->getNumMarks(); n++)
					{
						Mark* mark2 = block->getMark(n);
						// make sure these aren't the same marks
						//if (mark->getName() == mark2->getName())
						//	continue;
						int bounds2[4];
						mark2->getBounds(bounds2[0], bounds2[1], bounds2[2], bounds2[3]);
						if (bounds[1] == bounds2[1] && bounds[0] == bounds2[0]) // if these are at the same level and the same location
						{
							hasOverlap = true;
							// on which level is there a conflict?
							int conflictLevel = 0;
							for (unsigned int h = 0; h < levelHeight.size(); h++)
							{
								if (bounds[1] == levelHeight[h])
								{
									conflictLevel = h;
									break;
								}
							}
							if (conflictLevel + 1 == levelHeight.size())
							{
								currentTrackLocation += trackHeight;
								levelHeight.push_back(bounds[1] + trackHeight);
							}
							bounds2[1] += trackHeight;
							mark2->setBounds(bounds2[0], bounds2[1], bounds2[2], bounds2[3]);
							//std::cout << "Conflict between " << block->getName() << " [" << mark->getName() << "]" << " [" << mark2->getName() << "]" << std::endl;
							
						}
					}
				}
			}

			block->setBounds(startPos, trackTop, endPos - startPos, trackHeight * levelHeight.size());
		
		}

		currentTrackLocation += trackHeight;
		track->setBounds(trackStart, trackTop, trackWidth, currentTrackLocation - trackTop);
	}

	// make sure that the widget can accomodate all the tracks
	this->h(currentTrackLocation + trackHeight);

	// time window
	// viewable time window bar will be proportional to the model time
	double modelStart = 0;
	double modelEnd = model->getEndTime();

	int timeWindowLocationStart = trackStart;
	int timeWindowLocationEnd = right;

	// make sure time window is at least of a minimum size
	int timeWindowSize = timeWindowLocationEnd - timeWindowLocationStart;
	if (timeWindowSize < 5)
		timeWindowLocationEnd = timeWindowLocationStart + 5;

	// setup the time window
	int timeWindowTop = 0;
	int timeWindowBottom = timeWindowHeight;
	int timeWindowLeft = timeWindowLocationStart;
	int timeWindowRight = timeWindowLocationEnd;

	int timeWindowWidth = timeWindowRight - timeWindowLeft;

	// draw the time bar
	fltk::Rectangle timerec(timeWindowLocationStart, timeWindowTop, 
		                    timeWindowSize, timeWindowBottom - timeWindowTop);


	// set the time window bounds
	this->setTimeWindowBounds(timeWindowLeft, timeWindowTop, timeWindowLocationEnd - timeWindowLocationStart, timeWindowHeight);

	// set the time slider bounds
	int sliderLeft = 0;
	int sliderRight = 0;

	double sliderStartTime = this->getViewableTimeStart();
	double sliderEndTime = this->getViewableTimeEnd();

	double modelTimeSpan = modelEnd - modelStart;
	double ratioStart = sliderStartTime / modelTimeSpan;
	double ratioEnd = sliderEndTime / modelTimeSpan;
	sliderLeft = int(ratioStart * double(timeWindowWidth)) + timeWindowLeft;
	sliderRight = int(ratioEnd* double(timeWindowWidth)) + timeWindowLeft;
	this->setTimeSliderBounds(sliderLeft, timeWindowTop, sliderRight - sliderLeft, timeWindowHeight);
}

void EditorWidget::resize(int x, int y, int w, int h)
{
	fltk::Group::resize(x, y, w, h);
	setup();
}

void EditorWidget::setModel(NonLinearEditorModel* m)
{
	this->model = m;
}

NonLinearEditorModel* EditorWidget::getModel()
{
	return model;
}

void EditorWidget::initialize()
{
}

void EditorWidget::setWindowStart(double time)
{
	windowStartTime = time;
}

void EditorWidget::setWindowEnd(double time)
{
	windowEndTime = time;
}

void EditorWidget::setViewableTimeStart(double time)
{
	viewableTimeStart = time;
}

double EditorWidget::getViewableTimeStart()
{
	return viewableTimeStart;
}

void EditorWidget::setViewableTimeEnd(double time)
{
	viewableTimeEnd = time;
}

double EditorWidget::getViewableTimeEnd()
{
	return viewableTimeEnd;
}


void EditorWidget::draw()
{
	setup();
	fltk::Group::draw();

	if (!model)
		return;

	// draw the work area
	drawBackground();

	// draw the window sliders
	drawTimeWindow();

	// draw the grid ticks
	drawTicks();

	// draw each track
	int numTracks = model->getNumTracks();
	for (int t = 0; t < numTracks; t++)
	{
		Track* track = model->getTrack(t);
		drawTrack(track, t);
	}

	// draw each block
	for (int t = 0; t < numTracks; t++)
	{
		Track* track = model->getTrack(t);
		// draw each block
		for (int b = 0; b < track->getNumBlocks(); b++)
		{
			Block* block = track->getBlock(b);
			drawBlock(block, t, b);
			// draw the marks on the track
			for (int m = 0; m < block->getNumMarks(); m++)
			{
				Mark* mark = block->getMark(m);
				drawMark(block, mark, t, b, m);
			}
		}
	}

	// show the mouse hit targets
	if (false)
	{
		fltk::setcolor(fltk::GREEN);

		int loc[4];
		this->getTimeWindowBounds(loc[0], loc[1], loc[2], loc[3]);
		fltk::Rectangle r(loc[0], loc[1], loc[2], loc[3]);
		fltk::fillrect(r);

		for (int t = 0; t < numTracks; t++)
		{
			Track* track = model->getTrack(t);
			// draw each block
			for (int b = 0; b < track->getNumBlocks(); b++)
			{
				Block* block = track->getBlock(b);
				block->getBounds(loc[0], loc[1], loc[2], loc[3]);
				fltk::Rectangle r(loc[0], loc[1], loc[2], loc[3]);
				fltk::fillrect(r);
			}
		}
	}

	// draw the current time
	int timePos = this->convertTimeToPosition(this->getModel()->getTime());
	fltk::color(fltk::RED);
	fltk::drawline(timePos, top, timePos, bottom);

	// draw the time if dragging
	bool leftOrRight;
	Block* blockBeingDragged = getBlockCandidate(leftOrRight);
	if (blockBeingDragged)
	{
		double t = 0.0;
		if (leftOrRight)
			t = blockBeingDragged->getStartTime();
		else
			t = blockBeingDragged->getEndTime();
		char buff[256];
		sprintf(buff, "%8.4f", t);
		fltk::color(fltk::BLACK);
		int bounds[4];
		blockBeingDragged->getTrack()->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
		fltk::drawtext(buff, float(this->convertTimeToPosition(t)), float(bounds[1]) +  5.0f * float(bounds[3]) / 6.0f);
	}


}

void EditorWidget::drawBackground()
{
	fltk::setcolor(fltk::GRAY25);

	fltk::Rectangle back(left, top, width, height);
	fltk::fillrect(back);
}

void EditorWidget::drawTicks()
{
	if (!this->getModel())
		return;

	fltk::setcolor(fltk::GRAY25);

	// draw the timing every second
	char buff[128];
	int counter = 0;

	double cur =  this->getViewableTimeStart();
	double tickAmount = .25;

	// make sure that there aren't too many tick marks
	bool tickOk = false;
	while (!tickOk)
	{
		int numTicks = int((this->getViewableTimeEnd() - this->getViewableTimeStart()) / tickAmount);
		if (numTicks > 100)
			tickAmount *= 4.0;
		else
			tickOk = true;
	}
	
	while (cur <= this->getViewableTimeEnd())
	{
		int timePos = convertTimeToViewablePosition(cur);
		fltk::drawline(timePos, top, timePos, bottom);
		if (counter % 4 == 0)
		{
			sprintf(buff, "%6.2f", cur);
			int textW, textH;
			fltk::measure(buff, textW, textH);
			fltk::drawtext(buff, float(timePos) - float(textW) / 2.0f, float(top) + float(this->h()) - float(textH));
		}
		counter++;
		cur += tickAmount;
		
	}


}

void EditorWidget::drawTimeWindow()
{
	int bounds[4];

	this->getTimeSliderBounds(bounds[0], bounds[1], bounds[2], bounds[3]);

	fltk::Rectangle timerec(bounds[0], bounds[1], bounds[2], bounds[3]);
	fltk::fillrect(timerec);
	fltk::setcolor(fltk::BLACK);
	fltk::strokerect(timerec);

	// draw the time box
	if (isTimeWindowSelected())
		fltk::setcolor(fltk::GRAY10);
	else
		fltk::setcolor(fltk::WHITE);

	fltk::fillrect(timerec);
	fltk::setcolor(fltk::BLACK);
	fltk::strokerect(timerec);

	// draw the time label
	char buff[128];
	sprintf(buff, "%6.2f", this->getViewableTimeStart());
	fltk::drawtext(buff, float(bounds[0]), float((bounds[1] + (bounds[1] + bounds[3])) / 2 + 5));
	sprintf(buff, "%6.2f", this->getViewableTimeEnd());
	fltk::drawtext(buff, float(bounds[0] + bounds[2]), float((bounds[1] + (bounds[1] + bounds[3])) / 2 + 5));
}

void EditorWidget::drawTrack(Track* track, int trackNum)
{
	if (!track)
		return;

	int bounds[4];
	track->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
 
    // show the active/inactive box
    if (track->isActive())
    {
        fltk::setcolor(fltk::BLACK);
#ifdef WIN32
		Rectangle rect(left, bounds[1] + bounds[3] / 2 - activationSize, activationSize, activationSize);
        fltk::fillrect(rect);
#else
        fltk::fillrect(left, bounds[1] + bounds[3] / 2 - activationSize, activationSize, activationSize);
#endif
    }
    else
    {
        fltk::setcolor(fltk::BLACK);
#ifdef WIN32
		Rectangle rect(left, bounds[1] + bounds[3] / 2 - activationSize, activationSize, activationSize);
        fltk::strokerect(rect);
#else
        fltk::strokerect(left, bounds[1] + bounds[3] / 2 - activationSize, activationSize, activationSize);
#endif
    }

	// write the track names
	fltk::setcolor(fltk::BLACK);
	fltk::drawtext(track->getName().c_str(), float(left + 15), float((bounds[1] + (bounds[1] + bounds[3])) / 2));

	if (track->isSelected())
		fltk::setcolor(fltk::GRAY50);
	else
		fltk::setcolor(fltk::GRAY75);
	fltk::Rectangle trackrec(bounds[0], bounds[1], bounds[2], bounds[3]);
	fltk::fillrect(trackrec);

	fltk::setcolor(fltk::GRAY25);
	fltk::strokerect(trackrec);
}

void EditorWidget::drawBlock(Block* block, int trackNum, int blockNum)
{
	int viewableStart = this->convertTimeToPosition(getViewableTimeStart());
	int viewableEnd = this->convertTimeToPosition(getViewableTimeEnd());
	
	Track* track = block->getTrack();
	int trackBounds[4];
	track->getBounds(trackBounds[0], trackBounds[1], trackBounds[2], trackBounds[3]);
	int trackTop = trackBounds[1];
	
	int bounds[4];
	block->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	if (viewableStart > bounds[0] + bounds[2])
		return;
	if (viewableEnd < bounds[0])
		return;


	if (block->isSelected())
		fltk::setcolor(fltk::YELLOW);
	else
		fltk::setcolor(block->getColor());

	bool drawText = true;
	if (viewableStart > bounds[0])
	{
		int diff = viewableStart - bounds[0];
		bounds[0] += diff;
		bounds[2] -= diff;
		drawText = false;
	}
	fltk::Rectangle rec(bounds[0], bounds[1], bounds[2], bounds[3]);
	fltk::fillrect(rec);

	// draw an outline around the block
	fltk::setcolor(fltk::BLACK);
	fltk::strokerect(rec);

	
	if (block->isShowName())
	{
		if (drawText)
		{
			// draw the name
			int width, height;
			std::string blockName = block->getName();
			fltk::measure(blockName.c_str(), width, height);
			// make sure that the width of the block will fit in the block area
			if (width > bounds[2])
			{
				// show only the first few letters...
				if (blockName.size() > 2)
				{
					blockName = blockName.substr(0, 2);
					blockName.append("..");
				}
				fltk::measure(blockName.c_str(), width, height);
				if (width > bounds[2])
					width = bounds[2];
			}

			int startTextX = (bounds[0] + (bounds[0] + bounds[2])) / 2 - width / 2;
			int startTextY = trackTop + int(3.0 * double(trackHeight) / 4.0);
			if (startTextX + width <= viewableEnd)
				fltk::drawtext(blockName.c_str(), float(startTextX), float(startTextY));
		}
	}		
}

void EditorWidget::drawMark(Block* block, Mark* mark, int trackNum, int blockNum, int markNum)
{
	int viewableStart = this->convertTimeToPosition(getViewableTimeStart());
	int viewableEnd = this->convertTimeToPosition(getViewableTimeEnd());	
	
	int trackTop = padding + timeWindowHeight + trackNum * trackHeight;
	
	int bounds[4];
	mark->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
	if (viewableStart > bounds[0] + bounds[2])
		return;
	if (viewableEnd < bounds[0])
		return;

	if (mark->isSelected())
		fltk::setcolor(fltk::YELLOW);
	else
		fltk::setcolor(mark->getColor());

	bool drawText = true;
	if (viewableStart > bounds[0])
	{
		int diff = viewableStart - bounds[0];
		bounds[0] += diff;
		bounds[2] -= diff;
		drawText = false;
	}
	fltk::Rectangle rec(bounds[0], bounds[1], bounds[2], bounds[3]);
	fltk::strokerect(rec);

	
	if (mark->isShowName())
	{
		if (drawText)
		{
			int width, height;
			std::string markName = mark->getName();
			fltk::measure(markName.c_str(), width, height);

			int startTextX = bounds[0];
			int startTextY = bounds[1];
			if (startTextX + width <= viewableEnd)
			{
				fltk::setcolor(fltk::BLACK);
				fltk::drawtext(markName.c_str(), float(startTextX), float(startTextY));
			}		
		}
	}
}

int EditorWidget::handle(int event)
{
	NonLinearEditorModel* model = this->getModel();

	if (!model)
	{
		return fltk::Group::handle(event);
	}

	int mousex = fltk::event_x();
	int mousey = fltk::event_y();

	bool mouseHit = false;
	bool found = false;
	bool foundBorder = false;
	bool leftOrRight = false;
	Block* candidateBlock = NULL;             
    Block* selectedBlock = NULL;   

	switch (event)
	{
		case fltk::SHORTCUT:
		case fltk::KEY:
			{
				switch(fltk::event_key())
				{
					case 'f': 
						// bring any selected objects in focus
						// if nothing is selected, bring everything into focus
						std::vector<Block*> selectedBlocks;
						double minTime = 9999999;
						double maxTime = -9999999;
						double selectedMinTime = 0;
						double selectedMaxTime = 1;
						for (int t = 0; t < model->getNumTracks(); t++)
						{
							Track* track = model->getTrack(t);
							for (int b = 0; b < track->getNumBlocks(); b++)
							{
								Block* block = track->getBlock(b);
								if (block->isSelected())
								{
									selectedBlocks.push_back(block);
									if (block->getStartTime() < selectedMinTime)
										selectedMinTime = block->getStartTime();
									if (block->getEndTime() > selectedMaxTime)
										selectedMaxTime = block->getEndTime();
								}
								if (block->getStartTime() < minTime)
									minTime = block->getStartTime();
								if (block->getEndTime() > maxTime)
									maxTime = block->getEndTime();

							}
						}
						if (selectedBlocks.size() > 0)
						{
							if (selectedMinTime < 0)
								selectedMinTime = 0;
							if (selectedMaxTime > model->getEndTime())
								selectedMaxTime = model->getEndTime();
							if (selectedMaxTime - selectedMinTime < 1.0)
								selectedMaxTime = selectedMinTime + 1.0;
							this->setViewableTimeStart(selectedMinTime);
							this->setViewableTimeEnd(selectedMaxTime);
						}
						else
						{
							if (minTime < 0)
								minTime = 0;
							if (maxTime > model->getEndTime())
								maxTime = model->getEndTime();
							if (maxTime - minTime < 1.0)
								maxTime = minTime + 1.0;
							this->setViewableTimeStart(minTime);
							this->setViewableTimeEnd(maxTime);
						}
						redraw();
						break;
				}
			}
			break;
		case fltk::DRAG:
			{
				bool altKeyPressed = (fltk::get_key_state(fltk::LeftAltKey) || fltk::get_key_state(fltk::RightAltKey));
				if (altKeyPressed)
				{
					if (cameraState == CAMERASTATE_ZOOM)
					{
						int diff = clickPositionY - mousey;
						double expandAmount = double(diff) * .01;
						double timeSpan = clickViewableTimeEnd - clickViewableTimeStart;
						double newTimeSpan = timeSpan + timeSpan * expandAmount;
						if (newTimeSpan < 1.0) // make sure there is at least one second between min/max times
							newTimeSpan = 1.0;
						double timeSpanDiff = newTimeSpan - timeSpan;
						double newStartViewableTime = clickViewableTimeStart - timeSpanDiff / 2.0;
						double newEndViewableTime =  clickViewableTimeEnd + timeSpanDiff / 2.0;
						if (newStartViewableTime < 0)
						{
							newEndViewableTime += -newStartViewableTime;
							newStartViewableTime = 0;
						}
						if (newEndViewableTime > model->getEndTime())
						{
							newStartViewableTime -= newEndViewableTime -  model->getEndTime();
							newEndViewableTime = model->getEndTime();
						}
						this->setViewableTimeStart(newStartViewableTime);
						this->setViewableTimeEnd(newEndViewableTime);
						redraw();
						break;
					}
					else if (cameraState == CAMERASTATE_PAN)
					{
						int diff = mousex - clickPositionX;
						double panAmount = double(diff) * .01;
						double newStartViewableTime = clickViewableTimeStart + panAmount;
						double newEndViewableTime =  clickViewableTimeEnd + panAmount;
						double span = newEndViewableTime - newStartViewableTime;
						if (newStartViewableTime < 0)
						{	
							newEndViewableTime += -newStartViewableTime;
							newStartViewableTime = 0;				
						}
						if (newEndViewableTime > model->getEndTime())
						{
							newEndViewableTime = model->getEndTime();
							newStartViewableTime = newEndViewableTime - span;
						}

						this->setViewableTimeStart(newStartViewableTime);
						this->setViewableTimeEnd(newEndViewableTime);
						redraw();
						break;
					}
				}
			}
			candidateBlock = this->getBlockCandidate(leftOrRight);
			if (candidateBlock)
			{
                bool shiftKeyPressed = (fltk::get_key_state(fltk::LeftShiftKey) || fltk::get_key_state(fltk::RightShiftKey));

				double time = this->convertViewablePositionToTime(mousex);
				if (leftOrRight)
				{
					// find the block whose end time matches this start time                    
                    Block* blockToLeft = candidateBlock->getTrack()->getPrevBlock(candidateBlock);
                    
                    if (blockToLeft)
                    {
                        // if the SHIFT key is selected, move the start and end times in concert
                        if (shiftKeyPressed)
                        {
                            blockToLeft->setEndTime(time);
                        }
                        else
                        {
                            // limit to end time of other block
                            if (blockToLeft->getEndTime() > time)
                                time = blockToLeft->getEndTime();
                        }
                    }
                    candidateBlock->setStartTime(time);
				}
				else
				{
                    // if the SHIFT key is selected,
					// find the block whose end time matches this start time
                    Block* blockToRight = candidateBlock->getTrack()->getNextBlock(candidateBlock);
                    
                    if (blockToRight)
                    {
                        // if the SHIFT key is selected, move the start and end times in concert
                        if (shiftKeyPressed)
                        {
                            blockToRight->setStartTime(time);
                        }
                        else
                        {
                            // limit to end time of other block
                            if (blockToRight->getStartTime() < time)
                                time = blockToRight->getStartTime();
                        }
                    }                  
					candidateBlock->setEndTime(time);	
				}
                model->setModelChanged(true);
				redraw();
				return 1;
			}

			if (selectState == STATE_TIMEWINDOWSTART || 
				selectState == STATE_TIMEWINDOWEND ||
				selectState == STATE_TIMEWINDOWMIDDLE)
			{
				double startViewableTime = this->getViewableTimeStart();
				double endViewableTime = this->getViewableTimeEnd();

				int posDiff = mousex - clickPositionX;
				double endTime = this->getModel()->getEndTime();
				int windowBounds[4];
				this->getTimeWindowBounds(windowBounds[0], windowBounds[1], windowBounds[2], windowBounds[3]);
				int width = windowBounds[2];
				double movementRatio = double(posDiff) / double(width);
				double timeMovement = movementRatio * endTime;
				clickPositionX = mousex;

				if (selectState == STATE_TIMEWINDOWSTART)
				{
					double newTime = this->getViewableTimeStart() + timeMovement;
					if (newTime < 0)
						newTime = 0;
					if (newTime > this->getViewableTimeEnd() - 1.0)
						newTime = this->getViewableTimeEnd() - 1.0;
					this->setViewableTimeStart(newTime); 
					redraw();
				}
				if (selectState == STATE_TIMEWINDOWEND)
				{
					double newTime = this->getViewableTimeEnd() + timeMovement;
					if (newTime > this->getModel()->getEndTime())
						newTime = this->getModel()->getEndTime();
					if (newTime < this->getViewableTimeStart() + 1.0)
						newTime = this->getViewableTimeStart() + 1.0;
					this->setViewableTimeEnd(newTime); 
					redraw();
				}
				if (selectState == STATE_TIMEWINDOWMIDDLE)
				{
					double newStartTime = this->getViewableTimeStart() + timeMovement;
					double newEndTime = this->getViewableTimeEnd() + timeMovement;
					if (newStartTime < 0)
					{
						newStartTime = 0;
						newEndTime = (this->getViewableTimeEnd() - this->getViewableTimeStart());
					}
					if (newEndTime > this->getModel()->getEndTime())
					{
						newEndTime = this->getModel()->getEndTime();
						newStartTime = newEndTime - (this->getViewableTimeEnd() - this->getViewableTimeStart());
					}
					
					this->setViewableTimeStart(newStartTime);
					this->setViewableTimeEnd(newEndTime); 
					redraw();
				}
			}

            // check to see if the user is moving an entire block 
            for (int t = 0; t < model->getNumTracks(); t++)
            {
                Track* track = model->getTrack(t);
                for (int b = 0; b < track->getNumBlocks(); b++)
                {
                    Block* block = track->getBlock(b);
                    if (block->isSelected())
                    {
                        selectedBlock = block;
                        break;
                    }
                }
                if (selectedBlock)
                    break;
            }
            if (selectedBlock) // block is selected and user clicked on the middle of the block
            {
                // determine where to move the block
                double origTime = this->convertViewablePositionToTime(clickPositionX);
                double newTime = this->convertViewablePositionToTime(mousex);
                double timeDiff = newTime - origTime; // adjust block start/end by this amount
                                
                bool okToMove = true;
                if (timeDiff > 0) // if moving to the right, move block to the right until another block has been hit

                {
                    Block* nextBlock = selectedBlock->getTrack()->getNextBlock(selectedBlock);
                    if (nextBlock)
                    {
                        if (nextBlock->getStartTime() < selectedBlock->getEndTime() + timeDiff)
                        {
                            timeDiff = nextBlock->getStartTime() - selectedBlock->getEndTime();
                            if (timeDiff < 0)
                                okToMove = false;
                        }
                    }
                }
                else // if moving to the left, move block to the left until another block has been hit
                {
                    Block* prevBlock = selectedBlock->getTrack()->getPrevBlock(selectedBlock);
                    if (prevBlock)
                    {
                        if (prevBlock->getEndTime() > selectedBlock->getStartTime() + timeDiff)
                        {
                            timeDiff = prevBlock->getEndTime() -  selectedBlock->getStartTime();
                            if (timeDiff > 0)
                                okToMove = false;
                        }
                    }
                }
                if (okToMove)
                {
                    // make sure that the block doesn't go beyond the beginning
                    double startTime = selectedBlock->getStartTime() + timeDiff;
                    double endTime = selectedBlock->getEndTime() + timeDiff;
                    if (startTime >= 0)
                    {
                        selectedBlock->setStartTime(startTime);
                        selectedBlock->setEndTime(endTime);
                    }                        
                        
                    // reset the mouse position
                    clickPositionX = mousex;
                    model->setModelChanged(true);
                    redraw();
                }
                
            }
			break;

		case fltk::PUSH:		
            
            // remember this mouse position
            clickPositionX = mousex;
			clickPositionY = mousey;

			{
				// if the alt key is pressed, then zoom then enter zoom mode
				bool alt = (fltk::get_key_state(fltk::LeftAltKey) || fltk::get_key_state(fltk::RightAltKey));
				if (alt)
				{
					int button =  fltk::event_button();
					// right mouse is zoom
					if (button == 3)
					{
						cameraState = CAMERASTATE_ZOOM;
						clickViewableTimeStart = this->getViewableTimeStart();
						clickViewableTimeEnd = this->getViewableTimeEnd();
						return 1;
					}
					// middle mouse is pan
					else if (button == 2)
					{
						cameraState = CAMERASTATE_PAN;
						clickViewableTimeStart = this->getViewableTimeStart();
						clickViewableTimeEnd = this->getViewableTimeEnd();
						return 1;
					}
					// left mouse is ???
					else if (button == 1)
					{
					}

				}
			}

			// first, check for existence of block borders
			if (this->getBlockCandidate(leftOrRight))
			{
				return 1;
			}

			// check to see if the time window was hit		
			{				
				int bounds[4];
				this->getTimeSliderBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
				int minX = bounds[0];
				int maxX = bounds[0] + bounds[2];
				int minY = bounds[1];
				int maxY = bounds[1] + bounds[3];

				if ((mousex >= minX && mousex <= maxX) &&
					(mousey >= minY && mousey <= maxY))
				{
					// if left side was hit, move start time
					if (mousex <= minX + 2)
					{
						selectState = STATE_TIMEWINDOWSTART;
					}
					// if right side was hit, move end time
					else if (mousex + 2 >= maxX)
					{
						selectState = STATE_TIMEWINDOWEND;
					}
					// if middle was move, move entire time
					else
					{
						selectState = STATE_TIMEWINDOWMIDDLE;
						clickStartTime = this->getViewableTimeStart();
					}
					this->setTimeWindowSelected(true);
					mouseHit = true;
					found = true;
                    redraw();
				}
			}

			// check to see if a block was hit			
			for (int t = 0; t < model->getNumTracks(); t++)
			{
				Track* track = model->getTrack(t);
				for (int b = 0; b < track->getNumBlocks(); b++)
				{
					Block* block = track->getBlock(b);
					int bounds[4];
					block->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
					int minX = bounds[0];
					int maxX = bounds[0] + bounds[2];
					int minY = bounds[1];
					int maxY = bounds[1] + bounds[3];

					if ((mousex >= minX && mousex <= maxX) &&
						(mousey >= minY && mousey <= maxY))
					{
						block->setSelected(!block->isSelected());
						for (int t = 0; t < model->getNumTracks(); t++)
						{
							Track* selectTrack = model->getTrack(t);
							if (selectTrack != block->getTrack())
								selectTrack->setSelected(false);
							else
								selectTrack->setSelected(true);
						}
						block->getTrack()->setSelected(block->isSelected());
						mouseHit = true;
						// unselect the other blocks
						for (int b2 = 0; b2 < track->getNumBlocks(); b2++)
						{
							Block* block2 = track->getBlock(b2);
							if (block2 != block)
								block2->setSelected(false);
						}
						found = true;
                        model->setModelChanged(true);
                        redraw();
                    }
				}
			}

			// check for track selection
			if (!mouseHit)
			{
				for (int t = 0; t < model->getNumTracks(); t++)
				{
					Track* track = model->getTrack(t);
					int bounds[4];
					track->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
					int trackMinX = bounds[0];
					int trackMaxX = bounds[0] + bounds[2];
					int trackMinY = bounds[1];
					int trackMaxY = bounds[1] + bounds[3];

					if ((mousex >= trackMinX && mousex <= trackMaxX) &&
						(mousey >= trackMinY && mousey <= trackMaxY))
					{
						// hit the track
						for (int t = 0; t < model->getNumTracks(); t++)
						{
							Track* selectTrack = model->getTrack(t);
							if (selectTrack != track)
								selectTrack->setSelected(false);
							else
								selectTrack->setSelected(true);
						}
						mouseHit = true;
					}
                    else if (mousex < left + activationSize && mousex > left &&
                             mousey >= trackMinY && mousey <= trackMaxY) // check for track selection in name area
                    {
                        // hit the track, use activation/deactivation
                        if (track->isActive())
                        {
                            track->setActive(false);
                        }
                        else
                        {
                            track->setActive(true);
                        }
                        mouseHit = true;
                    }
				}
			}
                        
            model->setModelChanged(true);
            if (!mouseHit)
			{
				for (int t = 0; t < model->getNumTracks(); t++)
				{
					Track* track = model->getTrack(t);
					for (int b = 0; b < track->getNumBlocks(); b++)
					{
						Block* block = track->getBlock(b);
						block->setSelected(false);
					}
				}
			}
			else
			{
				redraw();
			}
			
			if (found)
				return 1;
			break;

	

		case fltk::RELEASE:
			selectState = STATE_NORMAL;
			cameraState = CAMERASTATE_NORMAL;
			this->setTimeWindowSelected(false);
			redraw();
			break;

		case fltk::MOVE:
			// if the cursor is above a block border, 
			// then change the cursor type
			for (int t = 0; t < model->getNumTracks(); t++)
			{
				Track* track = model->getTrack(t);
				for (int b = 0; b < track->getNumBlocks(); b++)
				{
					Block* block = track->getBlock(b);
					int bounds[4];					
					block->getBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
					int leftMinX = bounds[0];
					int leftMaxX = bounds[0] + 4;
					int leftMinY = bounds[1];
					int leftMaxY = bounds[1] + bounds[3];

					int rightMinX = bounds[0] + bounds[2] - 4;
					int rightMaxX = bounds[0] + bounds[2];
					int rightMinY = bounds[1];
					int rightMaxY = bounds[1] + bounds[3];


					if ((mousex >= leftMinX && mousex <= leftMaxX) &&
						(mousey >= leftMinY && mousey <= leftMaxY))
					{	
						this->setBlockCandidate(block, true);
						this->cursor(CURSOR_CROSS);
						foundBorder = true;
					}
					else if ((mousex >= rightMinX && mousex <= rightMaxX) &&
							 (mousey >= rightMinY && mousey <= rightMaxY))
					{	
						this->setBlockCandidate(block, false);
						this->cursor(CURSOR_CROSS);
						foundBorder = true;
					}
				}
			}
			break;

		default:
			break;
	}
	if (foundBorder)
	{
		this->cursor(CURSOR_CROSS);
	}
	else
	{
		this->cursor(CURSOR_DEFAULT);
		this->setBlockCandidate(NULL, true);
	}


	return fltk::Group::handle(event);
}

int EditorWidget::convertTimeToPosition(double time)
{
	if (!this->getModel())
		return (right - trackStart) / 2;

	double viewStartTime = this->getViewableTimeStart();
	double viewEndTime = this->getViewableTimeEnd();
	double spanTime = viewEndTime - viewStartTime;

	double ratio = (time - viewStartTime) / spanTime;
	if (ratio < 0)
		ratio = 0;
	if (ratio > 1)
		ratio = 1;

	double dWidth = (double) (right - trackStart);
	double relPosition = dWidth * ratio;

	return (int) relPosition + trackStart;
}

double EditorWidget::convertPositionToTime(int position)
{
	if (!this->getModel())
		return 0.0;

	double viewStartTime = this->getViewableTimeStart();
	double viewEndTime = this->getViewableTimeEnd();
	double spanTime = viewEndTime - viewStartTime;

	int relPosition = position - trackStart;
	if (relPosition < 0)
		relPosition = 0;

	double dWidth = (double) (right - trackStart);
	double ratio = relPosition / dWidth;

	double time = ratio * spanTime + this->getViewableTimeStart();

	return time;
}

int EditorWidget::convertTimeToViewablePosition(double time)
{
	if (!this->getModel())
		return (right - trackStart) / 2;

	// determine the size of the editor
	double ratio = (time - this->getViewableTimeStart()) / (this->getViewableTimeEnd() - this->getViewableTimeStart());

	double dWidth = (double) (right - trackStart);
	double result = dWidth * ratio;

	return (int) result + trackStart;
}

double EditorWidget::convertViewablePositionToTime(int position)
{
	if (!this->getModel())
		return 0.0;

	int relPosition = position - trackStart;
	if (relPosition < 0)
		relPosition = 0;

	double timeSpan = (this->getViewableTimeEnd() - this->getViewableTimeStart());

	double time = ((double) relPosition / (double) (right - trackStart)) * timeSpan;
	time += this->getViewableTimeStart();

	if (time > this->getViewableTimeEnd())
		time = this->getViewableTimeEnd();

	if (time < this->getViewableTimeStart())
		time = this->getViewableTimeStart();

	return time;
}


void EditorWidget::setTimeWindowBounds(int x, int y, int w, int h)
{
	timeWindowBounds[0] = x;
	timeWindowBounds[1] = y;
	timeWindowBounds[2] = w;
	timeWindowBounds[3] = h;
}

void EditorWidget::getTimeWindowBounds(int& x, int& y, int& w, int& h)
{
	x = timeWindowBounds[0];
	y = timeWindowBounds[1];
	w = timeWindowBounds[2];
	h = timeWindowBounds[3];
}

void EditorWidget::setTimeSliderBounds(int x, int y, int w, int h)
{
	timeSliderBounds[0] = x;
	timeSliderBounds[1] = y;
	timeSliderBounds[2] = w;
	timeSliderBounds[3] = h;
}

void EditorWidget::getTimeSliderBounds(int& x, int& y, int& w, int& h)
{
	x = timeSliderBounds[0];
	y = timeSliderBounds[1];
	w = timeSliderBounds[2];
	h = timeSliderBounds[3];
}

bool EditorWidget::isTimeWindowSelected()
{
	return timeWindowSelected;
}

void EditorWidget::setTimeWindowSelected(bool val)
{
	timeWindowSelected = val;
}

Block* EditorWidget::getBlockCandidate(bool& beginning)
{
	beginning = candidateBeginning;
	return blockCandidate;
}

void EditorWidget::setBlockCandidate(Block* block, bool beginning)
{
	blockCandidate = block;
	candidateBeginning = beginning;
}

