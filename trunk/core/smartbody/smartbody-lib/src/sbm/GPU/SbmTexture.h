#pragma once
#if __native_client__
#include <GLES2/gl2.h>
#elif __FLASHPLAYER__
#include <GL/gl.h>
#include <GL/glu.h>
#else
//#include "external/glew/glew.h"
#include <windows.h>
# include <GL/gl.h>
# include <GL/glu.h>
#endif
#include <map>
#include <string>
#include <sb/SBTypes.h>

class SbmTexture;

typedef std::map<std::string,SbmTexture*> StrTextureMap;

class SbmTextureManager
{
public:
	enum { TEXTURE_DIFFUSE = 0, TEXTURE_NORMALMAP, TEXTURE_SPECULARMAP };
protected:
	StrTextureMap textureMap;
	StrTextureMap normalTexMap;
	StrTextureMap specularTexMap;
private:
	static SbmTextureManager* _singleton;
	SbmTextureManager(void);
	~SbmTextureManager(void);
public:
	SBAPI static SbmTextureManager& singleton() 
	{
		if (!_singleton)
			_singleton = new SbmTextureManager();
		return *_singleton;			
	}

	SBAPI static void destroy_singleton() {
		if( _singleton )
			delete _singleton;
		_singleton = NULL;
	}
	SBAPI SbmTexture* findTexture(int type, const char* textureName);
	SBAPI void loadTexture(int type, const char* textureName, const char* fileName);
	SBAPI void updateTexture();
protected:
	StrTextureMap& findMap(int type);
};

class SbmTexture // simple place holder for OpenGL texture
{
protected:
	std::string textureName;
	int width, height;
	int channels; // num of channels in the iamge	
	unsigned char* buffer;
	bool finishBuild;

	GLuint texID;	
	GLuint internal_format, texture_format;		
public:
	SBAPI SbmTexture(const char* texName);
	SBAPI ~SbmTexture(void);
	SBAPI bool hasBuild() { return finishBuild; }
	SBAPI const std::string& getName() { return textureName; }
	SBAPI GLuint getID() { return texID; }
	SBAPI void loadImage(const char* fileName);
	SBAPI void buildTexture();

	SBAPI unsigned char* getBuffer() { return buffer; }
	SBAPI int getWidth() const { return width; }	
	SBAPI int getHeight() const { return height; }
	SBAPI int getNumChannels() const { return channels; }	
};
