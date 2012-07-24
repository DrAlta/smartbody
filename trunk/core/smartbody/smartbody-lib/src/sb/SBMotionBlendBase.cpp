#include <sb/SBMotionBlendBase.h>
#include <sb/SBMotion.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sbm/mcontrol_util.h>

#include <sr/sr.h>
#include <sr/sr_lines.h>

namespace SmartBody {


	SBMotionBlendBase::SBMotionBlendBase()
	{
		_isFinalized = false;
		parameterDim = 3;	
		_dimension = "3D";
		blendEngine = NULL;		
		interpType = "KNN";
	}

	SBMotionBlendBase::SBMotionBlendBase(const std::string& name, const std::string& skelName, int dimension) : SBAnimationBlend(name)
	{
		_isFinalized = false;
		parameterDim = 3;	
		_dimension = "3D";
		interpType = "KNN";

		SBSkeleton* sbSkel = SBScene::getScene()->getSkeleton(skelName);
		if (sbSkel)
		{
			skeletonName = skelName;
			blendEngine = new MeCtBlendEngine(sbSkel, "base");
		}
		else // error
		{
			blendEngine = NULL;		
		}
	}

	SBMotionBlendBase::~SBMotionBlendBase()
	{
	}

	void SBMotionBlendBase::addMotion( const std::string& motion, std::vector<double>& parameter )
	{
		addSkMotion(motion);
		setMotionParameter(motion, parameter);
	}

	void SBMotionBlendBase::removeMotion( const std::string& motionName )
	{
		SBAnimationBlend::removeMotion(motionName);

		// remove correspondence points
		removeSkMotion(motionName);
	}

	bool SBMotionBlendBase::getWeightsFromParameters( double x, std::vector<double>& weights )
	{	
		if (interpType == "Barycentric")
		{
			return PABlend::getWeightsFromParameters(x,weights);
		}
		dVector param;
		param.resize(1);
		param(0) = x;
		blendEngine->setBlendParameter(param, weights);	
		return true;
	}

	bool SBMotionBlendBase::getWeightsFromParameters( double x, double y, std::vector<double>& weights )
	{
		if (interpType == "Barycentric")
		{
			return PABlend::getWeightsFromParameters(x,y,weights);
		}
		dVector param;
		param.resize(2);
		param(0) = x;
		param(1) = y;
		blendEngine->setBlendParameter(param, weights);
		return true;
	}

	bool SBMotionBlendBase::getWeightsFromParameters( double x, double y, double z, std::vector<double>& weights )
	{
		if (interpType == "Barycentric")
		{
			return PABlend::getWeightsFromParameters(x,y,z,weights);
		}
		dVector param;
		param.resize(3);
		param(0) = x;
		param(1) = y;
		param(2) = z;
		blendEngine->setBlendParameter(param, weights);
		return true;
	}

	void SBMotionBlendBase::getParametersFromWeights( float& x, std::vector<double>& weights )
	{
		dVector param;
		blendEngine->getBlendParameterFromWeights(param, weights);
		x = (float)param(0);
	}

	void SBMotionBlendBase::getParametersFromWeights( float& x, float& y, std::vector<double>& weights )
	{
		dVector param;
		blendEngine->getBlendParameterFromWeights(param, weights);
		x = (float)param(0);
		y = (float)param(1);
	}

	void SBMotionBlendBase::getParametersFromWeights( float& x, float& y, float& z, std::vector<double>& weights )
	{
		dVector param;
		blendEngine->getBlendParameterFromWeights(param, weights);
		x = (float)param(0);
		y = (float)param(1);
		z = (float)param(2);	
	}

	void SBMotionBlendBase::setMotionParameter( const std::string& motion, std::vector<double>& parameter )
	{
		if (parameter.size() == 1)
			setParameter(motion,parameter[0]);
		else if (parameter.size() == 2)
			setParameter(motion,parameter[0],parameter[1]);
		else if (parameter.size() == 3)
			setParameter(motion,parameter[0],parameter[1],parameter[2]);
	}

