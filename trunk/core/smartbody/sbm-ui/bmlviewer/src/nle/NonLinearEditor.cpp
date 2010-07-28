#include "NonLinearEditor.h"
#include <sstream>
#include <limits>

namespace nle {

Mark::Mark()
{
	startTime = -1;
	endTime = -1;
	block = NULL;
	setSelected(false);

	for (int i = 0; i < 4; i++)
		bounds[i] = 0;

	setColor(fltk::BLACK);
	
	showName = true;
	info = "";
}

Mark::~Mark()
{
}

std::string Mark::getName()
{
	return name;
}

void Mark::setName(std::string markName)
{
	name = markName;
}

void Mark::setShowName(bool val)
{
	showName = val;
}

bool Mark::isShowName()
{
	return showName;
}

nle::Block* Mark::getBlock()
{
	return block;
}

void Mark::setBlock(Block* b)
{
	block = b;
}

bool Mark::isSelected()
{
	return selected;
}

void Mark::setSelected(bool val)
{
	selected = val;
}

void Mark::setBounds(int x, int y, int w, int h)
{
	bounds[0] = x;
	bounds[1] = y;
	bounds[2] = w;
	bounds[3] = h;
}

void Mark::getBounds(int& x, int& y, int& w, int& h)
{
	x = bounds[0];
	y = bounds[1];
	w = bounds[2];
	h = bounds[3];
}

void Mark::setStartTime(double t)
{
	startTime = t;
}

double Mark::getStartTime()
{
	return startTime;
}

void Mark::setEndTime(double t)
{
	endTime = t;
}

double Mark::getEndTime()
{
	return endTime;
}

void Mark::setColor(fltk::Color c)
{
	color = c;
}

fltk::Color Mark::getColor()
{
	return color;
}

void Mark::setInfo(std::string str)
{
	info = str;
}

std::string Mark::getInfo()
{
	return info;
}

Block::Block()
{
	startTime = -1;
	endTime = -1;
	track = NULL;
	setSelected(false);

	for (int i = 0; i < 4; i++)
		bounds[i] = 0;

	setColor(fltk::WHITE);
	info = "";
	showName = true;
}

Block::~Block()
{
	startTime = 0;
	endTime = 0;

	for (unsigned int m = 0; m < marks.size(); m++)
	{
		delete marks[m];
	}
}

std::string Block::getName()
{
	return name;
}

void Block::setName(std::string blockName)
{
	name = blockName;
}

void Block::setShowName(bool val)
{
	showName = val;
}

bool Block::isShowName()
{
	return showName;
}

void Block::setStartTime(double t)
{
	startTime = t;
}

double Block::getStartTime()
{
	return startTime;
}

void Block::setEndTime(double t)
{
	endTime = t;
}

double Block::getEndTime()
{
	return endTime;
}

bool Block::isSelected()
{
	return selected;
}

void Block::setSelected(bool val)
{
	selected = val;
}

void Block::setTrack(Track* t)
{
	track = t;
}

Track* Block::getTrack()
{
	return track;
}

void Block::setBounds(int x, int y, int w, int h)
{
	bounds[0] = x;
	bounds[1] = y;
	bounds[2] = w;
	bounds[3] = h;
}

void Block::getBounds(int& x, int& y, int& w, int& h)
{
	x = bounds[0];
	y = bounds[1];
	w = bounds[2];
	h = bounds[3];
}

int Block::getNumMarks()
{
	return marks.size();
}

Mark* Block::getMark(std::string name)
{
	for (unsigned int b = 0; b < marks.size(); b++)
	{
		if (marks[b]->getName() == name)
			return marks[b];
	}
	return NULL;
}

Mark* Block::getMark(int num)
{
	return marks[num];
}

void Block::removeMark(int num)
{
	int counter = 0;
	for (std::vector<Mark*>::iterator iter = marks.begin();
										iter != marks.end();
										iter++)
	{
		if (counter == num)
		{
			Mark* mark = *iter;
			marks.erase(iter);
			delete mark;
			return;
		}
		counter++;
	}
}

void Block::removeMark(std::string name)
{
	int counter = 0;
	for (std::vector<Mark*>::iterator iter = marks.begin();
										iter != marks.end();
										iter++)
	{
		if (marks[counter]->getName() == name)
		{
			Mark* mark = *iter;
			marks.erase(iter);
			delete mark;
			return;
		}
		counter++;
	}
}

void Block::removeMark(Mark* mark)
{
	int counter = 0;
	for (std::vector<Mark*>::iterator iter = marks.begin();
										iter != marks.end();
										iter++)
	{
		if (marks[counter] == mark)
		{
			Mark* mark = *iter;
			marks.erase(iter);
			delete mark;
			return;
		}
		counter++;
	}
}

void Block::releaseMark(Mark* mark)
{
	int counter = 0;
	for (std::vector<Mark*>::iterator iter = marks.begin();
										iter != marks.end();
										iter++)
	{
		if (marks[counter] == mark)
		{
			marks.erase(iter);
			return;
		}
		counter++;
	}
}


void Block::removeAllMarks()
{
	for (int m = this->getNumMarks() - 1; m >= 0; m--)
	{
		this->removeMark(m);
	}
}

Mark* Block::getMark(double time)
{
	// get the appropriate Mark for the time given
	for (unsigned int b = 0; b < this->marks.size(); b++)
	{
		if (marks[b]->getStartTime() <= time &&
			marks[b]->getEndTime() >= time)
			return marks[b];
	}
	return NULL;
}

void Block::addMark(Mark* mark)
{
	marks.push_back(mark);
	mark->setBlock(this);
}

void Block::setColor(fltk::Color c)
{
	color = c;
}

void Block::setInfo(std::string str)
{
	info = str;
}

std::string Block::getInfo()
{
	return info;
}

fltk::Color Block::getColor()
{
	return color;
}

Track::Track()
{
	setSelected(false);
    setActive(true);
}

Track::~Track()
{
	for (unsigned int b = 0; b < blocks.size(); b++)
	{
		delete blocks[b];
	}
}

std::string Track::getName()
{
	return name;
}

void Track::setName(std::string trackName)
{
	name = trackName;
}

void Track::addBlock(Block* block)
{
	blocks.push_back(block);
	block->setTrack(this);
}

int Track::getNumBlocks()
{
	return blocks.size();
}

Block* Track::getBlock(std::string name)
{
	for (unsigned int b = 0; b < blocks.size(); b++)
	{
		if (blocks[b]->getName() == name)
			return blocks[b];
	}
	return NULL;
}

Block* Track::getBlock(int num)
{
	return blocks[num];
}

void Track::removeBlock(int num)
{
	int counter = 0;
	for (std::vector<Block*>::iterator iter = blocks.begin();
										iter != blocks.end();
										iter++)
	{
		if (counter == num)
		{
			Block* block = *iter;
			blocks.erase(iter);
			delete block;
			return;
		}
		counter++;
	}
}

void Track::removeBlock(std::string name)
{
	int counter = 0;
	for (std::vector<Block*>::iterator iter = blocks.begin();
										iter != blocks.end();
										iter++)
	{
		if (blocks[counter]->getName() == name)
		{
			Block* block = *iter;
			blocks.erase(iter);
			delete block;
			return;
		}
		counter++;
	}
}

void Track::removeBlock(Block* block)
{
	int counter = 0;
	for (std::vector<Block*>::iterator iter = blocks.begin();
										iter != blocks.end();
										iter++)
	{
		if (blocks[counter] == block)
		{
			Block* block = *iter;
			blocks.erase(iter);
			delete block;
			return;
		}
		counter++;
	}
}

void Track::releaseBlock(Block* block)
{
	int counter = 0;
	for (std::vector<Block*>::iterator iter = blocks.begin();
										iter != blocks.end();
										iter++)
	{
		if (blocks[counter] == block)
		{
			blocks.erase(iter);
			return;
		}
		counter++;
	}
}

void Track::fitBlocks()
{
	// place all the blocks in sequence to make sure 
	// that no two controllers in a single track overlap
	bool hasOverlap = true;
	while (hasOverlap)
	{
		hasOverlap = false;
		for (unsigned int a = 0; a < blocks.size() - 1; a++)
		{
			for (unsigned int b = a + 1; b < blocks.size(); b++)
			{
				// check for overlap
				if ((blocks[a]->getStartTime() <= blocks[b]->getStartTime() &&
					blocks[a]->getEndTime() >  blocks[b]->getStartTime()))
				{
					hasOverlap = true; // do another iteration since a block was changed
					blocks[b]->setStartTime(blocks[a]->getEndTime());
					if (blocks[b]->getEndTime() <= blocks[b]->getStartTime())
					{
						blocks[b]->setEndTime(blocks[b]->getStartTime() + .2);
					}
				}
				else if (blocks[b]->getStartTime() <= blocks[a]->getStartTime() &&
						blocks[b]->getEndTime() >  blocks[a]->getStartTime())
				{
					hasOverlap = true; // do another iteration since a block was changed
					blocks[a]->setStartTime(blocks[b]->getEndTime());
					if (blocks[a]->getEndTime() <= blocks[a]->getStartTime())
					{
						blocks[a]->setEndTime(blocks[a]->getStartTime() + .2);
					}
				}
			}
		 }
	}
}

void Track::removeAllBlocks()
{
	for (int b = this->getNumBlocks() - 1; b >= 0; b--)
	{
		this->removeBlock(b);
	}
}

Block* Track::getBlock(double time)
{
	// get the appropriate block for the time given
	for (unsigned int b = 0; b < this->blocks.size(); b++)
	{
		if (blocks[b]->getStartTime() <= time &&
			blocks[b]->getEndTime() >= time)
			return blocks[b];
	}
	return NULL;
}

bool Track::isSelected()
{
	return selected;
}

void Track::setSelected(bool val)
{
	selected = val;
	if (!val)
	{
		for (int b = 0; b < this->getNumBlocks(); b++)
		{
			Block* block = this->getBlock(b);
			block->setSelected(false);
		}
	}
}

double Track::getLastBlockTime()
{
	double lastTime = 0;
	for (int b = 0; b < this->getNumBlocks(); b++)
	{
		Block* block = this->getBlock(b);
		if (block->getEndTime() > lastTime)
			lastTime = block->getEndTime();
	}

	return lastTime;
}

bool Track::isActive()
{
    return active;
}

void Track::setActive(bool val)
{
    active = val;
}

NonLinearEditorModel::NonLinearEditorModel()
{
	setTime(0.0);
	setEndTime(5.0);
 
    setModelChanged(true);
}

NonLinearEditorModel::~NonLinearEditorModel()
{
	clearContexts();

	for (unsigned int t = 0; t < tracks.size(); t++)
		delete tracks[t];
}

std::string NonLinearEditorModel::getName()
{
	return name;
}

void NonLinearEditorModel::setName(std::string modelName)
{
	name = modelName;
}

void NonLinearEditorModel::addTrack(Track* track)
{
	tracks.push_back(track);
}

int NonLinearEditorModel::getNumTracks()
{
	return tracks.size();
}

Track* NonLinearEditorModel::getTrack(std::string name)
{
	for (unsigned int b = 0; b < tracks.size(); b++)
	{
		if (tracks[b]->getName() == name)
			return tracks[b];
	}
	return NULL;
}

Track* NonLinearEditorModel::getTrack(unsigned int num)
{
	if (tracks.size() > num)
		return tracks[num];
	else
		return NULL;
}

int NonLinearEditorModel::getTrackIndex(Track* track)
{
    for (unsigned int b = 0; b < tracks.size(); b++)
    {
        if (track == tracks[b])
            return b;
    }
    return -1;
}


void NonLinearEditorModel::removeTrack(unsigned int num)
{
	int counter = 0;
	for (std::vector<Track*>::iterator iter = tracks.begin();
										iter != tracks.end();
										iter++)
	{
		if (counter == num)
		{
			Track* track = *iter;
			tracks.erase(iter);
			delete track;
			return;
		}
		counter++;
	}
}

void NonLinearEditorModel::removeTrack(std::string name)
{
	int counter = 0;
	for (std::vector<Track*>::iterator iter = tracks.begin();
										iter != tracks.end();
										iter++)
	{
		if (tracks[counter]->getName() == name)
		{
			Track* track = *iter;
			tracks.erase(iter);
			delete track;
			return;
		}
		counter++;
	}
}

void NonLinearEditorModel::removeTrack(Track* track)
{
	int counter = 0;
	for (std::vector<Track*>::iterator iter = tracks.begin();
										iter != tracks.end();
										iter++)
	{
		if (tracks[counter] == track)
		{
			Track* track = *iter;
			tracks.erase(iter);
			delete track;
			return;
		}
		counter++;
	}
}

void NonLinearEditorModel::setTime(double t)
{
	time = t;
}

double NonLinearEditorModel::getTime()
{
	return time;
}

void NonLinearEditorModel::setEndTime(double time)
{
	endTime = time;
}

double NonLinearEditorModel::getEndTime()
{
	return endTime;
}


void NonLinearEditorModel::removeAllTracks()
{
	for (int t = this->getNumTracks() - 1; t >= 0 ; t--)
	{
		this->removeTrack(t);
	}
}

void NonLinearEditorModel::clear(bool preserveForContexts)
{
	if (!preserveForContexts)
	{
		this->removeAllTracks();
	}
	else
	{
		tracks.clear(); // track objects are still allocated, assumed to be held in the contexts
	}
	this->setTime(0);
}

std::string NonLinearEditorModel::getUniqueBlockName()
{
	static int currentBlock = 0;

	std::stringstream strstr;

	bool isUnique = false;
	while (!isUnique)
	{
		isUnique = true;

        strstr.str("");
		strstr << "block" << currentBlock;
		currentBlock++;

		for (unsigned int t = 0; t < this->tracks.size(); t++)
		{
			Track* track = this->tracks[t];
			for (int b = 0; b < track->getNumBlocks(); b++)
			{
				if (track->getBlock(b)->getName() == strstr.str())
				{
					isUnique = false;
				}
			}
		}
	}

	return strstr.str();
}

std::string NonLinearEditorModel::getUniqueTrackName()
{
	static int currentTrack = 0;

	std::stringstream strstr;

	bool isUnique = false;
	while (!isUnique)
	{
        isUnique = true;

        strstr.str("");
        strstr << "track" << currentTrack;
		currentTrack++;

		for (unsigned int t = 0; t < this->tracks.size(); t++)
		{
			if (tracks[t]->getName() == strstr.str())
			{
				isUnique = false;
			}
		}
	}

	return strstr.str();
}

void NonLinearEditorModel::switchTracks(unsigned int num1, unsigned int num2)
{
    if (this->tracks.size() <= num1 || this->tracks.size() <= num2)
        return;
    
    // exchange the track pointers
    Track* temp = this->tracks[num1];
    this->tracks[num1] = this->tracks[num2];
    this->tracks[num2] = temp;    
    
    setModelChanged(true);
}


void NonLinearEditorModel::setModelChanged(bool val)
{
    changed = val;
}

bool NonLinearEditorModel::isModelChanged()
{
    return changed;
}



void Track::setBounds(int x, int y, int w, int h)
{
	bounds[0] = x;
	bounds[1] = y;
	bounds[2] = w;
	bounds[3] = h;
}

void Track::getBounds(int& x, int& y, int& w, int& h)
{
	x = bounds[0];
	y = bounds[1];
	w = bounds[2];
	h = bounds[3];
}

Block* Track::getFirstBlock()
{
	double prevTime = std::numeric_limits<double>::max();
	Block* prevBlock = NULL;
	for (unsigned int b = 0; b < this->blocks.size(); b++)
	{
		if (blocks[b]->getStartTime() < prevTime)
		{
			prevTime = blocks[b]->getStartTime();
			prevBlock = blocks[b];
		}
	}

	return prevBlock;
}

Block* Track::getPrevBlock(Block* block)
{
	if (block == NULL)
	{
		return NULL;
	}

	double closest = std::numeric_limits<double>::max();

	Block* prevBlock = NULL;
	for (unsigned int b = 0; b < this->blocks.size(); b++)
	{
		if (blocks[b] == block)
			continue;

		double distance = block->getStartTime() - blocks[b]->getStartTime();
		if (distance >=0 && distance < closest)
		{
			closest = distance;
			prevBlock = blocks[b];
		}
	}

	return prevBlock;
}

Block* Track::getNextBlock(Block* block)
{
	if (block == NULL)
	{
		return NULL;
	}

	double closest = std::numeric_limits<double>::max();

	Block* nextBlock = NULL;
	for (unsigned int b = 0; b < this->blocks.size(); b++)
	{
		if (blocks[b] == block)
			continue;

		double distance = blocks[b]->getStartTime() - block->getStartTime();
		if (distance >= 0 && distance < closest)
		{
			closest = distance;
			nextBlock = blocks[b];
		}
	}

	return nextBlock;
}

Block* Track::getLastBlock()
{
	double latestTime = -std::numeric_limits<double>::max();
	Block* latestBlock = NULL;
	for (unsigned int b = 0; b < this->blocks.size(); b++)
	{
		if (blocks[b]->getStartTime() > latestTime)
		{
			latestTime = blocks[b]->getStartTime();
			latestBlock = blocks[b];
		}
	}

	return latestBlock;
}

void NonLinearEditorModel::setContext(std::string name)
{
	for (unsigned int c = 0; c < contexts.size(); c++)
	{
		if (contexts[c].first == name)
		{
			currentContext = name;
			this->clear(true);
			std::vector<Track*>& allTracks = contexts[c].second;
			for (unsigned int t = 0; t < allTracks.size(); t++)
			{
				this->addTrack(allTracks[t]);
			}
			return;
		}

	}
	currentContext = "";
}

std::string NonLinearEditorModel::getContextName()
{
	return currentContext;
}

bool NonLinearEditorModel::getContext(std::string name, std::vector<Track*>& contextTracks)
{
	for (unsigned int c = 0; c < contexts.size(); c++)
	{
		if (contexts[c].first == name)
		{
		
			std::vector<Track*>& allTracks = contexts[c].second;
			for (int t = 0; t < allTracks.size(); t++)
			{
				contextTracks.push_back(allTracks[t]);
			}
			return true;
		}
	}

	return false;
}

void NonLinearEditorModel::saveContext(std::string name)
{
	for (unsigned int c = 0; c < contexts.size(); c++)
	{
		if (contexts[c].first == name)
		{
			return;
		}
	}
	std::vector<Track*> contextTracks;
	for (int t = 0; t < this->getNumTracks(); t++)
	{
		contextTracks.push_back(this->getTrack(t));
	}
	contexts.push_back( std::pair<std::string, std::vector<Track*> >(name, contextTracks) );
}

std::vector<std::pair<std::string, std::vector<Track*> > >& NonLinearEditorModel::getContexts()
{
	return contexts;
}

void NonLinearEditorModel::clearContexts()
{
	bool contextIsActive = false;
	for (unsigned int c = 0; c < contexts.size(); c++)
	{
		int counter = 0;
		std::vector<Track*>& contextTracks = contexts[c].second;
		for (std::vector<Track*>::iterator trackIter = contextTracks.begin();
			trackIter != contextTracks.end();
			trackIter++)
		{
			Track* track = (*trackIter);
			// if the track on the context is the same as the track,
			// the remove the track as well
			if (counter == 0 && tracks.size() > 0 && track == tracks[counter])
				contextIsActive = true;
			delete track;

		}
		if (contextIsActive)
			tracks.clear();
	}
	contexts.clear();
	
}


}
