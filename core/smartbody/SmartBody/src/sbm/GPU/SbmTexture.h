/*************************************************************
Copyright (C) 2017 University of Southern California

This file is part of Smartbody.

Smartbody is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Smartbody is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Smartbody.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************/

#pragma once
#include <map>
#include <string>
#include <vector>
#include <sb/SBTypes.h>
#include <sb/SBAsset.h>
#include <sr/sr_gl.h>
#include <sr/sr_color.h>

class SbmTexture;
typedef std::map<std::string,SbmTexture*> StrTextureMap;

#if defined(EMSCRIPTEN)
#define USE_CUBE_MAP 1
#else
#define USE_CUBE_MAP 0
#endif

//support for cube map
#if USE_CUBE_MAP
class SbmCubeMapTexture;
typedef std::map<std::string,SbmCubeMapTexture*> StrCubeTextureMap;
#endif


class SbmTextureManager
{
public:
	enum { TEXTURE_DIFFUSE = 0, TEXTURE_NORMALMAP, TEXTURE_SPECULARMAP, TEXTURE_GLOSSYMAP, TEXTURE_RENDER_TARGET, TEXTURE_HDR_MAP, TEXTURE_CUBEMAP};
protected:
	StrTextureMap textureMap;
	StrTextureMap normalTexMap;
	StrTextureMap specularTexMap;
	StrTextureMap glossyTexMap;
	StrTextureMap renderTargetTexMap;
	StrTextureMap hdrTexMap;
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
	SBAPI SbmTexture* createTexture(int type, const char* textureName);
	SBAPI void deleteTexture(int type, const char* textureName);
	SBAPI void createWhiteTexture(const char* textureName, int width = 1, int height = 1);	
	SBAPI void createBlackTexture(const char* textureName, int width = 1, int height = 1);
	SBAPI void createColorTexture(const char* textureName, SrColor initColor, int width = 1, int height = 1);
	
	SBAPI GLuint createFBO(const char* fboName, bool recreateFBO = true);

	SBAPI void updateEnvMaps();
	void releaseAllTextures();	
protected:
	StrTextureMap& findMap(int type);

//Zengrui: add some additional functions for cube-map texture
#if USE_CUBE_MAP
protected:
	StrCubeTextureMap cubeTextureMap;
public:
	SBAPI SbmCubeMapTexture* findCubeMapTexture(const char* cubeMapName);
	SBAPI void loadCubeMapTextures(const std::string cubeMapName, const std::vector<std::string> &textureNames, const std::vector<std::string> &textureFileNames);
#endif
};


class SbmTexture : public SmartBody::SBAsset// simple place holder for OpenGL texture
{
public:
	enum RotateEnum { ROTATE_NONE = 0, ROTATE_90, ROTATE_180, ROTATE_270 };
protected:
	std::string textureName;
	std::string textureFileName;
	int width, height;
	int channels; // num of channels in the image	
	unsigned char* buffer;
	std::vector<unsigned char> imgBuffer;
	bool finishBuild;
	bool transparentTexture;	
	bool buildMipMap;
	GLuint texID;	
	GLint internal_format;
	GLenum texture_format;		
	GLenum dataType;
	RotateEnum texRotate;
public:
	SBAPI SbmTexture(const char* texName);
	SBAPI ~SbmTexture(void);

	void cleanTexture();

	SBAPI bool hasBuild() { return finishBuild; }
	SBAPI bool isTransparent() { return transparentTexture; }
	bool isBuildMipMap() const { return buildMipMap; }
	void setBuildMipMap(bool val) { buildMipMap = val; }

	SBAPI const std::string& getName() { return textureName; }
	SBAPI const std::string& getFileName() { return textureFileName; }
	SBAPI GLuint getID() { return texID; }
	SBAPI bool loadImage(const char* fileName);	

	SBAPI bool loadHDRImage(const char* fileName);

	SBAPI void buildTexture(bool buildMipMap = false, bool recreateTexture = true);
	SBAPI void rotateTexture(RotateEnum rotate);

	SBAPI unsigned char* getBuffer();
	SBAPI int getBufferSize();
	SBAPI int getWidth() const { return width; }	
	SBAPI int getHeight() const { return height; }
	SBAPI int getNumChannels() const { return channels; }	

	SBAPI void setBuffer(unsigned char* buffer, int size);
	SBAPI void setTextureSize(int w, int h, int numChannels);

	SBAPI void bakeAlphaIntoTexture(SbmTexture* alphaTex, bool useAlphaBlend);
	// Creates a 1x1 white texture
	SBAPI void createWhiteTexture(int w = 1, int h = 1);	
	SBAPI void createEmptyTexture(int w , int h, int numChannels, GLenum type = GL_UNSIGNED_BYTE, SrColor initColor = SrColor::white);		

	
};
#if USE_CUBE_MAP
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
	GLenum dataType;
public:
	SBAPI SbmCubeMapTexture(const std::vector<std::string> &textureNames, const std::vector<std::string> &fileNames);
	SBAPI ~SbmCubeMapTexture(void);
	SBAPI bool hasBuild() { return finishBuild; }
	SBAPI bool isTransparent() { return transparentTexture; }
	SBAPI const std::vector<std::string>& getNames() { return textureNames; }
	SBAPI const std::vector<std::string>& getFileName() { return textureFileNames; }
	SBAPI GLuint getID() { return texID; }
	SBAPI bool loadImage(const char* fileName);	
	SBAPI bool loadHDRImage(const char* fileName);
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