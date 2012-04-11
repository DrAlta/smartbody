#ifndef _SBSTATE_H
#define _SBSTATE_H

#include <sbm/me_ct_param_animation_data.h>

namespace SmartBody {

class SBAnimationState : public PAState
{
	public:
		SBAnimationState();
		SBAnimationState(const std::string& name);
		~SBAnimationState();

		void addCorrespondancePoints(const std::vector<std::string>& motions, const std::vector<double>& points);

		virtual int getNumMotions();
		virtual std::string getMotion(int num);
		virtual int getNumCorrespondancePoints();
		virtual std::vector<double> getCorrespondancePoints(int num);

		virtual void removeMotion(const std::string& motionName);

		virtual std::string getDimension();
		bool validateState();

	protected:
		bool addSkMotion(const std::string& motionName);
		bool removeSkMotion(const std::string& motionName);
		/*
			This function make sure that all the correspondance points are in ascendant order
		*/
		void validateCorrespondancePoints();

	protected:
		std::string _dimension;
		bool _isFinalized;

};


class SBAnimationState0D : public SBAnimationState
{
	public:
		SBAnimationState0D();
		SBAnimationState0D(const std::string& name);
		~SBAnimationState0D();

		virtual void addMotion(const std::string& motion);
		virtual void removeMotion(const std::string& motionName);
		
};

class SBAnimationState1D : public SBAnimationState
{
	public:
		SBAnimationState1D();
		SBAnimationState1D(const std::string& name);
		~SBAnimationState1D();

		virtual void addMotion(const std::string& motion, float parameter);
		virtual void removeMotion(const std::string& motionName);
		void setParameter(const std::string& motion, float parameter);
};

class SBAnimationState2D : public SBAnimationState
{
	public:
		SBAnimationState2D();
		SBAnimationState2D(const std::string& name);
		~SBAnimationState2D();

		virtual void addMotion(const std::string& motion, float parameter1, float paramter2);
		virtual void removeMotion(const std::string& motionName);
		void setParameter(const std::string& motion, float parameter1, float parameter2);
		void addTriangle(const std::string& motion1, const std::string& motion2, const std::string&motion3);
};

class SBAnimationState3D : public SBAnimationState
{
	public:
		SBAnimationState3D();
		SBAnimationState3D(const std::string& name);
		~SBAnimationState3D();

		virtual void addMotion(const std::string& motion, float parameter1, float paramter2, float paramter3);
		virtual void removeMotion(const std::string& motionName);
		void setParameter(const std::string& motion, float parameter1, float parameter2, float parameter3);
		void addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);

};
}
#endif