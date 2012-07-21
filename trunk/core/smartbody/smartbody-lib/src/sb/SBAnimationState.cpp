#include "SBAnimationState.h"
#include <sb/SBMotion.h>
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBAnimationBlend::SBAnimationBlend() : PABlend()
{
	_isFinalized = false;
}

SBAnimationBlend::SBAnimationBlend(const std::string& name) : PABlend(name)
{
	_isFinalized = false;
}

SBAnimationBlend::~SBAnimationBlend()
{
}

SrSnColorSurf* SBAnimationBlend::createFlatSurface( float depth, unsigned int dimension, SrVec2 topLeft, SrVec2 lowerRight )
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


SrSnColorSurf* SBAnimationBlend::createCurveSurface( float radius, unsigned int dimension, SrVec center, SrVec2 phiRange, SrVec2 thetaRange )
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

void SBAnimationBlend::createErrorSurfaces( const std::string& type, SrVec center, int segments, int dimensions, std::vector<SrSnColorSurf*>& surfList )
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

void SBAnimationBlend::updateSmoothSurface( SrSnColorSurf* surf )
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


void SBAnimationBlend::updateErrorSurace( SrSnColorSurf* errorSurf, SrVec center )
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
	//maxError = 0.4f;
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




void SBAnimationBlend::buildVisSurfaces( const std::string& errorType, const std::string& surfaceType, int segments, int dimensions )
{
	if (errorType != "error" && errorType != "smooth")
	{
		LOG("Warning : errorType must be 'error' or 'smooth'");
		return;
	}
	if (_dimension != "3D")
	{
		LOG("Warning : build Vis Surface only works for 3D parameterization state");
	}

	std::vector<SrSnColorSurf*>& surfaces = (errorType == "error") ? errorSurfaces : smoothSurfaces;
	//SBSkeleton* sbSkel = SBScene::getScene()->getSkeleton(skeletonName);
	SrVec center = SrVec(0,0,0);
	//if (sbSkel && surfaceType == "curve")
	//	center = sbSkel->getJointByName("base")->gmat().get_translation();
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


void SBAnimationBlend::addCorrespondencePoints(const std::vector<std::string>& motionNames, const std::vector<double>& points)
{
	if (motions.size() == 0)
	{
		LOG("Add motions before add correspondence points for state");
		return;
	}
	if (motionNames.size() != motions.size())
	{
		LOG("Add correspondence points error, input motion number is not the same with that when adding motions");
		return;		
	}
	for (size_t i = 0; i < motionNames.size(); i++)
	{
		if (motionNames[i] != motions[i]->getName())
		{
			LOG("Add correspondence points error, input motion names are not in the same order with that when adding motions");
			return;
		}
	}
	if (motionNames.size() != points.size())
	{
		LOG("Add correspondence points error, input motion number is not the same with points number!");
		return;
	}	
	int num = motionNames.size();

	// find the right place to insert the keys
	int insertPosition = -1;
	if (keys.size() > 0)
	{
		for (size_t i = 0; i < keys[0].size(); i++)
		{
			if (points[0] <= keys[0][i])
			{
				insertPosition = i;
				break;
			}
		}
		if (insertPosition == -1)
		{
			insertPosition = keys[0].size();
		}
	}

	for (int i = 0; i < num; i++)
	{
		keys[i].insert(keys[i].begin() + insertPosition, points[i]);
	}

	validateCorrespondencePoints();
}

void SBAnimationBlend::setCorrespondencePoints(int motionIndex, int pointIndex, double value)
{
	if (motionIndex < 0 || pointIndex < 0 || (keys.size() == 0) || (pointIndex >= (int) keys[0].size()))
		return;

	keys[motionIndex][pointIndex] = value;
	validateCorrespondencePoints();
}

void SBAnimationBlend::removeCorrespondencePoints(int index)
{
	if (index < 0 || (keys.size() == 0) || (index >= (int) keys[0].size()))
		return;

	for (std::vector< std::vector<double> >::iterator iter = keys.begin();
		 iter != keys.end();
		 iter++)
	{
		std::vector<double>& keyArray = (*iter);
		keyArray.erase(keyArray.begin() + index);
	}	
}

int SBAnimationBlend::getNumMotions()
{
	return motions.size();
}

std::string SBAnimationBlend::getMotion(int num)
{
	if (motions.size() > (size_t) num && num >= 0)
	{
		return motions[num]->getName();
	}
	else
	{
		return "";
	}
}

int SBAnimationBlend::getNumCorrespondencePoints()
{
	return getNumKeys();
}

std::vector<double> SBAnimationBlend::getCorrespondencePoints(int num)
{
	if (keys.size() > (size_t) num && num >= 0)
	{
		return keys[num];
	}
	else
	{
		return std::vector<double>();
	}
}

std::string SBAnimationBlend::getDimension()
{
	return _dimension;
}

void SBAnimationBlend::removeMotion(const std::string& motionName)
{
	SmartBody::SBMotion* motion = SmartBody::SBScene::getScene()->getMotion(motionName);
	if (!motion)
	{
		LOG("No motion named %s found, cannot remove from state %s.", motionName.c_str(), this->stateName.c_str());
	}
}

bool SBAnimationBlend::addSkMotion(const std::string& motion)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SkMotion* skMotion = mcu.getMotion(motion);
	if (skMotion)
	{
		motions.push_back(skMotion);
/*		
		// adding two correspondence points when the first motion got inserted
		if (motions.size() == 1)
		{
			std::vector<double> keyVec;
			keyVec.resize(2);
			keyVec[0] = 0.0f;
			keyVec[1] = skMotion->duration();
			keys.push_back(keyVec);
		}
		else
*/
		{
			// add a zero-correspondence point for this new motion
			int numPoints = 0;
			if (keys.size() > 0)
				numPoints  = keys[0].size();
			std::vector<double> keyVec;
			if (numPoints > 0)
			{
				keyVec.resize(numPoints);
				double duration = skMotion->duration();
				if (numPoints >= 2)
				{
					keyVec[0] = 0.0f;
					keyVec[numPoints - 1] = duration;
				}
				// uniformly space the correspondence points
				double step = duration / double(numPoints);
				for (int i = 1; i < numPoints - 1; i++)
				{
					keyVec[i] = double(i) * step;
				}
			}
			keys.push_back(keyVec);
		}

		getParameters().push_back(SrVec());
	}
	else
	{
		LOG("SBAnimationBlend add sk motion failure, %s doesn't exist", motion.c_str());
		return false;
	}
	return true;
}

