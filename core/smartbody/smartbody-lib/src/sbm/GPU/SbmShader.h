#pragma once
#include "external/glew/glew.h"
#include <vector>
#include <string>
#include <map>

class SbmShaderProgram
{
protected:
	GLuint     vsID, fsID, programID;
	std::string     vsFilename, fsFilename;
	bool       isBuilt;
public:
	SbmShaderProgram();
	~SbmShaderProgram();		
	void initShaderProgram(const char* vsName, const char* fsName);		
	GLuint getShaderProgram() { return programID; }

	void buildShader();
	bool finishBuild() { return isBuilt; }
public:
	static char *textFileRead(const char *fn); // text file reading 	
	static void printShaderInfoLog(GLuint obj);
	static void printProgramInfoLog(GLuint obj);
protected:
	void loadShader(GLuint sID, const char* shaderFileName);		
};


class SbmShaderManager
{
public:
	static bool shaderInit;		
protected:
	std::map<std::string,SbmShaderProgram*> shaderMap;
private:
	// for singleton
	static SbmShaderManager* _singleton;
	SbmShaderManager(void);
	~SbmShaderManager(void);
public:
	static SbmShaderManager& singleton() 
	{
		if (!_singleton)
			_singleton = new SbmShaderManager();
		return *_singleton;			
	}

	static void destroy_singleton() {
		if( _singleton )
			delete _singleton;
		_singleton = NULL;
	}

	static bool initGLExtension();	
	void addShader(const char* entryName,const char* vsName, const char* fsName);
	SbmShaderProgram* getShader(const char* entryName);
	void buildShaders();
};