	void SBMotionBlendBase::buildBlendBase( const std::string& motionParameter, const std::string& interpolatorType, bool copySimplex)
	{
		LOG("build blend base");
		blendEngine->init(motionParameter);	
		LOG("num motions = %d, motion parameter = %s, interpolator type = %s",motions.size(), motionParameter.c_str(), interpolatorType.c_str());
		interpType = interpolatorType;	
		blendEngine->updateMotionExamples(motions, interpolatorType);	
		// update all motion parameters
		for (unsigned int i=0;i<motions.size();i++)
		{
			dVector motionParam;
			SkMotion* motion = motions[i];		
			std::vector<double> param = getMotionParameter(motion->getName());		
			setMotionParameter(motion->getName(), param);
		}
		// automatically generate tetrahedron
		if (copySimplex && blendEngine->simplexList)
		{
			VecOfSimplex& simplexList = *blendEngine->simplexList;
			for (unsigned int i=0;i<simplexList.size();i++)
			{
				std::vector<std::string> motionNameList;
				Simplex& simp = simplexList[i];
				if (simp.numDim != 3) continue; // only handle tetrahedrons

				int nvert = simp.vertexIndices.size();
				motionNameList.resize(nvert);
				for (int k=0;k<nvert;k++)
					motionNameList[k] = motions[simp.vertexIndices[k]]->getName();

				addTetrahedron(motionNameList[0],motionNameList[1],motionNameList[2],motionNameList[3]);
			}		
		}
		SBSkeleton* sbSkel = SBScene::getScene()->getSkeleton(skeletonName);
		SrVec center = SrVec(0,0,0);
		if (sbSkel)
			center = sbSkel->getJointByName("base")->gmat().get_translation();

		// 	createErrorSurfaces("curve", center, 0, 50, errorSurfaces);
		// 	createErrorSurfaces("curve", center, 0, 80, smoothSurfaces);
		// 	for (unsigned int i=0;i<errorSurfaces.size();i++)
		// 	{
		// 		SrSnColorSurf* surf = errorSurfaces[i];
		// 		updateErrorSurace(surf, center);
		// 	}

		// 	for (unsigned int i=0;i<smoothSurfaces.size();i++)
		// 	{
		// 		SrSnColorSurf* surf = smoothSurfaces[i];
		// 		updateSmoothSurface(surf);
		// 	}

		pseudoParameters = blendEngine->resamplePts;
	}

	

	std::vector<double> SBMotionBlendBase::getMotionParameter( const std::string& motion )
	{
		dVector motionParam;
		blendEngine->getMotionParameter(motion,motionParam);
		std::vector<double> param;
		for (unsigned int i=0;i<motionParam.size();i++)
			param.push_back(motionParam(i));
		return param;
	}

	void SBMotionBlendBase::addTetrahedron( const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4 )
	{
		PABlend::addTetrahedron(motion1, motion2, motion3, motion4);
	}


	
	void SBMotionBlendBase::createMotionVectorFlow(const std::string& motionName, float maxError, float errThresholdPct)
	{
		SkMotion* mo = SBAnimationBlend::getSkMotion(motionName);
		if(mo==0) return;
		if(mo->frames()<2)
		{
			LOG("createMotionVectorFlow(): motion does not have enough frames, aborting...");
			return;
		}
		SkSkeleton* sk = mo->connected_skeleton();
		if(sk==0)
		{
			LOG("createMotionVectorFlow(): motion is not connected to any skeleton, aborting...");
			return;
		}

		SR_CLIP(errThresholdPct, 0.0f, 1.0f);

		clearMotionVectorFlow(); // clear vector flow SrSnLines

		const std::vector<SkJoint*>& jnts = sk->joints();
		const int size = jnts.size();
		const int frames = mo->frames();
		const float dur = mo->duration();
		const float intv = dur / frames;
		SrArray<SrArray<SrVec>*> pnts_arr;
		for(int i=0; i<frames; i++)
		{
			pnts_arr.push(new SrArray<SrVec>);
			// mo->apply_frame(i); // just use original frames
			mo->apply(intv * i); // resample motion time uniformly
			sk->invalidate_global_matrices();
			sk->update_global_matrices();
			getJointsGPosFromSkel(sk, *pnts_arr.top(), jnts);
		}

		if(maxError<srtiny) // need to find max vec norm from motion
		{
			maxError = getVectorMaxNorm(pnts_arr);
			LOG("createMotionVectorFlow(): max vector norm = %f \n", maxError);
		}
		for(int i=1; i<mo->frames(); i++)
		{
			SrSnLines* l = new SrSnLines; l->ref();
			vecflowLinesArray.push_back(l);
			SrLines& line = l->shape();

			line.init();
			//line.resolution(1.5f); // change vector flow lines thickness
			for(int k=0; k<size; k++)
			{
				float norm = dist(pnts_arr[i]->get(k), pnts_arr[i-1]->get(k));
				//float c = norm / maxError;
				float c = (norm-maxError*errThresholdPct) / (maxError*(1-errThresholdPct));
				SR_CLIP(c, 0.0f, 1.0f);
				if(c > errThresholdPct)
				{
					line.push_color(SrColor::interphue(c));
					line.push_line(pnts_arr[i]->get(k), pnts_arr[i-1]->get(k));
				}
				else
				{
					line.push_color(SrColor::lightgray);
					line.push_line(pnts_arr[i]->get(k), pnts_arr[i-1]->get(k));
				}
			}
		}
	}

