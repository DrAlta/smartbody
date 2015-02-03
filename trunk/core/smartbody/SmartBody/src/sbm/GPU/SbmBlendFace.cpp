#include "vhcl.h"
#if !defined(__FLASHPLAYER__)
#include "external/glew/glew.h"
#include "external/jpge/jpge.h"
#endif

#include <algorithm>
#include "SbmBlendFace.h"
#include "sbm/sbm_deformable_mesh.h"
#include <sr/jpge.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>
#include <sb/SBPawn.h>
#include "SbmDeformableMeshGPU.h"

#include "external/glm/glm/glm.hpp"
#include "external/glm/glm/gtc/type_ptr.hpp"
#include "external/glm/glm/gtc/matrix_transform.hpp"

SbmBlendFace::SbmBlendFace() : DeformableMesh()
{
//	_VBOPos							= NULL;
	_VBONormal						= NULL;
	_VBOTexCoord					= NULL;
	_initGPUVertexBuffer			= false;

	_faceCounter					= 0;

	_shaderName						= "BlendFace";
}

SbmBlendFace::~SbmBlendFace()
{
	for(int i = 0; i<_VBOPos.size(); i++) {
		if (_VBOPos[i]) {
			delete _VBOPos[i];
		}
	}
	_VBOPos.clear();
	
	if (_VBONormal)		delete _VBONormal;
	if (_VBOTexCoord)	delete _VBOTexCoord;
	if (_VBOTri)		delete _VBOTri;
	
	for (unsigned int i=0;i<subMeshTris.size();i++)
	{
		delete subMeshTris[i];
	}
	subMeshTris.clear();
}

void SbmBlendFace::initShader() {

	SbmShaderProgram* program		= SbmShaderManager::singleton().getShader(_shaderName);

	//	If GLSL program does not exist yet in SbmShaderManager
	if(program) 
	{
		_programID	= program->getShaderProgram();
//		LOG("Program %d exisits already", _programID);
	} 
	else
	{
//		LOG("Program does not exist yet");
		initShaderProgram();
		_programID	= SbmShaderManager::singleton().getShader(_shaderName)->getShaderProgram();
	}
	
	
}


void SbmBlendFace::setDeformableMesh(DeformableMesh* mesh)
{
	_mesh = mesh;
}

DeformableMesh* SbmBlendFace::getDeformableMesh() 
{
	return _mesh;
}

VBOVec3f* SbmBlendFace::getVBOPos(int i)
{
	return _VBOPos[i];
}

VBOVec3f* SbmBlendFace::getVBONormal()
{
	return _VBONormal;
}

VBOVec2f* SbmBlendFace::getVBOTexCoord()
{
	return _VBOTexCoord;
}

VBOVec3i* SbmBlendFace::getVBOTri()
{
	return _VBOTri;
}
	
		
bool SbmBlendFace::buildVertexBufferGPU(int number_of_shapes)
{
	bool hasGLContext = SbmShaderManager::singleton().initOpenGL() && SbmShaderManager::singleton().initGLExtension();
	if (!hasGLContext) 
		return false;

	if (_initGPUVertexBuffer)
		return true;

	_faceCounter	= 1;

	_VBOPos.resize(_faceCounter);
	_VBOPos[_faceCounter-1]	= new VBOVec3f((char*)"RestPos", VERTEX_POSITION, _mesh->posBuf, number_of_shapes);		
	_VBONormal				= new VBOVec3f((char*)"Normal", VERTEX_VBONORMAL, _mesh->normalBuf);
	_VBOTexCoord			= new VBOVec2f((char*)"TexCoord", VERTEX_TEXCOORD, _mesh->texCoordBuf);
	_VBOTri					= new VBOVec3i((char*)"TriIdx", GL_ELEMENT_ARRAY_BUFFER, _mesh->triBuf);
	
	for (unsigned int i=0; i<_mesh->subMeshList.size(); i++)
	{
		SbmSubMesh* subMesh		= _mesh->subMeshList[i];
		VBOVec3i* subMeshTriBuf = new VBOVec3i((char*)"TriIdx",GL_ELEMENT_ARRAY_BUFFER,subMesh->triBuf);
		subMeshTris.push_back(subMeshTriBuf);
	}
	
	_initGPUVertexBuffer = true;

	return true;
}

void SbmBlendFace::addFace(SbmDeformableMeshGPU* newFace) 
{
	_faceCounter++;
	_VBOPos.resize(_faceCounter);
	_VBOPos[_faceCounter-1]	= new VBOVec3f((char*)"RestPos", VERTEX_POSITION, newFace->posBuf );
}


