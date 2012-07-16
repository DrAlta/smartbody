#include <sb/SBMotionBlendBase.h>
#include <sb/SBMotion.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sbm/mcontrol_util.h>

#include <sr/sr.h>

namespace SmartBody {


SBMotionBlendBase::SBMotionBlendBase()
{
	_isFinalized = false;
	parameterDim = 3;		
	blendEngine = NULL;		
	interpType = "KNN";
}

SBMotionBlendBase::SBMotionBlendBase(const std::string& name, const std::string& skelName, int dimension) : SBAnimationBlend(name)
{
	_isFinalized = false;
	parameterDim = dimension;	
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

void SBMotionBlendBase::buildVisSurfaces( const std::string& errorType, const std::string& surfaceType, int segments, int dimensions )
{
	if (errorType != "error" && errorType != "smooth")
	{
		LOG("Warning : errorType must be 'error' or 'smooth'");
		return;
	}

	std::vector<SrSnColorSurf*>& surfaces = (errorType == "error") ? errorSurfaces : smoothSurfaces;
	SBSkeleton* sbSkel = SBScene::getScene()->getSkeleton(skeletonName);
	SrVec center = SrVec(0,0,0);
	if (sbSkel && surfaceType == "curve")
		center = sbSkel->getJointByName("base")->gmat().get_translation();

	createErrorSurfaces(surfaceType, center, segments, dimensions, surfaces);
	for (unsigned int i=0;i<surfaces.size();i++)
	{
		SrSnColorSurf* surf = surfaces[i];
		if (errorType == "error")
			updateErrorSurace(surf, center);
		else if (errorType == "smooth")
			updateSmoothSurface(surf);
	}	
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


SrSnColorSurf* SBMotionBlendBase::createFlatSurface( float depth, unsigned int dimension, SrVec2 topLeft, SrVec2 lowerRight )
{
	SrSnColorSurf* surf = new SrSnColorSurf(); surf->ref();	
	SrModel* surf_model = surf->model();

	float xPos, yPos, zPos = depth;
	const int dim = dimension;
	const int size = dim * dim;	
	SrVec pnt;
	float xSize = lowerRight[0] - topLeft[0];
	float ySize = lowerRight[1] - topLeft[1];
	SrColor temp_c;
	SrMaterial mtl;

	SrArray<SrVec> grid_array;
	// generate vertices for grid edges/surf
	for (int i=0; i<dim; i++)
	{		
		yPos = topLeft[1] + float(ySize/dim*i);
		for (int j=0; j<dim; j++)
		{			
			xPos = topLeft[0] + float(xSize/dim*j);
			pnt = SrVec(xPos, yPos, zPos);
			grid_array.push(pnt);
		}
	}

	// build surf
	surf_model->init();
	surf_model->culling = false; // !!! back-face culling can be enabled/disabled here !!!
	for(int i=0; i<dim; i++)
	{
		//theta = -(float)(SR_PI/9) + (float)(SR_PI*4/3/ dim * i);
		yPos = topLeft[1] + float(ySize/dim*i);
		for(int j=0; j<dim; j++)
		{
			//phi = (float)(SR_PI/3.5) + (float)(SR_PI/2 / dim * j); 
			xPos = topLeft[0] + float(xSize/dim*j);
			//pnt = SrVec( float(ctr.x+r*sin(phi)*cos(theta)),float(ctr.y+r*cos(phi)-j/2.2), ctr.z-r*sin(phi)*sin(theta) );
			pnt = SrVec( xPos, yPos, zPos );

			//surf_model->V.push(pnt); // set sphere as surf
			surf_model->V.push().set(grid_array[i*dim + j]);
			VecOfInt adjs;
			for (int x=-1;x<=1;x++) // get adjacent vertices
			{
				for (int y=-1;y<=1;y++)
				{
					if (x==0 && y==0) continue;
					int nx = (i+x); int ny = (j+y);
					if (nx < 0 || nx >= dim || ny < 0 || ny >= dim) continue;
					int adjIdx = nx*dim + (ny);
					adjs.push_back(adjIdx);
				}
			}
			surf->vtxAdjList.push_back(adjs);
			temp_c = SrColor::interphue((float)i / dim);
			mtl.diffuse.set(temp_c.r, temp_c.g, temp_c.b, (srbyte)255);
			surf_model->M.push(mtl);
		}
	}

	// make faces out of vertex
	for (int i=0; i<dim-1; i++)
	{
		for (int j=0; j<dim-1; j++)
		{
			surf_model->F.push().set( i*dim+j, i*dim+j+1, (i+1)*dim+j+1 );
			surf_model->F.push().set( i*dim+j, (i+1)*dim+j+1, (i+1)*dim+j );
		}
	}
	surf->changed(true); // force update surf
	//surf->render_mode(srRenderMode::srRenderModeDefault);
	//surf->render_mode(srRenderMode::srRenderModeFlat);
	return surf;
}


SrSnColorSurf* SBMotionBlendBase::createCurveSurface( float radius, unsigned int dimension, SrVec center, SrVec2 phiRange, SrVec2 thetaRange )
{
	SrSnColorSurf* surf = new SrSnColorSurf(); surf->ref(); // color surf
	SrModel* surf_model = surf->model();

	float phi, theta, r = radius;
	const int dim = dimension;
	const int size = dim * dim;
	const SrVec ctr = center;
	SrVec pnt;

	SrColor temp_c;
	SrMaterial mtl;

	SrArray<SrVec> grid_array;

	// generate vertices for grid edges/surf
	for (int i=0; i<dim; i++)
	{
		//theta = -(float)(SR_PI/9) + (float)(SR_PI*4/3 / dim * i);
		theta = thetaRange[0] + float(thetaRange[1]/dim*i);
		for (int j=0; j<dim; j++)
		{
			//phi = (float)(SR_PI/3.5) + (float)(SR_PI/2 / dim * j); 
			phi = phiRange[0] + float(phiRange[1]/dim*j);

			//pnt = SrVec( float(ctr.x+r*sin(phi)*cos(theta)), float(ctr.y+r*cos(phi)-j/2.2), float(ctr.z-r*sin(phi)*sin(theta)) );
			pnt = SrVec( float(ctr.x+r*sin(phi)*sin(theta)), float(ctr.y+r*cos(phi)), float(ctr.z+r*sin(phi)*cos(theta)) );
			grid_array.push(pnt);
		}
	}

	// build surf
	surf_model->init();
	surf_model->culling = false; // !!! back-face culling can be enabled/disabled here !!!
	for(int i=0; i<dim; i++)
	{
		//theta = -(float)(SR_PI/9) + (float)(SR_PI*4/3/ dim * i);
		theta = thetaRange[0] + float(thetaRange[1]/dim*i) ;
		for(int j=0; j<dim; j++)
		{
			//phi = (float)(SR_PI/3.5) + (float)(SR_PI/2 / dim * j); 
			phi = phiRange[0] + float(phiRange[1]/dim*j);
			//pnt = SrVec( float(ctr.x+r*sin(phi)*cos(theta)),float(ctr.y+r*cos(phi)-j/2.2), ctr.z-r*sin(phi)*sin(theta) );
			pnt = SrVec( float(ctr.x+r*sin(phi)*sin(theta)), float(ctr.y+r*cos(phi)), float(ctr.z+r*sin(phi)*cos(theta)) );

			//surf_model->V.push(pnt); // set sphere as surf
			surf_model->V.push().set(grid_array[i*dim + j]);
			VecOfInt adjs;
			for (int x=-1;x<=1;x++) // get adjacent vertices
			{
				for (int y=-1;y<=1;y++)
				{
					if (x==0 && y==0) continue;
					int nx = (i+x); int ny = (j+y);
					if (nx < 0 || nx >= dim || ny < 0 || ny >= dim) continue;
					int adjIdx = nx*dim + (ny);
					adjs.push_back(adjIdx);
				}
			}
			surf->vtxAdjList.push_back(adjs);

			temp_c = SrColor::interphue((float)i / dim);
			mtl.diffuse.set(temp_c.r, temp_c.g, temp_c.b, (srbyte)255);
			surf_model->M.push(mtl);
		}
	}

	// make faces out of vertex
	for (int i=0; i<dim-1; i++)
	{
		for (int j=0; j<dim-1; j++)
		{
			surf_model->F.push().set( i*dim+j, i*dim+j+1, (i+1)*dim+j+1 );
			surf_model->F.push().set( i*dim+j, (i+1)*dim+j+1, (i+1)*dim+j );
		}
	}
	surf->changed(true); // force update surf
		//surf->render_mode(srRenderMode::srRenderModeDefault);
		//surf->render_mode(srRenderMode::srRenderModeFlat);
	return surf;
}

void SBMotionBlendBase::createErrorSurfaces( const std::string& type, SrVec center, int segments, int dimensions, std::vector<SrSnColorSurf*>& surfList )
{
	if (type == "curve")
	{
		float minDist = 1e30f, maxDist = -1e30f;
		float minPhi = 1e30f, maxPhi = -1e30f;
		float minTheta = 1e30f, maxTheta = -1e30f;
		float toDegree = 180.f/(float)SR_PI;
		for (unsigned int i=0;i<parameters.size();i++)
		{		
			SrVec offset = parameters[i] - center;
			//LOG("parameter = %f %f %f",parameters[i][0],parameters[i][1],parameters[i][2]);
			//offset.y = 0.f; // ignore y distance		
			float curDist = offset.norm();
			if (curDist < minDist)
				minDist = curDist;
			if (curDist > maxDist)
				maxDist = curDist;
			SrVec offsetDir = parameters[i] - center;		
			offsetDir.normalize();
			float cosValue = offsetDir.y;		
			float tanValue = offsetDir.x/offsetDir.z;
			//float phi = acosf(offsetDir.y);//asinf(sqrtf(1.f-offsetDir.y*offsetDir.y));
			float phi = atan2f(sqrtf(offsetDir.x*offsetDir.x+offsetDir.z*offsetDir.z),offsetDir.y);
			//if (offsetDir.y < 0) phi = -phi;
			float theta = atan2f(offsetDir.x, offsetDir.z);//SR_PI - atan2f(offsetDir.x,offsetDir.z);
			if (theta > (float)SR_PI*3.f/4.f) theta = theta - (float)SR_PI*2;
			//float theta = atan2f(offsetDir.z,offsetDir.x);

			//LOG("tan = %f, theta = %f, cos = %f, phi = %f",tanValue, theta*toDegree, cosValue, phi*toDegree);
			if (theta < minTheta) minTheta = theta;
			if (theta > maxTheta) maxTheta = theta;
			if (phi < minPhi) minPhi = phi;
			if (phi > maxPhi) maxPhi = phi;
		}	 
		minDist = (minDist+maxDist)*0.5f; // make sure "near surf" is not too close
		//LOG("maxPhi = %f, minPhi = %f, maxTheta = %f, minTheta = %f",maxPhi*toDegree, minPhi*toDegree, maxTheta*toDegree, minTheta*toDegree);
		SrVec2 thetaRange = SrVec2(minTheta, maxTheta-minTheta);
		SrVec2 phiRange = SrVec2(minPhi, maxPhi-minPhi);
		if(segments==0 || maxDist-minDist<srtiny) // creat only one surface in the middle
		{
			float radius = (minDist+maxDist)*0.5f;
			SrSnColorSurf* Surf = createCurveSurface(radius, dimensions, center, phiRange, thetaRange);
			surfList.push_back(Surf);
		}
		else
		{
			float distOffset = (maxDist - minDist)/segments;
			for (float radius = minDist; radius <= maxDist + distOffset*0.05f; radius+= distOffset)
			{
				SrSnColorSurf* Surf = createCurveSurface(radius, dimensions, center, phiRange, thetaRange);
				surfList.push_back(Surf);
				//updateErrorSurace(Surf, center);
			}
		}
	}
	else if ( type == "flat")
	{
		SrVec bboxMin = SrVec(1e30f, 1e30f, 1e30f);
		SrVec bboxMax = SrVec(-1e30f, -1e30f, -1e30f);
		for (unsigned int i=0;i<parameters.size();i++)
		{
			SrVec& pos = parameters[i];
			for (int k=0;k<3;k++)
			{
				if (pos[k] < bboxMin[k])
					bboxMin[k] = pos[k];
				if (pos[k] > bboxMax[k])
					bboxMax[k] = pos[k];
			}
		}
		SrVec2 topLeft = SrVec2(bboxMin[0],bboxMin[1]);
		SrVec2 lowerRight = SrVec2(bboxMax[0],bboxMax[1]);
		SrVec2 surfSize = lowerRight - topLeft;
		float surfaceScale = max(surfSize[0],surfSize[1]);

		float minDist = bboxMin[2];
		float maxDist = bboxMax[2];
		if (segments == 0)
		{
			minDist = (minDist+maxDist)*0.5f;
		}
		if(segments==0 || maxDist-minDist<srtiny) // creat only one surface in the middle
		{
			float depth = (minDist+maxDist)*0.5f;
			SrSnColorSurf* Surf = createFlatSurface(depth, dimensions, topLeft, lowerRight);
			surfList.push_back(Surf);
		}
		else
		{
			float distOffset = (maxDist - minDist)/segments;
			for (float depth = minDist; depth <= maxDist; depth+= distOffset)
			{
				SrSnColorSurf* Surf = createFlatSurface(depth, dimensions, topLeft, lowerRight);
				surfList.push_back(Surf);
				Surf->surfaceScale = SrVec(surfaceScale/surfSize[0], surfaceScale/surfSize[1], 1.f);
				//updateErrorSurace(Surf, center);
			}
		}
	}	
}

void SBMotionBlendBase::updateSmoothSurface( SrSnColorSurf* surf )
{
	SrModel* surfModel = surf->model();	
	std::vector<VecOfDouble> weightList;
	float maxError = 1e-30f;
	float totalError = 0.f;
	for (int i=0;i<surfModel->V.size();i++)
	{
		SrVec para = surfModel->V[i];
		std::vector<double> weights;
		getWeightsFromParameters(para[0],para[1],para[2],weights);		
		weightList.push_back(weights);		
	}


	// compute the smoothness factor based on parameter weights
	std::vector<float> smoothList;
	float maxSmooth = 1e-30f;
	float totalSmooth = 0.f;
	for (int i=0;i<surfModel->V.size();i++)
	{
		const VecOfInt& adjIdx = surf->vtxAdjList[i];
		VecOfDouble laplacian = weightList[i];
		float ratio = 1.f/adjIdx.size();
		for (unsigned int k=0;k<adjIdx.size();k++)
		{
			const VecOfDouble& adjWeight = weightList[adjIdx[k]];
			for (unsigned int idx=0;idx<laplacian.size();idx++)
			{
				laplacian[idx] -= adjWeight[idx]*ratio;
			}			
		}
		float smooth = 0.f;
		for (unsigned int idx=0;idx<laplacian.size();idx++)
		{
			smooth += (float)(laplacian[idx]*laplacian[idx]);
		}
		smooth = sqrtf(smooth);
		totalSmooth += smooth;
		if (smooth > maxSmooth) maxSmooth = smooth;
		smoothList.push_back(smooth);
	}

	totalSmooth /= smoothList.size();
	LOG("total avg smooth = %f",totalSmooth);
	SrColor temp_c;
	SrMaterial mtl;
	surfModel->M.remove(0,smoothList.size());
	for (unsigned int i=0;i<smoothList.size();i++)
	{
		//surfModel->M.push()		
		float curError = (float)(smoothList[i]/maxSmooth);
		if (curError > 1.f) curError = 1.f;

		temp_c = SrColor::interphue(curError);
		mtl.diffuse.set(temp_c.r, temp_c.g, temp_c.b, (srbyte)160);
		surfModel->M.push(mtl);	
	}
	surf->changed(true);
}


void SBMotionBlendBase::updateErrorSurace( SrSnColorSurf* errorSurf, SrVec center )
{
	SrModel* surfModel = errorSurf->model();
	std::vector<float> errorList;
	std::vector<VecOfDouble> weightList;
	float maxError = 1e-30f;
	float totalError = 0.f;
	for (int i=0;i<surfModel->V.size();i++)
	{
		SrVec para = surfModel->V[i];
		std::vector<double> weights;
		getWeightsFromParameters(para[0],para[1],para[2],weights);
		float x,y,z;
		getParametersFromWeights(x,y,z,weights);

		float paraError = (para - SrVec(x,y,z)).norm();
		if (paraError > maxError)
			maxError = paraError;
		errorList.push_back(paraError);
		weightList.push_back(weights);
		totalError += paraError;
	}

	totalError /= errorList.size();
	LOG("total avg error = %f",totalError);
	SrColor temp_c;
	SrMaterial mtl;
	maxError = 0.4f;
	surfModel->M.remove(0,errorList.size());
	for (unsigned int i=0;i<errorList.size();i++)
	{
		//surfModel->M.push()		
		float curError = (float)(errorList[i]/maxError);
		if (curError > 1.f) curError = 1.f;
		
		temp_c = SrColor::interphue(curError);
		mtl.diffuse.set(temp_c.r, temp_c.g, temp_c.b, (srbyte)160);
		surfModel->M.push(mtl);	
	}
	errorSurf->changed(true);
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