	void SBMotionBlendBase::clearMotionVectorFlow(void)
	{
		// clear vector flow lines
		for(unsigned int i=0; i<vecflowLinesArray.size(); i++)
			vecflowLinesArray[i]->unref();
		vecflowLinesArray.resize(0);
	}

	void SBMotionBlendBase::plotMotion(const std::string& motionName, unsigned int interval,
		bool clearAll, bool useRandomColor)
	{
		SkMotion* mo = SBAnimationBlend::getSkMotion(motionName);
		if(mo==0) return;
		if(mo->frames()<3)
		{
			LOG("plotMotion(): motion does not have enough frames, aborting...");
			return;
		}
		SkSkeleton* sk = mo->connected_skeleton();
		if(sk==0)
		{
			LOG("plotMotion(): motion is not connected to any skeleton, aborting...");
			return;
		}

		if(interval < 2) interval = 2;

		// clear plot motion SrSnLines
		if(clearAll)
			clearPlotMotion();


		const SrColor color_begin = SrColor::blue;
		const SrColor color_end = SrColor::red;
		SrColor color_random;
		if(useRandomColor)
		{
			static float hue = 0.0f;
			color_random = SrColor::interphue(hue);
			hue += 0.1f;
			if(hue>1.0f) hue = 0.0f;
		}
		const std::vector<SkJoint*>& jnts = sk->joints();
		createJointExclusionArray(jnts); // making a list of excluded joints
		float mo_dur = mo->duration();
		for(unsigned int i=0; i<=interval; i++)
		{
			mo->apply(mo_dur/interval * i);
			sk->invalidate_global_matrices();
			sk->update_global_matrices();

			SrSnLines* l = new SrSnLines; l->ref();
			plotMotionLinesArray.push_back(l);
			SrLines& line = l->shape();

			line.init();
			if(useRandomColor) line.push_color(color_random);
			else line.push_color(lerp(color_begin, color_end, (float)i/interval));
			for(unsigned int k=0; k<jnts.size(); k++) // set k=1 to skip ref root
			{
				SkJoint* j = jnts[k];
				if(isExcluded(j)) continue;
				for(int m=0; m<j->num_children();m++)
				{
					if(isExcluded(j->child(m))) continue;
					line.push_line(j->gcenter(), j->child(m)->gcenter());
				}
			}
		}
	}

	void SBMotionBlendBase::plotMotionFrameTime(const std::string& motionName, float time, bool useRandomColor)
	{
		SkMotion* mo = SBAnimationBlend::getSkMotion(motionName);
		if(mo==0) return;
		SkSkeleton* sk = mo->connected_skeleton();
		if(sk==0)
		{
			LOG("plotMotionFrame(): motion is not connected to any skeleton, aborting...");
			return;
		}

		const std::vector<SkJoint*>& jnts = sk->joints();
		createJointExclusionArray(jnts); // making a list of excluded joints

		mo->apply(time);
		//sk->invalidate_global_matrices();
		sk->update_global_matrices();

		SrSnLines* l = new SrSnLines; l->ref();
		plotMotionLinesArray.push_back(l);
		SrLines& line = l->shape();
		line.init();
		if(useRandomColor)
		{
			static float hue = 0.0f;
			line.push_color(SrColor::interphue(hue));
			hue += 0.1f;
			if(hue>1.0f) hue = 0.0f;
		}
		else
			line.push_color(SrColor::lightgray);
		for(unsigned int k=0; k<jnts.size(); k++) // set k=1 to skip ref root
		{
			SkJoint* j = jnts[k];
			if(isExcluded(j)) continue;
			for(int m=0; m<j->num_children();m++)
			{
				if(isExcluded(j->child(m))) continue;
				line.push_line(j->gcenter(), j->child(m)->gcenter());
			}
		}
	}

