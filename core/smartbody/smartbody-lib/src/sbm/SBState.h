#ifndef _SBSTATE_H
#define _SBSTATE_H

#include <sbm/me_ct_param_animation_data.h>

class SBState : public PAStateData
{
	public:
		SBState();
		SBState(std::string name);
		~SBState();

		void addCorrespondancePoints(std::vector<std::string> motions, std::vector<double> points);

		virtual int getNumMotions();
		virtual std::string getMotion(int num);
		virtual int getNumCorrespondancePoints();
		virtual std::vector<double> getCorrespondancePoints(int num);

		virtual std::string getDimension();

	protected:
		std::string _dimension;

};


class SBState0D : public SBState
{
	public:
		SBState0D();
		SBState0D(std::string name);
		~SBState0D();

		void addMotion(std::string motion);
};

class SBState1D : public SBState
{
	public:
		SBState1D();
		SBState1D(std::string name);
		~SBState1D();

		void addMotion(std::string motion, float parameter);
};

class SBState2D : public SBState
{
	public:
		SBState2D();
		SBState2D(std::string name);
		~SBState2D();

		void addMotion(std::string motion, float parameter1, float paramter2);
		void addTriangle(std::string motion1, std::string motion2, std::string motion3);

};

class SBState3D : public SBState
{
	public:
		SBState3D();
		SBState3D(std::string name);
		~SBState3D();

		void addMotion(std::string motion, float parameter1, float paramter2, float paramter3);
		void addTetrahedron(std::string motion1, std::string motion2, std::string motion3,std::string motion4);

};

#endif