bool SBAnimationBlend::removeSkMotion(const std::string& motionName)
{
	// find the index of the motion
	int index = -1;
	for (int i = 0; i < getNumMotions(); i++)
	{
		SkMotion* m = motions[i];
		if (m->getName() == motionName)
		{
			index = i;
			break;
		}
	}
	if (index < 0)
	{
		LOG("SBAnimationBlend delete motion failure, %s doesn't exist", motionName.c_str());
		return false;
	}

	// first delete corresponding time markers
	removeParameter(motionName);

	// delete the motion and correspondence point
	motions.erase(motions.begin() + index);
	keys.erase(keys.begin() + index);
	return true;
}

/*
	P.S. This is organized way, but is not a efficient way to do it
*/
void SBAnimationBlend::validateCorrespondencePoints()
{
	for (int i = 0; i < getNumMotions(); i++)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SkMotion* skMotion = mcu.lookUpMotion(motions[i]->getName().c_str());		
		for (int j = 1; j < getNumCorrespondencePoints(); j++)
		{
			if (keys[i][j] < keys[i][j - 1])
			{
				for (int k = j; k < getNumCorrespondencePoints(); k++)
					keys[i][k] += skMotion->duration();
			}
		}
	}
}

bool SBAnimationBlend::validateState()
{
	if (_isFinalized)
		return true;

	for (int i=0; i < getNumMotions(); i++)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SkMotion* skMotion = mcu.lookUpMotion(motions[i]->getName().c_str());		
		if ((int)keys.size() < i) // no keys for this state
		{			
			keys.push_back(std::vector<double>());			
		}		
		std::vector<double>& keyVec = keys[i];
		if (keyVec.size() == 0) // if no keys for the motion, automatically set up this based on motion duration
		{
			keyVec.push_back(0.0);
			keyVec.push_back(skMotion->duration());
		}
	}
	_isFinalized = true;
	return true;
}