void SbmBlendFace::addFace(SrSnModel* newFace) 
{
	SrArray<SrVec> v = newFace->shape().V;
	
	std::vector<SrVec> vertices;
	for (int i = 0; i < v.size(); i++) {
		//std::cerr << "Adding face "<< i << ": " << v[i].x << ", " << v[i].y << ", " << v[i].z << "\n";
		vertices.push_back( v[i] );
	}
	_faceCounter++;
	_VBOPos.resize(_faceCounter);
	_VBOPos[_faceCounter-1]	= new VBOVec3f((char*)"RestPos", VERTEX_POSITION, vertices );
	
}


void SbmBlendFace::initShaderProgram_Dan() {
	
	const std::string shaderVs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendFace.vert";
	const std::string shaderFs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendFace.frag";
	//const std::string shaderName= "Blend_Face";

	GLint success = 0;

		// build the shader after there is an opengl context
	_vsID = -1;
	_fsID = -1;
	
	_vsID = glCreateShader(GL_VERTEX_SHADER);
	char *vs = NULL;		
	vs = SbmShaderProgram::textFileRead(shaderVs.c_str());
	std::string shaderStrVs = vs;
	const GLchar *sourceVs = (const GLchar *)shaderStrVs.c_str();
	delete vs;
	glShaderSource(_vsID, 1, &sourceVs, NULL);	
	glCompileShader(_vsID);
	glGetShaderiv(_vsID, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		LOG("ERROR in glCompileShader(_vsID);");
	}

	//loadShaderStr(_vsID, vsShaderStr.c_str());
	
	_fsID = glCreateShader(GL_FRAGMENT_SHADER);
	char *fs = NULL;		
	fs = SbmShaderProgram::textFileRead(shaderFs.c_str());
	std::string shaderStrFs = fs;
	const GLchar *sourceFs = (const GLchar *)shaderStrFs.c_str();
	delete fs;
	glShaderSource(_fsID, 1, &sourceFs, NULL);	
	glCompileShader(_fsID);
	//loadShaderStr(fsID,fsShaderStr.c_str());
	
	glGetShaderiv(_fsID, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE) {
		LOG("ERROR in glCompileShader(_fsID);");
	}

	SbmShaderProgram::printShaderInfoLog(_vsID);
    SbmShaderProgram::printShaderInfoLog(_fsID);
    
	_programID = glCreateProgram();
	if (_vsID != -1)
		glAttachShader(_programID, _vsID);
	if (_fsID != -1)
		glAttachShader(_programID, _fsID);

	glLinkProgram(_programID);
	GLint isLinked = 0;
	glGetProgramiv(_programID, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		LOG("ERROR inglLinkProgram(_programID);;");
		SbmShaderProgram::printProgramInfoLog(_programID);
	} else {
		LOG("ProgramID: %d", _programID);
	}

	
	glDetachShader(_programID, _vsID);
	glDetachShader(_programID, _fsID);
}

void SbmBlendFace::initShaderProgram()
{
	const std::string shaderVs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendFace.vert";
	const std::string shaderFs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendFace.frag";
	//const std::string shaderName= "Blend_Face";
	

	SbmShaderManager::singleton().addShader(_shaderName.c_str(), shaderVs.c_str(), shaderFs.c_str(), true);

	SbmShaderManager::singleton().buildShaders();
	/*
	_initShader = true;()

	if (SbmShaderManager::getShaderSupport() == SbmShaderManager::SUPPORT_OPENGL_3_0)
	{
		SbmShaderManager::singleton().addShader(shaderName.c_str(), shaderVs.c_str(), shaderFs.c_str(), true);
	}
	else if (SbmShaderManager::getShaderSupport() == SbmShaderManager::SUPPORT_OPENGL_2_0)
	{
#ifdef __APPLE__
		SbmShaderManager::singleton().addShader(shaderName.c_str(), shaderVs.c_str(),shaderFs.c_str(), true);
#else
		SbmShaderManager::singleton().addShader(shaderName.c_str(), shaderVs.c_str(), shaderFs.c_str(), true);
#endif
		
	}
	else
	{
		_initShader = false;	
	}
	*/
}

SbmBlendTextures::SbmBlendTextures()
{
}

SbmBlendTextures::~SbmBlendTextures()
{
}


