#ifndef _SBMOTIONBLENDBASE_H
#define _SBMOTIONBLENDBASE_H
#include <sr/sr_sn_colorsurf.h>
#include <sb/SBAnimationState.h>
#include <controllers/me_ct_param_animation_data.h>
#include <controllers/MeCtBlendEngine.h>


namespace SmartBody {
	class SBMotionBlendBase : public SBAnimationBlend
	{
	protected:
		MeCtBlendEngine* blendEngine;			
		int parameterDim;
		std::string interpType;
		std::vector<SrSnColorSurf*> errorSurfaces;
		std::vector<SrSnColorSurf*> smoothSurfaces;		
		std::string skeletonName;

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

	public:		
		SBMotionBlendBase();
		SBMotionBlendBase(const std::string& name, const std::string& skelName, int dimension);
		~SBMotionBlendBase();			
		virtual void addMotion(const std::string& motion, std::vector<double>& parameter);
		virtual void setMotionParameter(const std::string& motion, std::vector<double>& parameter);
		virtual std::vector<double> getMotionParameter(const std::string& motion);

		virtual void removeMotion(const std::string& motionName);
		void buildBlendBase(const std::string& motionParameter, const std::string& interpolatorType, bool copySimplex);
		void buildVisSurfaces(const std::string& errorType, const std::string& surfaceType, int segments, int dimensions);

		virtual bool getWeightsFromParameters(double x, std::vector<double>& weights);
		virtual bool getWeightsFromParameters(double x, double y, std::vector<double>& weights);
		virtual bool getWeightsFromParameters(double x, double y, double z, std::vector<double>& weights);
		virtual void getParametersFromWeights(float& x, std::vector<double>& weights);
		virtual void getParametersFromWeights(float& x, float& y, std::vector<double>& weights);
		virtual void getParametersFromWeights(float& x, float& y, float& z, std::vector<double>& weights);
		void addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);

		SrSnColorSurf* createCurveSurface(float radius, unsigned int dimension, SrVec center, SrVec2 phi, SrVec2 theta);		
		SrSnColorSurf* createFlatSurface(float depth, unsigned int dimension, SrVec2 topLeft, SrVec2 lowerRight);
		void createErrorSurfaces(const std::string& type, SrVec center, int segments, int dimensions, std::vector<SrSnColorSurf*>& surfList);
		void updateErrorSurace(SrSnColorSurf* surf, SrVec center);
		void updateSmoothSurface(SrSnColorSurf* surf);
		std::vector<SrSnColorSurf*>& getErrorSurfaces() { return errorSurfaces; }
		std::vector<SrSnColorSurf*>& getSmoothSurfaces() { return smoothSurfaces; }

		/* draw vector flow in-between consecutive frame pairs, to visualize smoothness of motion
		// if maxError=0, it'll be set as max vec norm among all frame pairs;
		// errThresholdPct is the threshold (percentage of maxError), vecs shorter than this are plotted in light gray.
		// added by David Huang, June 2012 */
		void createMotionVectorFlow(const std::string& motionName, float maxError=0.0f, float errThresholdPct=0.1f);
		//void createMotionVectorFlow(SkSkeleton* sk, SkMotion* mo, float max_err=0.0f, float err_th_pct=0.1f);
		std::vector<SrSnLines*>& getVectorFlowSrSnLines() { return vecflowLinesArray; }
		void clearMotionVectorFlow(void);

		/* plot motion frames. added by David Huang, June 2012 */
		void plotMotion(const std::string& motionName, unsigned int interval,
			bool clearAll, bool useRandomColor);
		void plotMotionFrameTime(const std::string& motionName, float time, bool useRandomColor);
		void plotMotionJointTrajectory(const std::string& motionName, const std::string& jointName,
			float start_t, float end_t, bool useRandomColor);
		std::vector<SrSnLines*>& getPlotMotionSrSnLines() { return plotMotionLinesArray; }
		void clearPlotMotion(void);
	};
}
#endif