void SBAnimationBlend::addEvent(const std::string& motion, double time, const std::string& type, const std::string& parameters, bool onceOnly)
{
	MotionEvent* motionEvent = new MotionEvent();
	motionEvent->setIsOnceOnly(onceOnly);
	motionEvent->setTime(time);
	motionEvent->setType(type);
	motionEvent->setParameters(parameters);
	addEventToMotion(motion, motionEvent);
}

void SBAnimationBlend::removeEvent(int index)
{
	if (index < 0 || (int) _events.size() > index)
		return;
}

MotionEvent* SBAnimationBlend::getEvent(int index)
{
	if (index < 0 || (int) _events.size() > index)
		return NULL;

	return _events[index].first;
}

void SBAnimationBlend::removeAllEvents()
{
	for (std::vector<std::pair<MotionEvent*, int> >::iterator iter = _events.begin();
		 iter != _events.end();
		 iter++)
	{
		delete (*iter).first;
	}
	_events.clear();
}

int SBAnimationBlend::getNumEvents()
{
	return _events.size();
}

std::string SBAnimationBlend::saveToString()
{
	SmartBody::SBAnimationBlend* state = this;
	SmartBody::SBAnimationBlend0D* state0D = dynamic_cast<SmartBody::SBAnimationBlend0D*>(state);
	SmartBody::SBAnimationBlend1D* state1D = dynamic_cast<SmartBody::SBAnimationBlend1D*>(state);
	SmartBody::SBAnimationBlend2D* state2D = dynamic_cast<SmartBody::SBAnimationBlend2D*>(state);
	SmartBody::SBAnimationBlend3D* state3D = dynamic_cast<SmartBody::SBAnimationBlend3D*>(state);

	std::string stateNameVariable = state->stateName + "State";
	std::stringstream strstr;
	strstr << "# blend " << stateName << "\n";
	strstr << "# autogenerated by SmartBody\n";
	strstr << "\n";
	strstr << "blendManager = scene.getBlendManager()\n";
	strstr << "\n";
	strstr << "# align motions first if needed\n";
	for (int i = 0; i < state->getNumMotions(); i++)
	{
		SBMotion* motion = dynamic_cast<SBMotion*>(state->motions[i]);
		if (!motion)
			continue;
		int alignIndex = motion->getAlignIndex();
		if (alignIndex > 0)
		{
			strstr << "alignmotion = scene.getMotion(\"" << state->getMotionName(i) << "\")\n";
			strstr << "alignmotion.alignToBegin(" << alignIndex << ")\n";
		}
		else if (alignIndex < 0)
		{
			strstr << "alignmotion = scene.getMotion(\"" << state->getMotionName(i) << "\")\n";
			strstr << "alignmotion.alignToEnd(" << -alignIndex << ")\n";
		}
	}

	if (state0D || state1D || state2D || state3D)
	{
		strstr << "\n";
		// add the motions
		if (state0D)
		{
			strstr << stateNameVariable << " = blendManager.createBlend0D(\"" << stateName << "\")\n";		
		}
		if (state1D)
		{
			strstr << stateNameVariable << " = blendManager.createBlend1D(\"" << stateName << "\")\n";
		}
		else if (state2D)
		{
			strstr << stateNameVariable << " = blendManager.createBlend2D(\"" << stateName << "\")\n";
		}
		else if (state3D)
		{
			strstr << stateNameVariable << " = blendManager.createBlend3D(\"" << stateName << "\")\n";
		}

		strstr << "\n";
		strstr << "motions = StringVec()\n";
		for (int x = 0; x < state->getNumMotions(); x++)
		{
			strstr << "motions.append(\"" << state->getMotion(x) << "\")\n";
		}
		// add the parameters
		strstr << "\n";
		if (state1D || state2D || state3D)
		{
			strstr << "paramsX = DoubleVec()\n";
		}
		if (state2D || state3D)
		{
			strstr << "paramsY = DoubleVec()\n";
		}
		if (state3D)
		{
			strstr << "paramsZ = DoubleVec()\n";
		}

		for (int x = 0; x < state->getNumMotions(); x++)
		{
			double p1, p2, p3;
			if (state1D)
			{
				state->getParameter(state->getMotion(x), p1);
				strstr << "paramsX.append(" << p1 << ") # " << state->getMotion(x) << " X\n";
			}
			else if (state2D)
			{
				state->getParameter(state->getMotion(x), p1, p2);
				strstr << "paramsX.append(" << p1 << ") # " << state->getMotion(x) << " X\n";
				strstr << "paramsY.append(" << p2 << ") # " << state->getMotion(x) << " Y\n";
			}
			else if (state3D)
			{
				state->getParameter(state->getMotion(x), p1, p2, p3);
				strstr << "paramsX.append(" << p1 << ") # " << state->getMotion(x) << " X\n";
				strstr << "paramsY.append(" << p2 << ") # " << state->getMotion(x) << " Y\n";
				strstr << "paramsZ.append(" << p3 << ") # " << state->getMotion(x) << " Z\n";
			}

		}
		strstr << "for i in range(0, len(motions)):\n";
		if (state0D)
		{
			strstr << "\t" << stateNameVariable << ".addMotion(motions[i])\n";
		}
		else if (state1D)
		{
			strstr << "\t" << stateNameVariable << ".addMotion(motions[i], paramsX[i])\n";
		}
		else if (state2D)
		{
			strstr << "\t" << stateNameVariable << ".addMotion(motions[i], paramsX[i], paramsY[i])\n";
		}
		else if (state3D)
		{
			strstr << "\t" << stateNameVariable << ".addMotion(motions[i], paramsX[i], paramsY[i], paramsZ[i])\n";
		}
		// add the correspondence points
		strstr << "\n";
		for (int c = 0; c < state->getNumKeys(); c++)
		{
			strstr << "points" << c << " = DoubleVec()\n";
			for (int m = 0; m < state->getNumMotions(); m++)
			{
				strstr << "points" << c << ".append(" << state->keys[m][c] << ") # " << state->getMotion(m) << " " << c << "\n";
			}
			strstr << stateNameVariable << ".addCorrespondencePoints(motions, points" << c << ")\n";
		}

	}
	if (state2D)
	{
		// create the triangles
		strstr << "\n";
		std::vector<TriangleInfo>& triangleInfo = state->getTriangles();
		for (size_t t = 0; t < triangleInfo.size(); t++)
		{
			strstr << stateNameVariable << ".addTriangle(\"" << triangleInfo[t].motion1 << "\", \"" <<  triangleInfo[t].motion2 << "\", \"" <<  triangleInfo[t].motion3 << "\")\n"; 
		}
	}
	if (state3D)
	{
		// create the tetrahedrons
		strstr << "\n";
		std::vector<TetrahedronInfo>& tetrahedronInfo = state->getTetrahedrons();
		for (size_t t = 0; t < tetrahedronInfo.size(); t++)
		{
			strstr << stateNameVariable << ".addTetrahedron(\"" << tetrahedronInfo[t].motion1 << "\", \"" <<  tetrahedronInfo[t].motion2 << "\", \"" <<  tetrahedronInfo[t].motion3 << "\", \"" <<  tetrahedronInfo[t].motion4 << "\")\n"; 
		}
	}
	return strstr.str();
}