GLuint SbmBlendTextures::getShader(const std::string _shaderName)
{
	if(_shaderName.compare("Blend_Two_Textures") == 0)
	{
		SbmShaderProgram* program		= SbmShaderManager::singleton().getShader(_shaderName);

		//	If GLSL program does not exist yet in SbmShaderManager
		if(program) 
		{
			return program->getShaderProgram();
		}
		//	If the GLSL shader is not in the ShaderManager yet
		else
		{
			LOG("Program does not exist yet");

			const std::string shaderVs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendTextures.vert";
			const std::string shaderFs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendTextures.frag";
	
			SbmShaderManager::singleton().addShader(_shaderName.c_str(), shaderVs.c_str(), shaderFs.c_str(), true);
			SbmShaderManager::singleton().buildShaders();

			std::cout << "Program Blend_Two_Textures #" << SbmShaderManager::singleton().getShader(_shaderName)->getShaderProgram() << "compiled OK\n";
				
			return SbmShaderManager::singleton().getShader(_shaderName)->getShaderProgram();
		}
	}
	else if(_shaderName.compare("Blend_All_Textures") == 0)
	{
		SbmShaderProgram* program		= SbmShaderManager::singleton().getShader(_shaderName);

		//	If GLSL program does not exist yet in SbmShaderManager
		if(program) 
		{
			return program->getShaderProgram();
		}
		//	If the GLSL shader is not in the ShaderManager yet
		else
		{
			LOG("Program does not exist yet");

			const std::string shaderVs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendAllTextures.vert";
			const std::string shaderFs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendAllTextures.frag";
	
			SbmShaderManager::singleton().addShader(_shaderName.c_str(), shaderVs.c_str(), shaderFs.c_str(), true);
			SbmShaderManager::singleton().buildShaders();
				
			return SbmShaderManager::singleton().getShader(_shaderName)->getShaderProgram();
		}
	}
	else if(_shaderName.compare("Blend_All_Textures_Pairwise") == 0)
	{
		SbmShaderProgram* program		= SbmShaderManager::singleton().getShader(_shaderName);

		//	If GLSL program does not exist yet in SbmShaderManager
		if(program) 
		{
			return program->getShaderProgram();
		}
		//	If the GLSL shader is not in the ShaderManager yet
		else
		{
			LOG("Program does not exist yet");

			const std::string shaderVs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendAllTexturesPairwise.vert";
			const std::string shaderFs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendAllTexturesPairwise.frag";
	
			SbmShaderManager::singleton().addShader(_shaderName.c_str(), shaderVs.c_str(), shaderFs.c_str(), true);
			SbmShaderManager::singleton().buildShaders();
				
			return SbmShaderManager::singleton().getShader(_shaderName)->getShaderProgram();
		}
	}
	else if(_shaderName.compare("BlendGeometry") == 0)
	{
		SbmShaderProgram* program		= SbmShaderManager::singleton().getShader(_shaderName);

		//	If GLSL program does not exist yet in SbmShaderManager
		if(program) 
		{
			return program->getShaderProgram();
		}
		//	If the GLSL shader is not in the ShaderManager yet
		else
		{
			LOG("Program does not exist yet");

			const std::string shaderVs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendGeometry.vert";
			const std::string shaderFs	= "../../../../core/smartbody/SmartBody/src/sbm/GPU/shaderFiles/blendGeometry.frag";
	
			SbmShaderManager::singleton().addShader(_shaderName.c_str(), shaderVs.c_str(), shaderFs.c_str(), true);
			SbmShaderManager::singleton().buildShaders();
				
			return SbmShaderManager::singleton().getShader(_shaderName)->getShaderProgram();
		}
	}
	else
	{
		LOG("*** ERROR: Invalid BlendTextures shader");
		return 0;
	}
}


std::string ZeroPadNumber(int num)
{
	std::stringstream ss;
	
	// the number is converted to string with the help of stringstream
	ss << num; 
	std::string ret;
	ss >> ret;
	
	// Append zero chars
	int str_length = ret.length();
	for (int i = 0; i < 5 - str_length; i++)
		ret = "0" + ret;
	return ret;
}



/*

	if (shape->isStaticMesh())
	{
		SmartBody::SBSkeleton* skel = shape->getSkeleton();
		SmartBody::SBPawn* pawn		= skel->getPawn();

		const std::string& parentJoint = pawn->getStringAttribute("blendShape.parentJoint");
		if (parentJoint != "")
		{
			SmartBody::SBJoint* joint = skel->getJointByName(parentJoint);
			if (joint)
			{
				const SrMat& woMat = joint->gmat();
				glMultMatrix(woMat);		

				const SrVec& offsetTrans	= pawn->getVec3Attribute("blendShape.parentJointOffsetTrans");
				const SrVec& offsetRot		= pawn->getVec3Attribute("blendShape.parentJointOffsetRot");


			}
		}

		float meshScale = shape->getMeshScale();
	*/