	void SBMotionBlendBase::plotMotionJointTrajectory(const std::string& motionName, const std::string& jointName,
		float start_t, float end_t, bool useRandomColor)
	{
		SkMotion* mo = SBAnimationBlend::getSkMotion(motionName);
		if(mo==0) return;
		if(mo->frames()<3)
		{
			LOG("plotMotionJointTrajectory(): motion does not have enough frames, aborting...");
			return;
		}
		SkSkeleton* sk = mo->connected_skeleton();
		if(sk==0)
		{
			LOG("plotMotionJointTrajectory(): motion is not connected to any skeleton, aborting...");
			return;
		}
		SkJoint* jnt = sk->search_joint(jointName.c_str());
		if(!jnt)
		{
			LOG("plotMotionJointTrajectory(): specified joint is not found in skeleton, aborting...");
			return;
		}

		unsigned int frames = mo->frames();
		const float mo_dur = mo->duration();
		SR_CLIP(start_t, 0.0f, mo_dur);
		SR_CLIP(end_t, 0.0f, mo_dur);
		const SrColor color_begin = SrColor::blue;
		const SrColor color_end = SrColor::red;
		SrColor color_random;
		if(useRandomColor)
		{
			static float hue = 0.0f;
			color_random = SrColor::interphue(hue);
			hue += 0.1f;
			if(hue>1.0f) hue = 0.0f;
		}
		SrSnLines* l = new SrSnLines; l->ref();
		plotMotionLinesArray.push_back(l);
		SrLines& line = l->shape();
		line.init();
		mo->apply(start_t);
		sk->update_global_matrices();
		SrVec last_jnt_pos = jnt->gcenter(); // first frame
		for(float t=start_t; t<end_t; t+=0.016667f)
		{
			if(useRandomColor) line.push_color(color_random);
			else line.push_color(lerp(color_begin, color_end, (t-start_t)/(end_t-start_t))); // begin~end: blue~red
			mo->apply(t);
			sk->update_global_matrices();
			SrVec cur_jnt_pos = jnt->gcenter();
			line.push_line(last_jnt_pos, cur_jnt_pos);
			last_jnt_pos = cur_jnt_pos;
		}
	}


	void SBMotionBlendBase::clearPlotMotion(void)
	{
		for(unsigned int i=0; i<plotMotionLinesArray.size(); i++)
			plotMotionLinesArray[i]->unref();
		plotMotionLinesArray.resize(0);
	}

	void SBMotionBlendBase::getJointsGPosFromSkel(SkSkeleton* sk, SrArray<SrVec>& pnts_array,
		const std::vector<SkJoint*>& jnt_list)
	{
		const unsigned int size = jnt_list.size();
		pnts_array.capacity(size);
		pnts_array.size(size);

		for(unsigned int k=0; k<size; k++) // set k=1 to skip ref root
		{
			SkJoint* j = jnt_list[k];
			pnts_array[k] = j->gcenter();
		}
	}

	float SBMotionBlendBase::getVectorMaxNorm(SrArray<SrArray<SrVec>*>& pnts_arr)
	{
		const int frames = pnts_arr.size();
		const int size = pnts_arr[0]->size();
		float max_err = 0.0f;
		for(int i=1; i<frames; i++)
		{
			for(int k=0; k<size; k++)
			{
				float norm = dist(pnts_arr[i]->get(k), pnts_arr[i-1]->get(k));
				if(norm > max_err)
					max_err = norm;
			}
		}
		return max_err;
	}


	void SBMotionBlendBase::createJointExclusionArray(const std::vector<SkJoint*>& orig_list)
	{
		plot_excld_list.resize(0);
		for(unsigned int i=0; i<orig_list.size(); i++)
		{
			SkJoint* j = orig_list[i];
			SrString jname(j->name().c_str());
			if(jname.search("face")>=0) { plot_excld_list.push_back(j); continue; }
			if(jname.search("brow")>=0) { plot_excld_list.push_back(j); continue; }
			if(jname.search("eye")>=0)  { plot_excld_list.push_back(j); continue; }
			if(jname.search("nose")>=0) { plot_excld_list.push_back(j); continue; }
			if(jname.search("lid")>=0)  { plot_excld_list.push_back(j); continue; }
			if(jname.search("jaw")>=0)  { plot_excld_list.push_back(j); continue; }
			if(jname.search("tongue")>=0) { plot_excld_list.push_back(j); continue; }
			if(jname.search("lip")>=0)    { plot_excld_list.push_back(j); continue; }
			if(jname.search("cheek")>=0)  { plot_excld_list.push_back(j); continue; }

			if(jname.search("finger")>=0) { plot_excld_list.push_back(j); continue; }
			if(jname.search("thumb")>=0)  { plot_excld_list.push_back(j); continue; }
			if(jname.search("index")>=0)  { plot_excld_list.push_back(j); continue; }
			if(jname.search("middle")>=0) { plot_excld_list.push_back(j); continue; }
			if(jname.search("pinky")>=0)  { plot_excld_list.push_back(j); continue; }
			if(jname.search("ring")>=0)   { plot_excld_list.push_back(j); continue; }
		}
	}

	bool SBMotionBlendBase::isExcluded(SkJoint* j)
	{
		for(unsigned int i=0; i<plot_excld_list.size(); i++)
			if(plot_excld_list[i] == j)
				return true;
		return false;
	}


} // namespace