SkMotion* SBAnimationBlend::getSkMotion( const std::string& motionName )
{
	for(int i=0; i<getNumMotions(); i++)
	{
		SkMotion* m = motions[i];
		if (m->getName() == motionName)
			return m;
	}
	// not found!
	LOG("Error: SBAnimationBlend::getSkMotion(): %s doesn't exist", motionName.c_str());
	return 0;
}

SBAnimationBlend0D::SBAnimationBlend0D() : SBAnimationBlend("unknown")
{
}

SBAnimationBlend0D::SBAnimationBlend0D(const std::string& name) : SBAnimationBlend(name)
{
	_dimension = "0D";
}

SBAnimationBlend0D::~SBAnimationBlend0D()
{
}

void SBAnimationBlend0D::addMotion(const std::string& motion)
{
	addSkMotion(motion);	
}

void SBAnimationBlend0D::removeMotion(const std::string& motion)
{
	SBAnimationBlend::removeMotion(motion);

	// remove correspondence points
	removeSkMotion(motion);
}

SBAnimationBlend1D::SBAnimationBlend1D() : SBAnimationBlend("unknown")
{
}


SBAnimationBlend1D::SBAnimationBlend1D(const std::string& name) : SBAnimationBlend(name)
{
	_dimension = "1D";
}

