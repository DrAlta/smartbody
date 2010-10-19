#ifndef _NONLINEAREDITOR_
#define _NONLINEAREDITOR_

#include <string>
#include <vector>
#include <map>
#include <fltk/Color.h>

namespace nle 
{

class Block;
class Track;



class Mark
{
	public:
		Mark();
		~Mark();

		virtual std::string getName();
		virtual void setName(std::string name);
		void setShowName(bool val);
		bool isShowName();

		virtual Block* getBlock();
		virtual void setBlock(Block* b);
		bool isSelected();
		void setSelected(bool val);

		void setBounds(int x, int y, int w, int h);
		void getBounds(int& x, int& y, int& w, int& h);

		virtual void setStartTime(double time);
		virtual double getStartTime();
		virtual void setEndTime(double time);
		virtual double getEndTime();

		void setColor(fltk::Color c);
		fltk::Color getColor();

		void setInfo(std::string str);
		std::string getInfo();

	protected:
		double startTime;
		double endTime;
		std::string name;
		Block* block;
		bool selected;
		int bounds[4];
		fltk::Color color;
		bool showName;
		std::string info;
};


class Block
{
	public:
		Block();
		~Block();

		virtual std::string getName();
		virtual void setName(std::string name);
		void setShowName(bool val);
		bool isShowName();

		virtual void setStartTime(double time);
		virtual double getStartTime();
		virtual void setEndTime(double time);
		virtual double getEndTime();
		virtual Track* getTrack();
		virtual void setTrack(Track* t);
		bool isSelected();
		void setSelected(bool val);
		void setBounds(int x, int y, int w, int h);
		void getBounds(int& x, int& y, int& w, int& h);

		virtual void addMark(Mark* mark);
		virtual int getNumMarks();
		virtual Mark* getMark(std::string name);
		Mark* getMark(double time);
		virtual Mark* getMark(int num);
		virtual void removeMark(int num);
		virtual void removeMark(std::string name);
		virtual void removeMark(Mark* mark);
		virtual void removeAllMarks();
		virtual void releaseMark(Mark* mark);

		void setColor(fltk::Color c);
		fltk::Color getColor();

		void setInfo(std::string str);
		std::string getInfo();

	private:
		double startTime;
		double endTime;
		std::string name;
		std::vector<Mark*> marks;
		Track* track;
		bool selected;
		int bounds[4];
		fltk::Color color;
		std::string info;
		bool showName;

};

class Track
{
	public:
		Track();
		~Track();

		virtual std::string getName();
		virtual void setName(std::string name);
		virtual void addBlock(Block* block);
		virtual int getNumBlocks();
		virtual Block* getBlock(std::string name);
		virtual Block* getBlock(int num);
		virtual void removeBlock(int num);
		virtual void removeBlock(std::string name);
		virtual void removeBlock(Block* block);
		virtual void removeAllBlocks();
		virtual void releaseBlock(Block* block);
		virtual void fitBlocks();

		bool isSelected();
		void setSelected(bool val);
		double getLastBlockTime();
		void setBounds(int x, int y, int w, int h);
		void getBounds(int& x, int& y, int& w, int& h);
		Block* getFirstBlock();
		Block* getLastBlock();
		Block* getNextBlock(Block* block);
		Block* getPrevBlock(Block* block);

		virtual Block* getBlock(double time);
  
        bool isActive();
        void setActive(bool val);

	private:
		std::string name;
		std::vector<Block*> blocks;
		bool selected;
		int bounds[4];
        bool active;  
};

class NonLinearEditorModel
{
	public:
		NonLinearEditorModel();
		~NonLinearEditorModel();

		virtual std::string getName();
		virtual void setName(std::string name);
		virtual int getNumTracks();
		virtual Track* getTrack(unsigned int num);
		virtual Track* getTrack(std::string name);
        virtual int getTrackIndex(Track* track);
        virtual void addTrack(Track* track);
		virtual void removeTrack(unsigned int num);
		virtual void removeTrack(std::string name);
		virtual void removeTrack(Track* track);
		virtual void removeAllTracks();
        virtual void switchTracks(unsigned int num1, unsigned int num2);
		virtual void clear(bool preserveForContexts = false);

		virtual void setTime(double time);
		virtual double getTime();
		virtual void setEndTime(double endTime);
		virtual double getEndTime();

		std::string getUniqueBlockName();
		std::string getUniqueTrackName();
  
        void setModelChanged(bool val);
        bool isModelChanged();    

		void setContext(std::string name);
		std::string getContextName();
		void saveContext(std::string name);
		bool getContext(std::string name, std::vector<Track*>& contextTracks);
		std::vector<std::pair<std::string, std::vector<Track*> > >& getContexts();
		std::vector<Track*>& getContext(std::string name);
		void clearContexts();

	protected:
		std::vector<Track*> tracks;
		double time;
		std::string name;
		double endTime;
        bool changed;
		std::vector<std::pair<std::string, std::vector<Track*> > > contexts;
		std::string currentContext;
};

}
#endif
