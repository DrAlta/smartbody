#ifndef _SBSTATE_H
#define _SBSTATE_H

#include <controllers/me_ct_param_animation_data.h>

namespace SmartBody {

class SBAnimationBlend : public PABlend
{
	public:
		SBAnimationBlend();
		SBAnimationBlend(const std::string& name);
		~SBAnimationBlend();

		void addCorrespondencePoints(const std::vector<std::string>& motions, const std::vector<double>& points);
		void removeCorrespondencePoints(int index);

		virtual int getNumMotions();
		virtual std::string getMotion(int num);
		virtual int getNumCorrespondencePoints();
		virtual std::vector<double> getCorrespondencePoints(int num);
		virtual void setCorrespondencePoints(int motionIndex, int pointIndex, double value);

		virtual void removeMotion(const std::string& motionName);
		virtual void addEvent(const std::string& motion, double time, const std::string& type, const std::string& parameters, bool onceOnly);
		virtual void removeEvent(int index);
		virtual MotionEvent* getEvent(int index);
		virtual void removeAllEvents();
		virtual int getNumEvents();

		virtual std::string getDimension();
		bool validateState();

		std::string saveToString();

	protected:
		bool addSkMotion(const std::string& motionName);
		bool removeSkMotion(const std::string& motionName);
		/*
			This function make sure that all the correspondence points are in ascendant order
		*/
		void validateCorrespondencePoints();

	protected:
		std::string _dimension;
		bool _isFinalized;

};


class SBAnimationBlend0D : public SBAnimationBlend
{
	public:
		SBAnimationBlend0D();
		SBAnimationBlend0D(const std::string& name);
		~SBAnimationBlend0D();

		virtual void addMotion(const std::string& motion);
		virtual void removeMotion(const std::string& motionName);
		
};

class SBAnimationBlend1D : public SBAnimationBlend
{
	public:
		SBAnimationBlend1D();
		SBAnimationBlend1D(const std::string& name);
		~SBAnimationBlend1D();

		virtual void addMotion(const std::string& motion, float parameter);
		virtual void removeMotion(const std::string& motionName);
		void setParameter(const std::string& motion, float parameter);
};

class SBAnimationBlend2D : public SBAnimationBlend
{
	public:
		SBAnimationBlend2D();
		SBAnimationBlend2D(const std::string& name);
		~SBAnimationBlend2D();

		virtual void addMotion(const std::string& motion, float parameter1, float paramter2);
		virtual void removeMotion(const std::string& motionName);
		void setParameter(const std::string& motion, float parameter1, float parameter2);
		void addTriangle(const std::string& motion1, const std::string& motion2, const std::string&motion3);
};

class SBAnimationBlend3D : public SBAnimationBlend
{
	public:
		SBAnimationBlend3D();
		SBAnimationBlend3D(const std::string& name);
		~SBAnimationBlend3D();

		virtual void addMotion(const std::string& motion, float parameter1, float paramter2, float paramter3);
		virtual void removeMotion(const std::string& motionName);
		void setParameter(const std::string& motion, float parameter1, float parameter2, float parameter3);
		void addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);

};
}
#endif