void SbmBlendTextures::BlendGeometry(GLuint * FBODst, GLuint * texDst, std::vector<float> weights, std::vector<GLuint> texIDs, std::vector<std::string> texture_names, DeformableMeshInstance* meshInstance/*_mesh*/, GLuint program)
{
	DeformableMesh * _mesh		= meshInstance->getDeformableMesh();


	glm::mat4x4 translation	= glm::mat4x4();
	translation = glm::translate(translation, glm::vec3(20.0, 65.0, 20.0));

	if (meshInstance->isStaticMesh())
	{
		SmartBody::SBSkeleton* skel = meshInstance->getSkeleton();
		SmartBody::SBPawn* pawn		= skel->getPawn();
		const std::string& parentJoint = pawn->getStringAttribute("blendShape.parentJoint");
		if (parentJoint != "")
		{
			SmartBody::SBJoint* joint = skel->getJointByName(parentJoint);
			if (joint)
			{
				const SrVec& offsetTrans	= pawn->getVec3Attribute("blendShape.parentJointOffsetTrans");
				const SrVec& offsetRot		= pawn->getVec3Attribute("blendShape.parentJointOffsetRot");

				translation = glm::translate(translation, glm::vec3(offsetTrans.x,offsetTrans.y,offsetTrans.z ));
			}
		}
	}


	SbmShaderProgram::printOglError("SbmBlendTextures::BlendGeometry #0");

	SbmBlendFace * aux = new SbmBlendFace();

	std::vector<int> verticesUsed(_mesh->posBuf.size(), 0);
	std::vector<SrVec2> edges;
	/*
	// Creates lists of edges
	for(int i=0; i< _mesh->triBuf.size(); i++)
	{
			int id1 = _mesh->triBuf[i][0];
			int id2 = _mesh->triBuf[i][1];
			int id3 = _mesh->triBuf[i][2];
			
			edges.push_back(SrVec2(id1,id2));
			edges.push_back(SrVec2(id1,id3));
			edges.push_back(SrVec2(id3,id2));
	}

	for(int i=0; i<edges.size(); i++)
	{
		SrVec2 edge			= edges[i];
		SrVec2 edge_reverse = SrVec2(edge.y, edge.x);
		int count			= 0;
		
		count			= std::count (edges.begin(), edges.end(), edge);
		count			+= std::count (edges.begin(), edges.end(), edge_reverse);

		if(count == 1)
		{
			verticesUsed[edge.x] = 1;
			verticesUsed[edge.y] = 1;

//			std::cerr << "Vertex " << edge.x << " is in the edge.\n";
//			std::cerr << "Vertex " << edge.y << " is in the edge.\n";
		}
	}
	*/
	GLuint tbo, verticesUsedBuffer;
	glGenTextures(1, &tbo);
	
	glGenBuffers( 1, &verticesUsedBuffer);
    glBindBuffer( GL_TEXTURE_BUFFER, verticesUsedBuffer);
#if _MSC_VER == 1500
	glBufferData( GL_TEXTURE_BUFFER, verticesUsed.size() * sizeof(int), &verticesUsed.front(), GL_DYNAMIC_DRAW);
#else
	glBufferData( GL_TEXTURE_BUFFER, verticesUsed.size() * sizeof(int), verticesUsed.data(), GL_DYNAMIC_DRAW);
#endif
	
	aux->setDeformableMesh(_mesh);
	aux->buildVertexBufferGPU(weights.size());

	SrSnModel* writeToBaseModel = NULL;
	SrSnModel* baseModel		= NULL;
	bool foundBaseModel			= false;

	std::vector<SrArray<SrPnt>*> shapes;

	SrArray<SrPnt> neutralV;
	SrArray<SrPnt> visemeV;
	// find the base shape and other shapes
	std::map<std::string, std::vector<SrSnModel*> >::iterator mIter;
	for (mIter = _mesh->blendShapeMap.begin(); mIter != _mesh->blendShapeMap.end(); ++mIter)
	{
		for (size_t i = 0; i < mIter->second.size(); ++i)
		{
			if (strcmp(mIter->first.c_str(), (const char*)mIter->second[i]->shape().name) == 0)
			{
				baseModel		= mIter->second[i];
				foundBaseModel	= true;
				break;
			}
		}
		if (baseModel == NULL)
		{
			LOG("original base model cannot be found");
			continue;
		}

		neutralV	= (baseModel->shape().V);

		// Copies reference to the shape vector
		shapes.push_back(&(baseModel->shape().V));

		for (size_t i = 0; i < mIter->second.size(); ++i)
		{
			if ((i == 0) ||(!mIter->second[i]))
			{
				continue;
			}
		
			visemeV = mIter->second[i]->shape().V;
			aux->addFace(mIter->second[i]);

			// Copies reference to the shape vector
			shapes.push_back(&(baseModel->shape().V));
		}
	}
	
	const int MAX_SHAPES = 14;	// I can't enable more than 16 attributes (15 vertex buffer + 1 texture coordinate buffer)
								// NOTE: Also change #define in shader if you change this value

	std::vector<float>	usedWeights;
	std::vector<int>	usedShapeIDs;
	std::vector<int>	areas;

	GLfloat modelview_matrix[16];
	GLfloat projection_matrix[16];
	
	glGetFloatv(GL_PROJECTION_MATRIX, projection_matrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

	glUseProgram(program);
		GLuint aVertexTexcoord	= glGetAttribLocation(program, "aVertexTexcoord");
		GLuint aVertexPosition	= glGetAttribLocation(program, "aVertexPosition");

		for(int i=0; i<weights.size(); i++)
		{
			// If it is the first weight (netural shape), or wieght is > 0.000, sends this shape to shader
			if(((weights[i] > 0.0001) && (usedWeights.size() < MAX_SHAPES)) || (i == 0))
			{
				glEnableVertexAttribArray(aVertexPosition + usedWeights.size());
				aux->getVBOPos(i)->VBO()->BindBuffer();
				glVertexAttribPointer(aVertexPosition + usedWeights.size(), 3, GL_FLOAT, GL_FALSE, 0,0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				// Pushes this weight to the vector of used weights
				usedWeights.push_back(weights[i]);
				usedShapeIDs.push_back(i);

				// Areas is a vector<int> used to set which areas each shape affect (0 -> all, 1-> upper)
				// If this shape is for eye_blink, sets area to upper
				if(
					//(texture_names[i].find("eye") != std::string::npos) ||
					(texture_names[i].find("brows") != std::string::npos) 
					)
					areas.push_back(1);										
				else if(
						(texture_names[i].find("smile") != std::string::npos)||
						(texture_names[i].find("bmp") != std::string::npos)||
						(texture_names[i].find("fv") != std::string::npos) ||
						(texture_names[i].find("w") != std::string::npos) 
						)
				{
					areas.push_back(2);
				}
				else
				{
					areas.push_back(0);
				}
			}
		}

		glEnableVertexAttribArray(aVertexTexcoord);
		aux->getVBOTexCoord()->VBO()->BindBuffer();
		glVertexAttribPointer(aVertexTexcoord, 2, GL_FLOAT, GL_FALSE, 0,0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint uMatrixMV		= glGetUniformLocation(program, "uMatrixMV");
		GLuint uMatrixProj		= glGetUniformLocation(program, "uMatrixProj");
		GLuint uWeights			= glGetUniformLocation(program, "uWeights");
		GLuint uAreas			= glGetUniformLocation(program, "uAreas");
		GLuint uBorderVertices	= glGetUniformLocation(program, "uBorderVertices");
		GLuint uNumberOfShapes	= glGetUniformLocation(program, "uNumberOfShapes");
		GLuint uTranslate		= glGetUniformLocation(program, "uTranslate");
		GLuint uNeutralSampler	= glGetUniformLocation(program, "uNeutralSampler");

		int * image_array		= new int[MAX_SHAPES];
		float * w				= new float[usedWeights.size()];
		
		for(int i=0; i<MAX_SHAPES; i++)
		{
			if(i < usedWeights.size())
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, texIDs[usedShapeIDs[i]]);
				image_array[i]	= i;
				w[i]			= usedWeights[i];
			}
			// Textures not used, but we still need to pass 15 textures to the fragment shader for completeness
			else
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, texIDs[0]);
				image_array[i]	= i;
			}
		}

		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_BUFFER, tbo);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R8I, verticesUsedBuffer);


		glUniformMatrix4fv(uMatrixMV, 1, GL_FALSE, modelview_matrix);
		glUniformMatrix4fv(uMatrixProj, 1, GL_FALSE, projection_matrix);
		glUniformMatrix4fv(uTranslate, 1, GL_FALSE, glm::value_ptr(translation));
		glUniform1iv(uAreas, areas.size(), &(areas[0]));
		glUniform1fv(uWeights, usedWeights.size(), w);
		glUniform1i(uNumberOfShapes, usedWeights.size());
		glUniform1iv(uNeutralSampler, MAX_SHAPES, image_array);
		glUniform1i(uBorderVertices,  14);


		aux->subMeshTris[0]->VBO()->BindBuffer();
			glDrawElements(GL_TRIANGLES, _mesh->triBuf.size()*3 , GL_UNSIGNED_INT,0);
		aux->subMeshTris[0]->VBO()->UnbindBuffer();

		glBindTexture(GL_TEXTURE_2D, 0);
		aux->getVBOPos(0)->VBO()->UnbindBuffer();
		aux->getVBOPos(1)->VBO()->UnbindBuffer();
		aux->getVBOTexCoord()->VBO()->UnbindBuffer();

		for(int i=0; i<usedWeights.size(); i++)
		{
			glDisableVertexAttribArray(aVertexPosition + i);
		}

		glDisableVertexAttribArray(aVertexTexcoord);

	glUseProgram(0);

	glDeleteTextures(1, &tbo);
	
	glDeleteBuffers( 1, &verticesUsedBuffer);

	delete aux;

	SbmShaderProgram::printOglError("BlendGeometry FINAL");

}

