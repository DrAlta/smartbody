#pragma once

#if __native_client__
#include<GLES2/gl2.h>
#else
#include "external/glew/glew.h"
#endif
#include <sr/sr_viewer.h>
#include <vector>
#include <string>
#include <map>

class SbmShaderProgram
{
protected:
	GLuint     vsID, fsID, programID;
	std::string     vsFilename, fsFilename;
	std::string     vsShaderStr, fsShaderStr;
	bool       isBuilt;
public:
	SbmShaderProgram();
	~SbmShaderProgram();		
	void initShaderProgram(const char* vsName, const char* fsName);		
	void initShaderProgramStr(const char* shaderVS, const char* shaderFS);	
	GLuint getShaderProgram() { return programID; }

	void buildShader();
	bool finishBuild() { return isBuilt; }
public:
	static char *textFileRead(const char *fn); // text file reading 	
	static void printShaderInfoLog(GLuint obj);
	static void printProgramInfoLog(GLuint obj);
	static void printOglError(const char* tag);
protected:
	void loadShader(GLuint sID, const char* shaderFileName);
	void loadShaderStr(GLuint sID, const char* shaderStr);
};


class SbmShaderManager
{
public:
	enum { SUPPORT_OPENGL_3_0, SUPPORT_OPENGL_2_0, NO_GPU_SUPPORT };
protected:
	std::map<std::string,SbmShaderProgram*> shaderMap;
	SrViewer* viewer;
	bool shaderInit;	
	static int shaderSupport;
private:
	// for singleton
	static SbmShaderManager* _singleton;
	SBAPI SbmShaderManager(void);
	SBAPI ~SbmShaderManager(void);
public:
	SBAPI static SbmShaderManager& singleton() 
	{
		if (!_singleton)
			_singleton = new SbmShaderManager();
		return *_singleton;			
	}

	SBAPI static void destroy_singleton() {
		if( _singleton )
			delete _singleton;
		_singleton = NULL;
	}
	SBAPI bool initOpenGL();
	SBAPI bool initGLExtension();	
	SBAPI void setViewer(SrViewer* vw);	
	SBAPI void addShader(const char* entryName,const char* vsName, const char* fsName, bool shaderFile = true);
	SBAPI SbmShaderProgram* getShader(const std::string& entryName);
	SBAPI void buildShaders();
	SBAPI static int getShaderSupport() { return shaderSupport; }
};
