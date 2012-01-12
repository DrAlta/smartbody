#include "SbmTexture.h"
#include "SbmShader.h"
#include "external/SOIL/SOIL.h"

/************************************************************************/
/* Sbm Texture Manager                                                  */
/************************************************************************/
SbmTextureManager* SbmTextureManager::_singleton = NULL;

SbmTextureManager::SbmTextureManager( void )
{

}

SbmTextureManager::~SbmTextureManager( void )
{

}

StrTextureMap& SbmTextureManager::findMap( int type )
{
	if (type == TEXTURE_DIFFUSE)
		return textureMap;
	else if (type == TEXTURE_NORMALMAP)
		return normalTexMap;

	return textureMap;
}


void SbmTextureManager::loadTexture(int iType, const char* textureName, const char* fileName )
{
	std::string strTex = textureName;
	StrTextureMap& texMap = findMap(iType);
	if (texMap.find(strTex) == texMap.end()) // the texture does not exist in the texture map, create a new one
	{
		SbmTexture* texture = new SbmTexture(textureName);
		texture->loadImage(fileName);
		texMap[strTex] = texture;
	}	
}

void SbmTextureManager::updateTexture()
{
	StrTextureMap::iterator vi;
	for ( vi  = textureMap.begin();
		  vi != textureMap.end();
		  vi++)
	{
		SbmTexture* tex = vi->second;
		if (!tex->hasBuild())
			tex->buildTexture();
	}

	for ( vi  = normalTexMap.begin();
		  vi != normalTexMap.end();
		  vi++)
	{
		SbmTexture* tex = vi->second;
		if (!tex->hasBuild())
			tex->buildTexture();
		}
}

SbmTexture* SbmTextureManager::findTexture(int type, const char* textureName )
{
	std::string strTex = textureName;
	StrTextureMap& texMap = findMap(type);
	if (texMap.find(strTex) != texMap.end())
		return texMap[strTex];
	return NULL;
}
/************************************************************************/
/* Sbm Texture                                                          */
/************************************************************************/

SbmTexture::SbmTexture( const char* texName )
{
	textureName = texName;
	texID = 0;
	buffer = NULL;
	finishBuild = false;
}

SbmTexture::~SbmTexture(void)
{
}

void SbmTexture::loadImage( const char* fileName )
{
	buffer = SOIL_load_image(fileName,&width,&height,&channels,SOIL_LOAD_AUTO);			
}

void SbmTexture::buildTexture()
{	
	GLuint iType = GL_TEXTURE_2D;
	glEnable(GL_TEXTURE);
	glEnable(iType);	
	glGenTextures(1,&texID);
	glBindTexture(iType,texID);

	if (!glIsTexture(texID))
	{
		SbmShaderProgram::printOglError("SbmTexture.cpp:100");
	}

	//SbmShaderProgram::printOglError("SbmTexture.cpp:50");	
	glTexParameteri(iType,GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(iType,GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(iType, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(iType, GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
	//SbmShaderProgram::printOglError("SbmTexture.cpp:100");	
	if (channels == 3)
	{
		internal_format = GL_RGB8;
		texture_format = GL_RGB;		
	}
	else if (channels == 4)
	{
		internal_format = GL_RGBA8;
		texture_format = GL_RGBA;				
	}
	//glTexImage2D(iType,0,internal_format,width,height,0,texture_format,GL_UNSIGNED_BYTE,buffer);	
	gluBuild2DMipmaps(iType, channels, width, height, texture_format, GL_UNSIGNED_BYTE, buffer );
	//glGenerateMipmap(iType);
	//SbmShaderProgram::printOglError("SbmTexture.cpp:200");

	GLclampf iPrority = 1.0;
	glPrioritizeTextures(1,&texID,&iPrority);
	//TextureDebug();	
	glBindTexture(iType,0);	
	finishBuild = true;
	//printf("Texture name = %s, texture ID = %d\n",textureName.c_str(),texID);	
}