SBAnimationBlend1D::~SBAnimationBlend1D()
{
}

void SBAnimationBlend1D::addMotion(const std::string& motion, float parameter)
{
	addSkMotion(motion);

	setType(0);
	setParameter(motion, parameter);
}

void SBAnimationBlend1D::removeMotion(const std::string& motionName)
{
	SBAnimationBlend::removeMotion(motionName);

	// remove correspondnce points
	removeSkMotion(motionName);
}

void SBAnimationBlend1D::setParameter(const std::string& motion, float parameter)
{
	PABlend::setParameter(motion, parameter);
}

SBAnimationBlend2D::SBAnimationBlend2D() : SBAnimationBlend("unknown")
{
}

SBAnimationBlend2D::SBAnimationBlend2D(const std::string& name) : SBAnimationBlend(name)
{
	_dimension = "2D";
}

SBAnimationBlend2D::~SBAnimationBlend2D()
{
}

void SBAnimationBlend2D::addMotion(const std::string& motion, float parameter1, float parameter2)
{
	addSkMotion(motion);
	
	setType(1);
	PABlend::setParameter(motion, parameter1, parameter2);
}

void SBAnimationBlend2D::removeMotion(const std::string& motionName)
{
	SBAnimationBlend::removeMotion(motionName);

	// remove correspondence points
	removeSkMotion(motionName);

	// do something about triangle
	removeTriangles(motionName);
}

void SBAnimationBlend2D::setParameter(const std::string& motion, float parameter1, float parameter2)
{
	PABlend::setParameter(motion, parameter1, parameter2);
}

void SBAnimationBlend2D::addTriangle(const std::string& motion1, const std::string& motion2, const std::string& motion3)
{
	PABlend::addTriangle(motion1, motion2, motion3);
}

SBAnimationBlend3D::SBAnimationBlend3D() : SBAnimationBlend("unknown")
{
}


SBAnimationBlend3D::SBAnimationBlend3D(const std::string& name) : SBAnimationBlend(name)
{
	_dimension = "3D";
}

SBAnimationBlend3D::~SBAnimationBlend3D()
{
}


void SBAnimationBlend3D::addMotion(const std::string& motion, float parameter1, float parameter2, float parameter3)
{
	addSkMotion(motion);
	
	setType(1);
	setParameter(motion, parameter1, parameter2, parameter3);
}

void SBAnimationBlend3D::removeMotion(const std::string& motionName)
{
	SBAnimationBlend::removeMotion(motionName);

	// remove correspondence points
	removeSkMotion(motionName);

	// do something about tetrahedrons
	removeTetrahedrons(motionName);
}

void SBAnimationBlend3D::setParameter(const std::string& motion, float parameter1, float parameter2, float parameter3)
{
	PABlend::setParameter(motion, parameter1, parameter2, parameter3);
}

void SBAnimationBlend3D::addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4)
{
	PABlend::addTetrahedron(motion1, motion2, motion3, motion4);
}

}