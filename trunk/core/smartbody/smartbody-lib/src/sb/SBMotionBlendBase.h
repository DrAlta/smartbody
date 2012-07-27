#ifndef _SBMOTIONBLENDBASE_H
#define _SBMOTIONBLENDBASE_H
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
		std::string skeletonName;

	public:		
		SBMotionBlendBase();
		SBMotionBlendBase(const std::string& name, const std::string& skelName, int dimension);
		~SBMotionBlendBase();			
		virtual void addMotion(const std::string& motion, std::vector<double>& parameter);
		virtual void setMotionParameter(const std::string& motion, std::vector<double>& parameter);
		virtual std::vector<double> getMotionParameter(const std::string& motion);

		virtual void removeMotion(const std::string& motionName);
		void buildBlendBase(const std::string& motionParameter, const std::string& interpolatorType, bool copySimplex);


		virtual bool getWeightsFromParameters(double x, std::vector<double>& weights);
		virtual bool getWeightsFromParameters(double x, double y, std::vector<double>& weights);
		virtual bool getWeightsFromParameters(double x, double y, double z, std::vector<double>& weights);
		virtual void getParametersFromWeights(float& x, std::vector<double>& weights);
		virtual void getParametersFromWeights(float& x, float& y, std::vector<double>& weights);
		virtual void getParametersFromWeights(float& x, float& y, float& z, std::vector<double>& weights);
		void addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);

	};
}
#endif // namespace