void SbmBlendTextures::BlendAllAppearancesPairwise(GLuint * FBODst, GLuint * texDst, std::vector<float> weights, std::vector<GLuint> texIDs, std::vector<std::string> texture_names, GLuint program, int w, int h)
{
	int numTextures		= weights.size();

	float sumOfWeights	= 0;

	float WeightUpToNow = 0;

	for(std::vector<float>::iterator j=weights.begin(); j!=weights.end(); ++j)
		sumOfWeights += *j;

	for(int i = numTextures-1; i >= 0; i--) 
	{
		WeightUpToNow += weights[i];

		int faceArea;
//		if((texture_names[i].find("eyebrowsUp") != std::string::npos) && (weights[i] > 0.001))
//		{
//			faceArea = 1;
//			//std::cerr << "eye_blink\t" << weights[i] << "\n";
//		} 
//		else if((texture_names[i].find("smile") != std::string::npos) && (weights[i] > 0.001))
//		{
//			faceArea = 2;
//			//std::cerr << texture_names[i] <<"\t" << weights[i] << "\n";
//		}
//		else
		{
			faceArea = 0;
		}

	
		glPushMatrix();
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBODst[i]);                                                              // Bind the framebuffer object
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texDst[i], 0);              // Attach texture to FBO

			assert( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT );

			glPushAttrib(GL_ENABLE_BIT);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_LIGHTING);
				glMatrixMode (GL_PROJECTION);
				glPushMatrix();
					glLoadIdentity ();
					gluOrtho2D(-1, 1, -1, 1);
					glMatrixMode (GL_MODELVIEW);
					glPushAttrib(GL_VIEWPORT_BIT);
					glPushAttrib(GL_TEXTURE_BIT);
						glViewport(0, 0, w, h);
						glLoadIdentity ();

						glClearColor(1.0f, 1.0f, 1.0f, 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						GLuint uNumberOfTextures		= glGetUniformLocation(program, "uNumberOfTextures");
						GLuint uIteration				= glGetUniformLocation(program, "uIteration");
						GLuint uWeight					= glGetUniformLocation(program, "uWeight");
						GLuint uWeightUpToNow			= glGetUniformLocation(program, "uWeightUpToNow");
						GLuint uFaceArea				= glGetUniformLocation(program, "uFaceArea");
						GLuint uTotalWeights			= glGetUniformLocation(program, "uTotalWeights");
						GLuint uNeutralSampler			= glGetUniformLocation(program, "uNeutralSampler");
						GLuint uExpressionSampler		= glGetUniformLocation(program, "uExpressionSampler");
						GLuint uPreviousResultSampler	= glGetUniformLocation(program, "uPreviousResultSampler");

						glUseProgram(program);
						glEnable(GL_TEXTURE_2D);

						glUniform1i(uNumberOfTextures, numTextures);
						glUniform1i(uIteration, i);
						glUniform1f(uWeight, weights[i]);
						glUniform1f(uWeightUpToNow, WeightUpToNow);
						glUniform1i(uFaceArea, faceArea);
						glUniform1f(uTotalWeights, sumOfWeights);
						glUniform1i(uNeutralSampler, 0);
						glUniform1i(uExpressionSampler, 1);
						glUniform1i(uPreviousResultSampler, 2);

						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, texIDs[0]);

						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, texIDs[i]);

						//std::cerr << "uNeutral: " << texIDs[0] << "\tuExpression: " << texIDs[i];

						// if first iteration, previous result will not be used, passing a random texture just for completeness  
						if(i == numTextures-1) {
							glActiveTexture(GL_TEXTURE2);
							glBindTexture(GL_TEXTURE_2D, texIDs[1]);
							//std::cerr  << "\tprevious: " << texIDs[1] << "\tDest:" << texDst[i] << "\ti = " << i << "\n";
						} else {
							glActiveTexture(GL_TEXTURE2);
							glBindTexture(GL_TEXTURE_2D, texDst[i+1]);
							//std::cerr << "\tprevious: " << texDst[i+1] << "\tDest:" << texDst[i] <<  "\ti = " << i << "\n";
						}

						glBegin(GL_QUADS);
							glTexCoord2f(0, 1);
							glVertex3f(-1.0f, 1.0f, -0.5f);

							glTexCoord2f(0, 0);
							glVertex3f(-1.0f, -1.0f, -0.5f);

							glTexCoord2f(1, 0);
							glVertex3f(1.0f, -1.0f, -0.5f);

							glTexCoord2f(1, 1);
							glVertex3f(1.0f, 1.0f, -0.5f);
						glEnd();

					glUseProgram(0);

					glPopAttrib();                          // Pops texture bit
					glDisable(GL_TEXTURE_2D);
					glPopAttrib();							// Pops viewport information
				glMatrixMode (GL_PROJECTION);
				glPopMatrix();                              // Pops ENABLE_BIT
				glMatrixMode (GL_MODELVIEW);
			glPopAttrib();

			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);                                                                                                          // Bind the render buffer
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);         
		
			glActiveTexture(GL_TEXTURE0);
																									   // Bind the frame buffer object
		glPopMatrix();

		/*
		//	Saves USColorCode for current weight i in EXR32 format
		int channels = 3;
		std::string path =  "C:/tmp/iteration." + ZeroPadNumber(i) + ".jpg";
		GLubyte *image = (GLubyte *) malloc(512 * 512 * sizeof(GLubyte) * channels);
		glBindTexture(GL_TEXTURE_2D, texDst[i]);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		jpge::compress_image_to_jpeg_file(path.c_str(),  512, 512, 3, image);
		glBindTexture(GL_TEXTURE_2D, 0);
		delete(image);
		*/
	}
}

