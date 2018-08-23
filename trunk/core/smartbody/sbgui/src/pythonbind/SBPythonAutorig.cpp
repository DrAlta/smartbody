
#include "SBPythonAutoRig.h"
#include "SBInterfaceListener.h"
#include <sb/SBObject.h>
#include <sb/SBScene.h>
#include <sb/SBAssetManager.h>
#include <sb/SBPawn.h>
#include <sbm/GPU/SbmDeformableMeshGPU.h>
#include <sbm/ParserOpenCOLLADA.h>
#include <autorig/SBAutoRigManager.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <vector>
#include <string>

#ifndef SB_NO_PYTHON
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#include <boost/python.hpp>
#endif

#include <RootWindow.h>
#include <fltk_viewer.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"



#ifndef SB_NO_PYTHON


struct SBInterfaceListenerWrap : SBInterfaceListener, boost::python::wrapper<SBInterfaceListener>
{
	virtual void onStart()
	{
		if (boost::python::override o = this->get_override("onStart"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}

		SBInterfaceListener::onStart();
	};

	void default_onStart()
	{
		SBInterfaceListener::onStart();
	}

	virtual bool onMouseClick(int x, int y, int button)
	{
		if (boost::python::override o = this->get_override("onMouseClick"))
		{
			try {
				return o(x, y, button);
			} catch (...) {
				PyErr_Print();
			}
		}

		return SBInterfaceListener::onMouseClick(x, y, button);
	};

	bool default_onMouseClick(int x, int y, int button)
	{
		return SBInterfaceListener::onMouseClick(x, y, button);
	}

	virtual bool onMouseMove(int x, int y)
	{
		if (boost::python::override o = this->get_override("onMouseMove"))
		{
			try {
				return o(x, y);
			} catch (...) {
				PyErr_Print();
			}
		}

		return SBInterfaceListener::onMouseMove(x, y);
	};

	bool default_onMouseMove(int x, int y)
	{
		return SBInterfaceListener::onMouseMove(x, y);
	}

	virtual bool onMouseRelease(int x, int y, int button)
	{
		if (boost::python::override o = this->get_override("onMouseRelease"))
		{
			try {
				return o(x, y);
			} catch (...) {
				PyErr_Print();
			}
		}

		return SBInterfaceListener::onMouseRelease(x, y, button);
	};

	bool default_onMouseRelease(int x, int y, int button)
	{
		return SBInterfaceListener::onMouseRelease(x, y, button);
	}

	virtual bool onMouseDrag(int x, int y)
	{
		if (boost::python::override o = this->get_override("onMouseDrag"))
		{
			try {
				return o(x, y);
			} catch (...) {
				PyErr_Print();
			}
		}

		return SBInterfaceListener::onMouseDrag(x, y);
	};

	bool default_onMouseDrag(int x, int y)
	{
		return SBInterfaceListener::onMouseDrag(x, y);
	}

	virtual bool onKeyboardPress(char c)
	{
		if (boost::python::override o = this->get_override("onKeyboardPress"))
		{
			try {
				return o(c);
			} catch (...) {
				PyErr_Print();
			}
		}

		return SBInterfaceListener::onKeyboardPress(c);
	};

	bool default_onKeyboardPress(char c)
	{
		return SBInterfaceListener::onKeyboardPress(c);
	}

	virtual bool onKeyboardRelease(char c)
	{
		if (boost::python::override o = this->get_override("onKeyboardRelease"))
		{
			try {
				return o(c);
			} catch (...) {
				PyErr_Print();
			}
		}

		return SBInterfaceListener::onKeyboardRelease(c);
	};

	bool default_onKeyboardRelease(char c)
	{
		return SBInterfaceListener::onKeyboardRelease(c);
	}

	virtual void onEnd()
	{
		if (boost::python::override o = this->get_override("onEnd"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}

		SBInterfaceListener::onEnd();
	};

	void default_onEnd()
	{
		SBInterfaceListener::onEnd();
	}

};




void setPawnMesh(const std::string& pawnName, const std::string& meshName, SrVec meshScale)
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::SBAssetManager* assetManager = scene->getAssetManager();
	SmartBody::SBPawn* pawn = scene->getPawn(pawnName);
	if (!pawn)
		return;

	DeformableMesh* mesh = assetManager->getDeformableMesh(meshName);
	if (!mesh)
	{
		return;
	}
	if (mesh)
	{
		pawn->dStaticMeshInstance_p = new SbmDeformableMeshGPUInstance();
		pawn->dStaticMeshInstance_p->setToStaticMesh(true);
		DeformableMeshInstance* meshInsance = pawn->dStaticMeshInstance_p;
		meshInsance->setDeformableMesh(mesh);
		//meshInsance->setSkeleton(pawn->getSkeleton());	
		meshInsance->setPawn(pawn);
		meshInsance->setMeshScale(meshScale);
	}
}

void saveDeformableMesh(const std::string& meshName, const std::string& skelName, const std::string& outDir)
{
	std::vector<std::string> moNames;
	double scale = 1.0;
	SmartBody::SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(skelName);
	if (character)
	{
		SrVec scale3 = character->getVec3Attribute("deformableMeshScale");
		scale = scale3.x;
	}

	ParserOpenCOLLADA::exportCollada(outDir, skelName, meshName, moNames, true, true, false, scale);
}

void saveDeformableMeshScale(const std::string& meshName, const std::string& skelName, const std::string& outDir, float meshScale)
{
	std::vector<std::string> moNames;
	double scale = meshScale;
	ParserOpenCOLLADA::exportCollada(outDir, skelName, meshName, moNames, true, true, false, scale);
}

inline float clamp(float x, float a, float b) { return std::max(a, std::min(b, x)); }
inline float saturate(float x) { return clamp(x, 0.f, 1.f); }

void lab2rgb(float* lab, unsigned char* rgb, unsigned int size)
{
	float temp1[16] = { 1, 1, 1, 0,
					   1, 1, -1, 0,
					   1, -2, 0, 0,
					   0, 0,  0, 0 };
	float temp2[16] = { sqrt(3.f) / 3.f, 0, 0, 0,
						0, sqrt(6.f) / 6.f, 0 , 0,
						0, 0, sqrt(2.f) / 2.f , 0,
						0, 0,  0,  0};
	SrMat m1_lab2rgb = SrMat(temp1);
	SrMat m2_lab2rgb = SrMat(temp2);

	float aLMS2RGB[16] = { 4.4679f, -3.5873f, 0.1193f , 0,
						 -1.2186f, +2.3809f, -0.1624f, 0,
						  0.0497f, -0.2439f, 1.2045f , 0,
							0,        0,        0,     0};
	SrMat mLMS2RGB = SrMat(aLMS2RGB);

	m1_lab2rgb.transpose();
	m2_lab2rgb.transpose();
	mLMS2RGB.transpose();

	for (unsigned int i = 0; i < size; i = i + 4)
	{
		float l = lab[i];
		float a = lab[i + 1];
		float b = lab[i + 2];
		SrVec lab(l, a, b);

		//Convert to LMS
		//SrVec LMS = m1_lab2rgb * m2_lab2rgb * lab;
		SrVec LMS = lab * m2_lab2rgb * m1_lab2rgb;
		LMS = SrVec(pow(10, LMS.x), pow(10, LMS.y), pow(10, LMS.z));//pow(SrVec(10), LMS);
		//Convert to RGB
		//auto rgbv = mLMS2RGB * LMS;
		auto rgbv = LMS * mLMS2RGB;
		rgb[i] = static_cast<unsigned char>(255.*saturate(rgbv.x));
		rgb[i + 1] = static_cast<unsigned char>(255.*saturate(rgbv.y));
		rgb[i + 2] = static_cast<unsigned char>(255.*saturate(rgbv.z));
	}
}

void rgb2lab(unsigned char* rgb, float* lab, unsigned int size)
{
	float temp1[16] = { 1.f / sqrt(3.f) , 0, 0, 0,  
					   0, 1.f / sqrt(6.f), 0 , 0, 
					   0, 0, 1.f / sqrt(2.f), 0,
					   0, 0, 0, 0};
	float temp2[16] = { 1, 1, 1, 0, 
						1, 1, -2, 0,
						1, -1, 0, 0,
						0, 0, 0, 0};

	SrMat m1_rgb2lab = SrMat(temp1);
	SrMat m2_rgb2lab = SrMat(temp2);

	

	float aRGB2LMS[16] = { 0.3811f, 0.5783f, 0.0402f, 0, 
						  0.1967f, 0.7244f, 0.0782f, 0,
						  0.0241f, 0.1288f, 0.8444f, 0,
						  0,       0,       0,       0};
	SrMat tmRGB2LMS = SrMat(aRGB2LMS);

	m1_rgb2lab.transpose();
	m2_rgb2lab.transpose();
	tmRGB2LMS.transpose();
	for (unsigned int i = 0; i < size; i = i + 4)
	{
		// log10(0) = -inf, so gotta use FLT_TRUE_MIN
		// otherwise, image stats are fucked up
		float r = std::max(FLT_TRUE_MIN, rgb[i] / 255.f);
		float g = std::max(FLT_TRUE_MIN, rgb[i + 1] / 255.f);
		float b = std::max(FLT_TRUE_MIN, rgb[i + 2] / 255.f);
		SrVec rgb(r, g, b);

		//Convert to LMS
		//SrVec LMS = tmRGB2LMS * rgb;
		SrVec LMS = rgb * tmRGB2LMS;
		LMS = SrVec(log10(LMS.x), log10(LMS.y), log10(LMS.z));

		//Convert to lab
		//SrVec labv = m1_rgb2lab * m2_rgb2lab  * LMS;
		SrVec labv = LMS * m2_rgb2lab * m1_rgb2lab;

		lab[i] = labv.x;
		lab[i + 1] = labv.y;
		lab[i + 2] = labv.z;
	}
}

void computeImageMeanAndStd(float* lab, unsigned char* mask, int imgSize, SrVec& outMean, SrVec& outStd)
{
	int pixCount = 0;
	SrVec imgMean = SrVec(0,0,0), imgStd = SrVec(0,0,0);
	for (unsigned int i = 0; i < imgSize; i++)
	{
		int labIdx = i * 4;
		if (mask[i] == 0) continue;
		SrVec pix = SrVec(lab[labIdx], lab[labIdx + 1], lab[labIdx + 2]);
		//SmartBody::util::log("Pix %d = %s", i, pix.toString().c_str());
		imgMean += pix;
		pixCount++;
	}

	SmartBody::util::log("Image Pix Count = %d", pixCount);
	imgMean /= pixCount;

	for (unsigned int i = 0; i < imgSize; i++)
	{
		int labIdx = i * 4;
		if (mask[i] == 0) continue;
		for (int k=0;k<3;k++)
			imgStd[k] += pow(lab[labIdx+k] - imgMean[k], 2);
	}

	for (int k = 0; k < 3; k++)
		imgStd[k] = sqrtf(imgStd[k] / pixCount);
	outMean = imgMean;
	outStd = imgStd;

	SmartBody::util::log("Image Mean = %s, Std = %s", outMean.toString().c_str(), outStd.toString().c_str());
}

void imageColorTransfer(std::string srcImg, std::string srcMask, std::string tgtImg, std::string tgtMask, std::string outImage)
{
	int srcWidth, srcHeight, srcChannel;
	int forceImgChannel = 4;
	int forceMaskChannel = 1;
	unsigned char* srcBuf = stbi_load(srcImg.c_str(), &srcWidth, &srcHeight, &srcChannel, forceImgChannel);
	unsigned char* srcMaskBuf = stbi_load(srcMask.c_str(), &srcWidth, &srcHeight, &srcChannel, forceMaskChannel);

	int tgtWidth, tgtHeight, tgtChannel;
	unsigned char* tgtBuf = stbi_load(tgtImg.c_str(), &tgtWidth, &tgtHeight, &tgtChannel, forceImgChannel);
	unsigned char* tgtMaskBuf = stbi_load(tgtMask.c_str(), &tgtWidth, &tgtHeight, &tgtChannel, forceMaskChannel);

	int srcSize = srcHeight*srcWidth;
	int tgtSize = tgtHeight*tgtWidth;
	float* srcLab = new float[srcSize*forceImgChannel];
	float* tgtLab = new float[tgtSize*forceImgChannel];

	rgb2lab(srcBuf, srcLab, srcSize*forceImgChannel);
	rgb2lab(tgtBuf, tgtLab, tgtSize*forceImgChannel);

	SrVec srcMean, srcStd, tgtMean, tgtStd;
	computeImageMeanAndStd(srcLab, srcMaskBuf, srcSize, srcMean, srcStd);
	computeImageMeanAndStd(tgtLab, tgtMaskBuf, tgtSize, tgtMean, tgtStd);

	SrVec newStdRato;
	for (int k = 0; k < 3; k++)
		newStdRato[k] = tgtStd[k] / srcStd[k];
	for (int i = 0; i < srcSize; i ++)
	{
		if (srcMaskBuf[i] == 0) continue;
		int labIdx = i * 4;
		for (int k = 0; k < 3; k++)
		{
			// adjust source image in LAB space based on target image's Mean and Std
			srcLab[labIdx + k] = (srcLab[labIdx + k] - srcMean[k])*newStdRato[k] + tgtMean[k];
		}
	}
	lab2rgb(srcLab, srcBuf, srcSize*forceImgChannel);
	int imageWriteSuccess = stbi_write_png(outImage.c_str(), srcWidth, srcHeight, 4, srcBuf, srcWidth*4);
	SmartBody::util::log("Writing PNG %s, result = %d", outImage.c_str(), imageWriteSuccess);
}




void createCustomMeshFromBlendshapes(std::string templateMeshName, std::string blendshapesDir, std::string baseMeshName, std::string hairMeshName, std::string outMeshName)
{
	SmartBody::SBAssetManager* assetManager = SmartBody::SBScene::getScene()->getAssetManager();
	DeformableMesh* mesh = assetManager->getDeformableMesh(templateMeshName);
	if (!mesh)
	{
		SmartBody::util::log("Error creating custom mesh from blendshapes :: mesh '%s' does not exist.", templateMeshName.c_str());
		return;
	}

	for (std::map<std::string, std::vector<SrSnModel*> >::iterator iter = mesh->blendShapeMap.begin();
		iter != mesh->blendShapeMap.end();
		iter++)
	{
		std::vector<SrSnModel*>& targets = (*iter).second;
		for (size_t t = 0; t < targets.size(); t++) // ignore first target since it is a base mesh
		{
			if (targets[t] == NULL)
				continue;
			SrModel& curModel = targets[t]->shape();
			SrModel newShape;
			std::string blendshapeName = (const char*) curModel.name;

			if (t == 0)
				blendshapeName = baseMeshName;
			//SmartBody::util::log("mesh '%s', blendShapeName = '%s'", templateMeshName.c_str(), blendshapeName.c_str());
			std::string meshFileNamae = blendshapesDir + "/" + blendshapeName;
			newShape.import_ply(meshFileNamae.c_str());
			if (newShape.V.size() == curModel.V.size())
			{
				curModel.V = newShape.V;
				curModel.N = newShape.N;
			}
			if (t == 0)
			{
				SrModel& staticBaseModel = mesh->dMeshStatic_p[0]->shape();
				staticBaseModel.V = newShape.V;
				staticBaseModel.N = newShape.N;

				SrModel& dynamicBaseModel = mesh->dMeshDynamic_p[0]->shape();
				dynamicBaseModel.V = newShape.V;
				dynamicBaseModel.N = newShape.N;

				SrModel hairModel;
				std::string hairFileName = blendshapesDir + "/" + hairMeshName;
				hairModel.import_ply(hairFileName.c_str());

				SrSnModel* hairSrSn = new SrSnModel();
				std::string hairName = "HairMesh";
				hairSrSn->shape(hairModel);
				hairSrSn->shape().name = hairName.c_str();
				mesh->dMeshStatic_p.push_back(hairSrSn);

				SkinWeight* hairSkin = new SkinWeight();
				SkinWeight* headSkin = mesh->skinWeights[0];
				SrMat hairBindShape = headSkin->bindShapeMat;
				SrMat hairBindPose;
				std::string headJointName = "Head";
				SmartBody::util::log("headSkin bindShape Mat = %s", hairBindShape.toString().c_str());
				for (unsigned int i = 0; i < headSkin->infJointName.size(); i++)
				{
					if (headSkin->infJointName[i] == headJointName)
					{
						//SmartBody::util::log("headSkin inf joint %d : '%s'", i, headSkin->infJointName[i].c_str());
						hairBindPose = headSkin->bindPoseMat[i];
						//SmartBody::util::log("headSkin bindPose Mat %d = %s", i, bindPose.toString().c_str());
					}
				}

				hairSkin->bindShapeMat = hairBindShape;
				hairSkin->bindPoseMat.push_back(hairBindPose);
				hairSkin->infJointName.push_back(headJointName);
				hairSkin->sourceMesh = hairName;
				hairSkin->bindWeight.push_back(1.0f);
				for (unsigned int i = 0; i < hairModel.V.size(); i++)
				{
					hairSkin->jointNameIndex.push_back(0);
					hairSkin->numInfJoints.push_back(1);
					hairSkin->weightIndex.push_back(0);
				}
				mesh->skinWeights.push_back(hairSkin);
			}
		}
	}

	// handle hair mesh
	


	std::string outputMeshFile = blendshapesDir + "/" + outMeshName;
	mesh->saveToDmb(outputMeshFile);
	// load base model
	//mesh->rebuildVertexBuffer(true);
}

//	Callback function for Python module Misc to run the checkVisibility function
std::vector<std::string> checkVisibility(const std::string& character)
{
	bool DEBUG_CHECK_VISIBILITY			= true;
	
	SmartBody::SBScene* scene			= SmartBody::SBScene::getScene();

	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window && window->curViewer)
		window->curViewer->make_current(); // make sure the OpenGL context is current

