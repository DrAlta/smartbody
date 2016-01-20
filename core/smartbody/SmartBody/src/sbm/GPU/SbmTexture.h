#pragma once
#include <map>
#include <string>
#include <vector>
#include <sb/SBTypes.h>
#include <sr/sr_gl.h>

class SbmTexture;
typedef std::map<std::string,SbmTexture*> StrTextureMap;

//support for cube map
#if defined(EMSCRIPTEN)
class SbmCubeMapTexture;
typedef std::map<std::string,SbmCubeMapTexture*> StrCubeTextureMap;
#endif


class SbmTextureManager
{
public:
	enum { TEXTURE_DIFFUSE = 0, TEXTURE_NORMALMAP, TEXTURE_SPECULARMAP, TEXTURE_CUBEMAP};
protected:
	StrTextureMap textureMap;
	StrTextureMap normalTexMap;
	StrTextureMap specularTexMap;
	std::map<std::string, GLuint> FBOMap;
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
	SBAPI GLuint findFBO(const char* fboName);
	SBAPI void loadTexture(int type, const char* textureName, const char* fileName);
	SBAPI void updateTexture();
	SBAPI void reloadTexture();
	SBAPI std::vector<std::string> getTextureNames(int type);

	// Creates a 1x1 white texture
	SBAPI void createWhiteTexture(const char* textureName, int width = 1, int height = 1);
	SBAPI void createFBO(const char* fboName);

	void releaseAllTextures();	
protected:
	StrTextureMap& findMap(int type);

//Zengrui: add some additional functions for cube-map texture
#if defined(EMSCRIPTEN)
protected:
	StrCubeTextureMap cubeTextureMap;
public:
	SBAPI SbmCubeMapTexture* findCubeMapTexture(const char* cubeMapName);
	SBAPI void loadCubeMapTextures(const std::string cubeMapName, const std::vector<std::string> &textureNames, const std::vector<std::string> &textureFileNames);
#endif
};

class SbmTexture // simple place holder for OpenGL texture
{
protected:
	std::string textureName;
	std::string textureFileName;
	int width, height;
	int channels; // num of channels in the image	
	unsigned char* buffer;
	std::vector<unsigned char> imgBuffer;
	bool finishBuild;
	bool transparentTexture;
	GLuint texID;	
	GLuint internal_format, texture_format;		
public:
	SBAPI SbmTexture(const char* texName);
	SBAPI ~SbmTexture(void);
	SBAPI bool hasBuild() { return finishBuild; }
	SBAPI bool isTransparent() { return transparentTexture; }
	SBAPI const std::string& getName() { return textureName; }
	SBAPI const std::string& getFileName() { return textureFileName; }
	SBAPI GLuint getID() { return texID; }
	SBAPI bool loadImage(const char* fileName);	
	SBAPI void buildTexture(bool buildMipMap = true);

	SBAPI unsigned char* getBuffer();
	SBAPI int getBufferSize();
	SBAPI int getWidth() const { return width; }	
	SBAPI int getHeight() const { return height; }
	SBAPI int getNumChannels() const { return channels; }	

	SBAPI void setBuffer(unsigned char* buffer, int size);
	SBAPI void setTextureSize(int w, int h, int numChannels);

	// Creates a 1x1 white texture
	SBAPI void createWhiteTexture(int w = 1, int h = 1);
};
#if defined(EMSCRIPTEN)
class SbmCubeMapTexture // simple place holder for OpenGL ES cubemap texture
{
protected:
	std::vector<std::string> textureNames;
	std::vector<std::string> textureFileNames;
	int width, height;
	int channels; // num of channels in the image	
	unsigned char* buffer;
	std::vector<unsigned char> imgBuffer;
	bool finishBuild;
	bool transparentTexture;
	GLuint texID;	
	GLuint internal_format, texture_format;		
public:
	SBAPI SbmCubeMapTexture(const std::vector<std::string> &textureNames, const std::vector<std::string> &fileNames);
	SBAPI ~SbmCubeMapTexture(void);
	SBAPI bool hasBuild() { return finishBuild; }
	SBAPI bool isTransparent() { return transparentTexture; }
	SBAPI const std::vector<std::string>& getNames() { return textureNames; }
	SBAPI const std::vector<std::string>& getFileName() { return textureFileNames; }
	SBAPI GLuint getID() { return texID; }
	SBAPI bool loadImage(const char* fileName);	
	SBAPI void buildCubeMapTexture(bool buildMipMap = false);
	SBAPI unsigned char* getBuffer();
	SBAPI int getBufferSize();
	SBAPI int getWidth() const { return width; }	
	SBAPI int getHeight() const { return height; }
	SBAPI int getNumChannels() const { return channels; }	

	SBAPI void setBuffer(unsigned char* buffer, int size);
	SBAPI void setTextureSize(int w, int h, int numChannels);

	// Creates a 1x1 white texture
	SBAPI void createWhiteTexture(){};
};
#endif