void SbmBlendTextures::BlendAllAppearances(GLuint FBODst, GLuint texDst, std::vector<float> weights, std::vector<GLuint> texIDs, GLuint program, int w, int h)
{
	const int MAX_SHAPES = 50;

	unsigned int numberOfShapes	= weights.size();

	if(numberOfShapes > MAX_SHAPES) {
		std::cerr << "ERROR: SbmBlendTextures::BlendAllAppearances can't handle more than 50 shapes" << "\n";
	}

	std::vector<float> weights_up_to_50(weights);
	weights_up_to_50.resize(MAX_SHAPES);
	float * weights_array = &(weights_up_to_50[0]);

	GLint texIDs_array[MAX_SHAPES];
	for(int i=0; i<MAX_SHAPES; i++) {
		texIDs_array[i] = i;
	}

	glPushMatrix();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBODst);                                                              // Bind the framebuffer object
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texDst, 0);              // Attach texture to FBO

		assert( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT );

		glPushAttrib(GL_ENABLE_BIT);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_LIGHTING);
			glMatrixMode (GL_PROJECTION);
			glPushMatrix();
				glLoadIdentity ();
				gluOrtho2D(-1, 1, -1, 1);
				glMatrixMode (GL_MODELVIEW);
				glPushAttrib(GL_VIEWPORT_BIT);
				glPushAttrib(GL_TEXTURE_BIT);
					glViewport(0, 0, w, h);
					glLoadIdentity ();

					glClearColor(1.0, 1.0, 1.0, 0);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					//GLuint texNeutralLoc	= glGetUniformLocation(program, "texNeutral");
					//GLuint texFvLoc		= glGetUniformLocation(program, "texFv");
					//GLuint texOpenLoc		= glGetUniformLocation(program, "texOpen");
					//GLuint texPBMLoc		= glGetUniformLocation(program, "texPBM");
					//GLuint texShChLoc		= glGetUniformLocation(program, "texShCh");
					//GLuint texWLoc		= glGetUniformLocation(program, "texW");
					//GLuint texWideLoc		= glGetUniformLocation(program, "texWide");
					GLuint uTextures		= glGetUniformLocation(program, "uTextures");

					//GLuint wNeutralLoc	= glGetUniformLocation(program, "wNeutral");
					//GLuint wFvLoc			= glGetUniformLocation(program, "wFv");
					//GLuint wOpenLoc		= glGetUniformLocation(program, "wOpen");
					//GLuint wPBMLoc		= glGetUniformLocation(program, "wPBM");
					//GLuint wShChLoc		= glGetUniformLocation(program, "wShCh");
					//GLuint wWLoc			= glGetUniformLocation(program, "wW");
					//GLuint wWideLoc		= glGetUniformLocation(program, "wWide");
					GLuint uWeights			= glGetUniformLocation(program, "uWeights");

					GLuint uNumberOfShapes	= glGetUniformLocation(program, "uNumberOfShapes");
				
					glUseProgram(program);
					glEnable(GL_TEXTURE_2D);

					//glUniform1f(wNeutralLoc, weights[0]);
					//glUniform1f(wFvLoc, weights[1]);
					//glUniform1f(wOpenLoc, weights[2]);
					//glUniform1f(wPBMLoc, weights[3]);
					//glUniform1f(wShChLoc, weights[4]);
					//glUniform1f(wWLoc, weights[5]);
					//glUniform1f(wWideLoc, weights[6]);
					glUniform1fv(uWeights, MAX_SHAPES, weights_array);

					glUniform1iv(uTextures, MAX_SHAPES, texIDs_array);

					glUniform1i(uNumberOfShapes, numberOfShapes);

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texIDs[0]);
					//glUniform1i(texNeutralLoc, 0);

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, texIDs[1]);
					//glUniform1i(texFvLoc, 1);

					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, texIDs[2]);
					//glUniform1i(texOpenLoc, 2);

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, texIDs[3]);
					//glUniform1i(texPBMLoc, 3);

					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, texIDs[4]);
					//glUniform1i(texShChLoc, 4);

					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D, texIDs[5]);
					//glUniform1i(texWLoc, 5);

					glActiveTexture(GL_TEXTURE6);
					glBindTexture(GL_TEXTURE_2D, texIDs[6]);
					//glUniform1i(texWideLoc, 6);

					glBegin(GL_QUADS);
						glTexCoord2f(0, 1);
						glVertex3f(-1.0f, 1.0f, -0.5f);

						glTexCoord2f(0, 0);
						glVertex3f(-1.0f, -1.0f, -0.5f);

						glTexCoord2f(1, 0);
						glVertex3f(1.0f, -1.0f, -0.5f);

						glTexCoord2f(1, 1);
						glVertex3f(1.0f, 1.0f, -0.5f);
					glEnd();

				glUseProgram(0);

				glPopAttrib();                          // Pops texture bit
				glDisable(GL_TEXTURE_2D);
				glPopAttrib();							// Pops viewport information
			glMatrixMode (GL_PROJECTION);
			glPopMatrix();                              // Pops ENABLE_BIT
			glMatrixMode (GL_MODELVIEW);
		glPopAttrib();
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);                                                                                                          // Bind the render buffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);         
		
		glActiveTexture(GL_TEXTURE0);
                                                                                                   // Bind the frame buffer object
	glPopMatrix();
}