	std::vector<std::string> visible	= scene->checkVisibility(character);
	
	if(DEBUG_CHECK_VISIBILITY) {
		SmartBody::util::log ("Visible pawns from %s: ", character.c_str());
		for( std::vector<std::string>::const_iterator i = visible.begin(); 
			 i != visible.end(); 
			 i++)
		{
			SmartBody::util::log("%s, ", (*i).c_str());
		}
	}

	return visible;
}

//	Callback function for Python module Misc to run the checkVisibility function
std::vector<std::string> checkVisibility_current_view()
{
	bool DEBUG_CHECK_VISIBILITY			= true;
	
	SmartBody::SBScene* scene			= SmartBody::SBScene::getScene();

	// make current
	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window && window->curViewer)
		window->curViewer->make_current(); // make sure the OpenGL context is current
	
	std::vector<std::string> visible	= scene->checkVisibility_current_view();

	if(DEBUG_CHECK_VISIBILITY) {
		SmartBody::util::log("Visible pawns: ");
		for( std::vector<std::string>::const_iterator i = visible.begin(); i != visible.end(); ++i)  {
			SmartBody::util::log("%s, ", i);
		}
	}

	return visible;
}

void addPoint(const std::string& pointName, SrVec point, SrVec color, int size)
{
	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window->curViewer)
	{
		window->curViewer->addPoint(pointName, point, color, size);
	}
}

