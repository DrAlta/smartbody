#ifndef _SBSTATE_H
#define _SBSTATE_H

#include <sbm/me_ct_param_animation_data.h>

namespace SmartBody {

class SBAnimationState : public PAStateData
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

		virtual std::string getDimension();

	protected:
		bool addSkMotion(const std::string& motionName);
		/*
			This function make sure that all the correspondance points are in ascendant order
		*/
		void validateCorrespondancePoints();

	protected:
		std::string _dimension;

};


class SBAnimationState0D : public SBAnimationState
{
	public:
		SBAnimationState0D();
		SBAnimationState0D(const std::string& name);
		~SBAnimationState0D();

		void addMotion(const std::string& motion);
};

class SBAnimationState1D : public SBAnimationState
{
	public:
		SBAnimationState1D();
		SBAnimationState1D(const std::string& name);
		~SBAnimationState1D();

		void addMotion(const std::string& motion, float parameter);
		void setParameter(const std::string& motion, float parameter);
};

class SBAnimationState2D : public SBAnimationState
{
	public:
		SBAnimationState2D();
		SBAnimationState2D(const std::string& name);
		~SBAnimationState2D();

		void addMotion(const std::string& motion, float parameter1, float paramter2);
		void setParameter(const std::string& motion, float parameter1, float parameter2);
		void addTriangle(const std::string& motion1, const std::string& motion2, const std::string&motion3);
};

class SBAnimationState3D : public SBAnimationState
{
	public:
		SBAnimationState3D();
		SBAnimationState3D(const std::string& name);
		~SBAnimationState3D();

		void addMotion(const std::string& motion, float parameter1, float paramter2, float paramter3);
		void setParameter(const std::string& motion, float parameter1, float parameter2, float parameter3);
		void addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);

};
}
#endif