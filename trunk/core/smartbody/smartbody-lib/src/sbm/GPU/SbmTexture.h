#pragma once
#include "external/glew/glew.h"
#include <map>
#include <string>

class SbmTexture;

typedef std::map<std::string,SbmTexture*> StrTextureMap;

class SbmTextureManager
{
public:
	enum { TEXTURE_DIFFUSE = 0, TEXTURE_NORMALMAP };
protected:
	StrTextureMap textureMap;
	StrTextureMap normalTexMap;
private:
	static SbmTextureManager* _singleton;
	SbmTextureManager(void);
	~SbmTextureManager(void);
public:
	static SbmTextureManager& singleton() 
	{
		if (!_singleton)
			_singleton = new SbmTextureManager();
		return *_singleton;			
	}

	static void destroy_singleton() {
		if( _singleton )
			delete _singleton;
		_singleton = NULL;
	}
	SbmTexture* findTexture(int type, const char* textureName);
	void loadTexture(int type, const char* textureName, const char* fileName);
	void updateTexture();
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
	SbmTexture(const char* texName);
	~SbmTexture(void);
	bool hasBuild() { return finishBuild; }
	const std::string& getName() { return textureName; }
	GLuint getID() { return texID; }
	void loadImage(const char* fileName);
	void buildTexture();

	unsigned char* getBuffer() { return buffer; }
	int getWidth() const { return width; }	
	int getHeight() const { return height; }
	int getNumChannels() const { return channels; }	
};