void removePoint(const std::string& pointName)
{
	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window->curViewer)
	{
		window->curViewer->removePoint(pointName);
	}
}

void removeAllPoints()
{
	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window->curViewer)
	{
		window->curViewer->removeAllPoints();
	}
}

void addLine(const std::string& lineName, std::vector<SrVec>& points, SrVec color, int width)
{
	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window->curViewer)
	{
		window->curViewer->addLine(lineName, points, color, width);
	}
}

void removeLine(const std::string& lineName)
{
	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window->curViewer)
	{
		window->curViewer->removeLine(lineName);
	}
}

void removeAllLines()
{
	BaseWindow* window = dynamic_cast<BaseWindow*>(SmartBody::SBScene::getScene()->getViewer());
	if (window->curViewer)
	{
		window->curViewer->removeAllLines();
	}
}

BOOST_PYTHON_MODULE(GUIInterface)
{
	boost::python::def("addPoint", addPoint, "addPoint");
	boost::python::def("removePoint", removePoint, "removePoint");
	boost::python::def("removeAllPoints", removePoint, "removeAllPoints");
	boost::python::def("addLine", addLine, "addLine");
	boost::python::def("removeLine", removeLine, "removeLine");
	boost::python::def("removeAllLines", removeAllLines, "removeAllLines");



	boost::python::class_<SBInterfaceListenerWrap, boost::noncopyable> ("SBInterfaceListener")
		.def(boost::python::init<>())
		.def("onStart", &SBInterfaceListener::onStart, "onStart")
		.def("onMouseClick", &SBInterfaceListener::onMouseClick, &SBInterfaceListenerWrap::default_onMouseClick, "onMouseClick")
		.def("onMouseMove", &SBInterfaceListener::onMouseMove, &SBInterfaceListenerWrap::default_onMouseMove, "onMouseMove")
		.def("onMouseRelease", &SBInterfaceListener::onMouseRelease, &SBInterfaceListenerWrap::default_onMouseRelease, "onMouseRelease")
		.def("onMouseDrag", &SBInterfaceListener::onMouseDrag, &SBInterfaceListenerWrap::default_onMouseDrag, "onMouseDrag")
		.def("onKeyboardPress", &SBInterfaceListener::onKeyboardPress, &SBInterfaceListenerWrap::default_onKeyboardPress, "onKeyboardPress")
		.def("onKeyboardRelease", &SBInterfaceListener::onKeyboardRelease, &SBInterfaceListenerWrap::default_onKeyboardRelease, "onKeyboardRelease")
		.def("onEnd", &SBInterfaceListener::onEnd, "onEnd")
	;

	boost::python::def("getInterfaceManager", SBInterfaceManager::getInterfaceManager, boost::python::return_value_policy<boost::python::reference_existing_object>(),"Gets the interface manager.");
	
	boost::python::class_<SBInterfaceManager, boost::noncopyable> ("SBInterfaceManager")
		.def("addInterfaceListener", &SBInterfaceManager::addInterfaceListener, "Adds an interface listener.")
		.def("removeInterfaceListener", &SBInterfaceManager::removeInterfaceListener, "Removes an interface listener.")
		.def("convertScreenSpaceTo3D", &SBInterfaceManager::convertScreenSpaceTo3D, "Converts screen space to 3D space given a point on a plane and a normal to that plane.")
		.def("getSelectedObject", &SBInterfaceManager::getSelectedObject, "Returns the name of the currently selection object.")
;
}


