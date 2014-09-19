#include "vhcl.h"
#if !defined(__FLASHPLAYER__)
#include "external/glew/glew.h"
#endif

#include "SbmBlendFace.h"
#include "sbm/sbm_deformable_mesh.h"

#include "SbmDeformableMeshGPU.h"


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
	
		
bool SbmBlendFace::buildVertexBufferGPU()
{
	bool hasGLContext = SbmShaderManager::singleton().initOpenGL() && SbmShaderManager::singleton().initGLExtension();
	if (!hasGLContext) 
		return false;

	if (_initGPUVertexBuffer)
		return true;

	_faceCounter	= 1;

	_VBOPos.resize(_faceCounter);
	_VBOPos[_faceCounter-1]	= new VBOVec3f((char*)"RestPos", VERTEX_POSITION, _mesh->posBuf);		
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
	else
	{
		LOG("*** ERROR: Invalid BlendTextures shader");
		return 0;
	}
}

void SbmBlendTextures::BlendAllAppearancesPairwise(GLuint * FBODst, GLuint * texDst, std::vector<float> weights, std::vector<GLuint> texIDs, GLuint program, int w, int h)
{
	int numTextures		= weights.size();

	float sumOfWeights	= 0;

	for(std::vector<float>::iterator j=weights.begin(); j!=weights.end(); ++j)
		sumOfWeights += *j;

	for(int i = numTextures-1; i >= 0; i--) 
	{
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

						glClearColor(1.0, 1.0, 1.0, 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						GLuint uNumberOfTextures		= glGetUniformLocation(program, "uNumberOfTextures");
						GLuint uIteration				= glGetUniformLocation(program, "uIteration");
						GLuint uWeight					= glGetUniformLocation(program, "uWeight");
						GLuint uTotalWeights			= glGetUniformLocation(program, "uTotalWeights");
						GLuint uNeutralSampler			= glGetUniformLocation(program, "uNeutralSampler");
						GLuint uExpressionSampler		= glGetUniformLocation(program, "uExpressionSampler");
						GLuint uPreviousResultSampler	= glGetUniformLocation(program, "uPreviousResultSampler");

						glUseProgram(program);
						glEnable(GL_TEXTURE_2D);

						glUniform1i(uNumberOfTextures, numTextures);
						glUniform1i(uIteration, i);
						glUniform1f(uWeight, weights[i]);
						glUniform1f(uTotalWeights, sumOfWeights);
						glUniform1i(uNeutralSampler, 0);
						glUniform1i(uExpressionSampler, 1);
						glUniform1i(uPreviousResultSampler, 2);

						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, texIDs[0]);

						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, texIDs[i]);

						//std::cerr << "uNeutralSampler: " << texIDs[0] << "\tuExpressionSampler: " << texIDs[i];

						// if first iteration, previous result will not be used, passing a random texture just for completeness  
						if(i == numTextures-1) {
							glActiveTexture(GL_TEXTURE2);
							glBindTexture(GL_TEXTURE_2D, texIDs[1]);
							//std::cerr  << "\tprevious: " << texIDs[1] <<  "\ti = " << i << "\n";
						} else {
							glActiveTexture(GL_TEXTURE2);
							glBindTexture(GL_TEXTURE_2D, texDst[i+1]);
							//std::cerr << "\tprevious: " << texDst[i+1] <<  "\ti = " << i << "\n";
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
