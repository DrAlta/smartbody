#include "vhcl.h"
#include "SBPythonAutoRig.h"
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


#ifndef SB_NO_PYTHON

void setPawnMesh(const std::string& pawnName, const std::string& meshName)
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
	}
}

void saveDeformableMesh(const std::string& meshName, const std::string& skelName, const std::string& outDir)
{
	std::vector<std::string> moNames;
	ParserOpenCOLLADA::exportCollada(outDir,skelName,meshName,moNames,true,true,false);
}

BOOST_PYTHON_MODULE(AutoRig)
{	
	boost::python::def("saveDeformableMesh", saveDeformableMesh, "Save the deformable model to the target directory");
	boost::python::def("setPawnMesh", setPawnMesh, "Set the deformable model to the target pawn");


	boost::python::class_<SBAutoRigManager>("SBAutoRigManager")
		.def("getAutoRigManager", &SBAutoRigManager::singletonPtr, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Get the autorigging manager")
		.staticmethod("getAutoRigManager")
		.def("buildAutoRiggingFromPawnMesh", &SBAutoRigManager::buildAutoRiggingFromPawnMesh, "Build the rigging from a pawn with mesh")
		;
		//.def("setHPRSmooth", &SBAutoRigManager::setHPRSmooth, "Sets the heading, pitch and roll of the character's world offset. The character will be rotated smoothly overtime to avoid popping.")
		//;	
}


void initAutoRigPythonModule()
{
	initAutoRig();
}



#endif
