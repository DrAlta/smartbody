#ifndef _SBSTATE_H
#define _SBSTATE_H
#include <sr/sr_sn_colorsurf.h>
#include <controllers/me_ct_param_animation_data.h>

# define VFLOW_LINE_WIDTH 2.0f

namespace SmartBody {

class SBAnimationBlend : public PABlend
{
	public:
		SBAnimationBlend();
		SBAnimationBlend(const std::string& name);
		~SBAnimationBlend();

		void setIncrementWorldOffsetY(bool flag);
		void addCorrespondencePoints(const std::vector<std::string>& motions, const std::vector<double>& points);
		void removeCorrespondencePoints(int index);

		virtual int getNumMotions();
		virtual std::string getMotion(int num);
		SkMotion* getSkMotion(const std::string& motionName);
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

		void buildVisSurfaces(const std::string& errorType, const std::string& surfaceType, int segments, int dimensions);
		SrSnColorSurf* createCurveSurface(float radius, unsigned int dimension, SrVec center, SrVec2 phi, SrVec2 theta);		
		SrSnColorSurf* createFlatSurface(float depth, unsigned int dimension, SrVec2 topLeft, SrVec2 lowerRight);
		void createErrorSurfaces(const std::string& type, SrVec center, int segments, int dimensions, std::vector<SrSnColorSurf*>& surfList);
		void updateErrorSurace(SrSnColorSurf* surf, SrVec center);
		void updateSmoothSurface(SrSnColorSurf* surf);
		std::vector<SrSnColorSurf*>& getErrorSurfaces() { return errorSurfaces; }
		std::vector<SrSnColorSurf*>& getSmoothSurfaces() { return smoothSurfaces; }


		/* motion vector flow for motion smoothness visualize: each vector is the abs. movement of a particular joint
		// as it traverse 3d space between two consecutive motion frames. Colors are assigned to the vectors representing
		// sudden change in vector length compared against local average of the length computed with a sliding window,
		// highlighting the abnormal speed-ups (warm color) and slowdowns (cool color) caused by jitters and such.
		// vecs within [1-plotTh, 1+plotTh] of local avg are plotted in light gray.
		// sliding window size = slidWinHalfSize x 2 + 1.
		// added by David Huang, June 2012 */
		void createMotionVectorFlow(const std::string& motionName, const std::string& chrName, float plotThreshold=0.45f, unsigned int slidWinHalfSize=7);
		std::vector<SrSnLines*>& getVectorFlowSrSnLines() { return vecflowLinesArray; }
		void clearMotionVectorFlow(void);

		/* plot motion frames (stick figures) and joint trajectory
		// added by David Huang, June 2012 */
		void plotMotion(const std::string& motionName, const std::string& chrName, unsigned int interval,
						bool clearAll, bool useRandomColor);
		void plotMotionFrameTime(const std::string& motionName, const std::string& chrName, float time, bool useRandomColor);
		void plotMotionJointTrajectory(const std::string& motionName, const std::string& chrName, const std::string& jointName,
										float start_t, float end_t, bool useRandomColor);
		std::vector<SrSnLines*>& getPlotMotionSrSnLines() { return plotMotionLinesArray; }
		void clearPlotMotion(void);

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

		std::vector<SrSnColorSurf*> errorSurfaces;
		std::vector<SrSnColorSurf*> smoothSurfaces;

		std::vector<SrSnLines*> vecflowLinesArray;
		// put a list of joint global positions into array
		void getJointsGPosFromSkel(SkSkeleton* sk, SrArray<SrVec>& pnts_array, const std::vector<SkJoint*>& jnt_list);
		// find maximum vector norm (vector are connected between jnt global positions in consecutive frame pairs)
		float getVectorMaxNorm(SrArray<SrArray<SrVec>*>& pnts_arr);
		std::vector<SrSnLines*> plotMotionLinesArray; // plotMotion()
		std::vector<SkJoint*> plot_excld_list;
		// create a list for joint exclusion when plotting motion and vector flow (hard coded to exclude fingers, eye, etc)
		void createJointExclusionArray(const std::vector<SkJoint*>& orig_list);
		bool isExcluded(SkJoint* j); // return true if joint is excluded

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