BOOST_PYTHON_MODULE(AutoRig)
{	
	boost::python::def("saveDeformableMesh", saveDeformableMesh, "Save the deformable model to the target directory");
	boost::python::def("saveDeformableMeshScale", saveDeformableMeshScale, "Save the deformable model with scaling factor to the target directory");
	boost::python::def("setPawnMesh", setPawnMesh, "Set the deformable model to the target pawn");
	boost::python::def("createCustomMeshFromBlendshapes", createCustomMeshFromBlendshapes, "create a new custom mesh with a different set of blendshapes.");
	boost::python::def("imageColorTransfer", imageColorTransfer, "color transfer of source image using the color styles of target image.");


	boost::python::class_<SBAutoRigManager>("SBAutoRigManager")
		.def("getAutoRigManager", &SBAutoRigManager::singletonPtr, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Get the autorigging manager")
		.staticmethod("getAutoRigManager")
		.def("buildAutoRiggingFromPawnMesh", &SBAutoRigManager::buildAutoRiggingFromPawnMesh, "Build the rigging from a pawn with mesh")
		;
		//.def("setHPRSmooth", &SBAutoRigManager::setHPRSmooth, "Sets the heading, pitch and roll of the character's world offset. The character will be rotated smoothly overtime to avoid popping.")
		//;	
}

BOOST_PYTHON_MODULE(Misc)
{	
	boost::python::def("checkVisibility", checkVisibility, boost::python::return_value_policy<boost::python::return_by_value>(), "Lists visible pawns for a given character");
	boost::python::def("checkVisibility_current_view", checkVisibility_current_view, boost::python::return_value_policy<boost::python::return_by_value>(), "Lists visible pawns from current viewport");

}

void initGUIInterfacePythonModule()
{
	initGUIInterface();
}

void initMiscPythonModule()
{
	initMisc();
}

void initAutoRigPythonModule()
{
	initAutoRig();
}



#endif