void SbmBlendTextures::BlendTwoFBO(GLuint tex0, GLuint tex1, GLuint FBODst, GLuint texDst, float alpha, GLuint m_blendingProgram, int w, int h)
{
	glPushMatrix();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBODst);                                                              // Bind the framebuffer object
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texDst, 0);              // Attach texture to FBO

	assert( glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT );

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode (GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity ();
		gluOrtho2D(-1, 1, -1, 1);
		glMatrixMode (GL_MODELVIEW);
		glPushAttrib(GL_VIEWPORT_BIT);
		glPushAttrib(GL_TEXTURE_BIT);
				glViewport(0, 0, w, h);
				glLoadIdentity ();

				glClearColor(1.0, 1.0, 1.0, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glColor3f(0.3f, 0.42f, 0.26f);

				GLuint t1Location		= glGetUniformLocation(m_blendingProgram, "tex0");
				GLuint t2Location		= glGetUniformLocation(m_blendingProgram, "tex1");
				GLuint outLocation		= glGetUniformLocation(m_blendingProgram, "out");
				GLuint alphaLocation	= glGetUniformLocation(m_blendingProgram, "alpha");

				glUseProgram(m_blendingProgram);
				glEnable(GL_TEXTURE_2D);

				glUniform1f(alphaLocation, alpha);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tex0);
				glUniform1i(t1Location, 0);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, tex1);
				glUniform1i(t2Location, 1);

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, texDst);
				glUniform1i(outLocation, 2);

				glBegin(GL_QUADS);
						glTexCoord2f(0, 1);
						glVertex3f(-1.0f, 1.0f, -0.5f);

						glTexCoord2f(0, 0);
						glVertex3f(-1.0f, -1.0f, -0.5f);

						glTexCoord2f(1, 0);
						glVertex3f(1.0f, -1.0f, -0.5f);

						glTexCoord2f(1, 1);
						glVertex3f(1.0f, 1.0f, -0.5f);
				glEnd();

				glUseProgram(0);

				glPopAttrib();                           // Pops texture bit
				glDisable(GL_TEXTURE_2D);

				glPopAttrib();							// Pops viewport information
				glMatrixMode (GL_PROJECTION);
		glPopMatrix();                                   // Pops ENABLE_BIT
		glMatrixMode (GL_MODELVIEW);

		glPopAttrib();
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);                                                                                                          // Bind the render buffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);                                                                                                            // Bind the frame buffer object
	glPopMatrix();
}
