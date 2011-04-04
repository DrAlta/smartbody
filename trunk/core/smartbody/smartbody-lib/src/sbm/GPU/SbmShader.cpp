#include "SbmShader.h"
//#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************************************************************/
/* Shader program class                                                 */
/************************************************************************/

SbmShaderProgram::SbmShaderProgram()
{
	vsID = -1; 
	fsID = -1;
	programID = -1;
	isBuilt = false;
}


SbmShaderProgram::~SbmShaderProgram()
{
	glDeleteObjectARB(programID);
	glDeleteObjectARB(vsID);
	glDeleteObjectARB(fsID);
	isBuilt = false;
}


void SbmShaderProgram::initShaderProgram( const char* vsName, const char* fsName )
{
	// we can initialize the shader name first, before there is a opengl context
	if (vsName)
		vsFilename = vsName;
	if (fsName)
		fsFilename = fsName;	
}


void SbmShaderProgram::buildShader()
{
	// build the shader after there is an opengl context
	if (vsFilename.size() > 0)
	{
		vsID = glCreateShader(GL_VERTEX_SHADER);
		loadShader(vsID,vsFilename.c_str());
	}
	if (fsFilename.size() > 0)
	{
		fsID = glCreateShader(GL_FRAGMENT_SHADER);
		loadShader(fsID,fsFilename.c_str());
	}

	printShaderInfoLog(vsID);
	programID = glCreateProgram();
	glAttachShader(programID,vsID);
	glAttachShader(programID,fsID);
	glLinkProgram(programID);
	printProgramInfoLog(programID);
	isBuilt = true;
}


void SbmShaderProgram::loadShader(GLuint sID,  const char* shaderFileName )
{
	char *vs = NULL;	
	vs = textFileRead(shaderFileName);	
	if (!vs) return;
	const char * vv = vs;
	glShaderSource(sID, 1, &vv,NULL);
	free(vs);
	glCompileShader(sID);	
}

char * SbmShaderProgram::textFileRead(const char *fn )
{
	FILE *fp;
	char *content = NULL;
	int count=0;
	if (fn != NULL) {
		fp = fopen(fn,"rt");
		if (fp != NULL) {
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);
			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

void SbmShaderProgram::printShaderInfoLog( GLuint obj )
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}

void SbmShaderProgram::printProgramInfoLog( GLuint obj )
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}
/************************************************************************/
/* Shader Manager                                                       */
/************************************************************************/

bool SbmShaderManager::shaderInit = false;
SbmShaderManager* SbmShaderManager::_singleton = NULL;

SbmShaderManager::SbmShaderManager(void)
{
}

SbmShaderManager::~SbmShaderManager(void)
{
	std::map<std::string,SbmShaderProgram*>::iterator vi;
	for ( vi  = shaderMap.begin();
		  vi != shaderMap.end();
		  vi++)
	{
		SbmShaderProgram* program = vi->second;
		delete program;
	}
}

bool SbmShaderManager::initGLExtension()
{	
	if (shaderInit) // already initialize glew
		return true;

	glewInit();
	if (glewIsSupported("GL_VERSION_2_0"))
	{
		printf("Ready for OpenGL 2.0\n");
		shaderInit = true;
		return true;
	}
	else {
		printf("OpenGL 2.0 not supported\n");
		//exit(1);
		return false;
	}	
}

void SbmShaderManager::addShader( const char* entryName,const char* vsName, const char* fsName )
{
	std::string keyName = entryName;
	if (shaderMap.find(keyName) != shaderMap.end())
	{
		SbmShaderProgram* tempS = shaderMap[keyName];
		delete tempS;
	}
	
    SbmShaderProgram* program = new SbmShaderProgram();
	program->initShaderProgram(vsName,fsName);
	shaderMap[keyName] = program;
}

void SbmShaderManager::buildShaders()
{
	std::map<std::string,SbmShaderProgram*>::iterator vi;
	for ( vi  = shaderMap.begin();
		  vi != shaderMap.end();
		  vi++)
	{
		SbmShaderProgram* program = vi->second;
		if (!program->finishBuild())
			program->buildShader();
	}
}

SbmShaderProgram* SbmShaderManager::getShader( const char* entryName )
{
	SbmShaderProgram* program = NULL;
	std::string keyName = entryName;
	if (shaderMap.find(keyName) != shaderMap.end())
	{
		program = shaderMap[keyName];		
	}
	return program;	
}