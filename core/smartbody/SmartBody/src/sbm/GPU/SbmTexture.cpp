#if !defined(__FLASHPLAYER__) && !defined(__ANDROID__) && !defined(SB_IPHONE) && !defined(EMSCRIPTEN)
#include "external/glew/glew.h"
#include "sbm/GPU/SbmDeformableMeshGPU.h"
#endif
#include "SbmTexture.h"
#include "SbmShader.h"
//#include "external/SOIL/SOIL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
#include <sb/SBTypes.h>
#include <sb/SBAttribute.h>
#include <sb/SBScene.h>
#include <vhcl_log.h>
//#include "external/imdebug/imdebug.h"

/************************************************************************/
/* Sbm Texture Manager                                                  */
/************************************************************************/
SbmTextureManager* SbmTextureManager::_singleton = NULL;

SbmTextureManager::SbmTextureManager( void )
{
	
}

SbmTextureManager::~SbmTextureManager( void )
{
    releaseAllTextures();
}

void SbmTextureManager::updateEnvMaps()
{	
	std::vector<std::string> hdrTexNames = getTextureNames(SbmTextureManager::TEXTURE_HDR_MAP);
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	SmartBody::StringAttribute* envMapAttr = dynamic_cast<SmartBody::StringAttribute*>(scene->getAttribute("Renderer.envMapName"));
	SmartBody::StringAttribute* envDiffuseMapAttr = dynamic_cast<SmartBody::StringAttribute*>(scene->getAttribute("Renderer.envDiffuseMapName"));

	if (envMapAttr)
		envMapAttr->setValidValues(hdrTexNames);
	if (envDiffuseMapAttr)
		envDiffuseMapAttr->setValidValues(hdrTexNames);
}


void SbmTextureManager::releaseAllTextures()
{
    StrTextureMap::iterator vi;
    for ( vi  = textureMap.begin();
          vi != textureMap.end();
          vi++)
    {
        SbmTexture* tex = vi->second;
        delete tex;		
    }

    for ( vi  = normalTexMap.begin();
          vi != normalTexMap.end();
          vi++)
    {
        SbmTexture* tex = vi->second;
        delete tex;		
    }

    for ( vi  = specularTexMap.begin();
        vi != specularTexMap.end();
        vi++)
    {
        SbmTexture* tex = vi->second;
        delete tex;
    }

	for (vi = glossyTexMap.begin();
		vi != glossyTexMap.end();
		vi++)
	{
		SbmTexture* tex = vi->second;
		delete tex;
	}



    textureMap.clear();
    normalTexMap.clear();
    specularTexMap.clear();	
	glossyTexMap.clear();
	
#if USE_CUBE_MAP
    StrCubeTextureMap::iterator it;
    for(it = cubeTextureMap.begin(); it != cubeTextureMap.end(); ++it){
        SbmCubeMapTexture* tex = it->second;
        delete tex;
    }
    cubeTextureMap.clear();
#endif
}

StrTextureMap& SbmTextureManager::findMap( int type )
{
    if (type == TEXTURE_DIFFUSE)
        return textureMap;
    else if (type == TEXTURE_NORMALMAP)
        return normalTexMap;
    else if (type == TEXTURE_SPECULARMAP)
		return specularTexMap;
	else if (type == TEXTURE_GLOSSYMAP)
		return glossyTexMap;
	else if (type == TEXTURE_HDR_MAP)
		return hdrTexMap;
	
    return textureMap;
}


std::vector<std::string> SbmTextureManager::getTextureNames( int type )
{
    StrTextureMap& texMap = findMap(type);
    
    std::vector<std::string> texNames;
    StrTextureMap::iterator vi;
    for ( vi  = texMap.begin();
        vi != texMap.end();
        vi++)
    {
        texNames.push_back(vi->first);		
    }
    return texNames;
}



void SbmTextureManager::loadTexture(int iType, const char* textureName, const char* fileName )
{
    std::string strTex		= textureName;

    // Retrieves texture map type: DIFFUSE, SPECULAR, GLOSSY, or NORMAL
    StrTextureMap& texMap	= findMap(iType);

    // If the texture does not exist in the texture map, create a new one
    if (texMap.find(strTex) == texMap.end()) 
    {
        SbmTexture* texture = new SbmTexture(textureName);
        if(!texture->loadImage(fileName))
        {
            LOG("ERROR: Can't load image %s. Invalid path? Is it an 8-bit image?", fileName);
        }
        texMap[strTex] = texture;
    }
}

void SbmTextureManager::createWhiteTexture(const char* textureName, int width, int height)
{
	createColorTexture(textureName, SrColor::white, width, height);
#if 0
    StrTextureMap& texMap	= findMap(SbmTextureManager::TEXTURE_DIFFUSE);

    // If the texture does not exist in the texture map, create a new one
    if (texMap.find(std::string(textureName)) == texMap.end()) 
    {
        SbmTexture* texture = new SbmTexture(textureName);
        texture->createWhiteTexture(width, height);
        texMap[std::string(textureName)] = texture;
        texture->buildTexture(false);
    }	
#endif
}


SBAPI void SbmTextureManager::createBlackTexture(const char* textureName, int width /*= 1*/, int height /*= 1*/)
{
	createColorTexture(textureName, SrColor::black, width, height);
}

SBAPI void SbmTextureManager::createColorTexture(const char* textureName, SrColor initColor, int width /*= 1*/, int height /*= 1*/) 
{
	StrTextureMap& texMap = findMap(SbmTextureManager::TEXTURE_DIFFUSE);

	// If the texture does not exist in the texture map, create a new one
	if (texMap.find(std::string(textureName)) == texMap.end())
	{
		SbmTexture* texture = new SbmTexture(textureName);
		texture->createEmptyTexture(width, height, 4, GL_UNSIGNED_BYTE, initColor);
		texMap[std::string(textureName)] = texture;
		texture->buildTexture(false);
	}
}

SBAPI SbmTexture* SbmTextureManager::createTexture(int type, const char* textureName)
{
	std::string strTex = textureName;
	StrTextureMap& texMap = findMap(type);
	if (texMap.find(strTex) != texMap.end())
		return texMap[strTex];
	
	SbmTexture* tex = new SbmTexture(textureName);
	texMap[strTex] = tex;
	return tex;
}

SBAPI GLuint SbmTextureManager::createFBO( const char* fboName )
{
    std::string strFBO = fboName;
    if (FBOMap.find(strFBO) == FBOMap.end())
    {
        GLuint fboID;
        glGenFramebuffers(1, &fboID);
        FBOMap[strFBO] = fboID;
    }
	return FBOMap[strFBO];
}


SBAPI GLuint SbmTextureManager::findFBO( const char* fboName )
{
    std::string strFBO = fboName;
    if (FBOMap.find(strFBO) != FBOMap.end()) 
    {
        return FBOMap[strFBO];
    }
    return 0;
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

    for ( vi  = specularTexMap.begin();
        vi != specularTexMap.end();
        vi++)
    {
        SbmTexture* tex = vi->second;
        if (!tex->hasBuild())
            tex->buildTexture();
    }

	for (vi = hdrTexMap.begin();
		vi != hdrTexMap.end();
		vi++)
	{
		SbmTexture* tex = vi->second;
		if (!tex->hasBuild())
			tex->buildTexture();
	}

#if USE_CUBE_MAP
    StrCubeTextureMap::iterator it;
    for(it = cubeTextureMap.begin(); it != cubeTextureMap.end(); ++it){
        SbmCubeMapTexture *tex = it->second;
        if(!tex->hasBuild())
            tex->buildCubeMapTexture();
    }
#endif
}


SBAPI void SbmTextureManager::reloadTexture()
{
    StrTextureMap::iterator vi;
    for ( vi  = textureMap.begin();
        vi != textureMap.end();
        vi++)
    {
        SbmTexture* tex = vi->second;
        tex->buildTexture();
    }

    for ( vi  = normalTexMap.begin();
        vi != normalTexMap.end();
        vi++)
    {
        SbmTexture* tex = vi->second;
        tex->buildTexture();
    }

    for ( vi  = specularTexMap.begin();
        vi != specularTexMap.end();
        vi++)
    {
        SbmTexture* tex = vi->second;
        tex->buildTexture();
    }

	for (vi = hdrTexMap.begin();
		vi != hdrTexMap.end();
		vi++)
	{
		SbmTexture* tex = vi->second;
		tex->buildTexture();
	}


    // recreate FBO when reloading texture
    std::map<std::string, GLuint>::iterator mi;
    for ( mi  = FBOMap.begin();
          mi != FBOMap.end();
          mi++)
    {
        GLuint fboID;
        glGenFramebuffers(1, &fboID);
        mi->second = fboID;
    }
#if USE_CUBE_MAP
    StrCubeTextureMap::iterator it;
    for(it = cubeTextureMap.begin(); it != cubeTextureMap.end(); ++it){
        SbmCubeMapTexture *tex = it->second;
        tex->buildCubeMapTexture();

    }
#endif
}




SbmTexture* SbmTextureManager::findTexture(int type, const char* textureName )
{
    std::string strTex = textureName;
    //LOG("Tex name: %s\tType: %d", strTex.c_str(), type);
    StrTextureMap& texMap = findMap(type);
    if (texMap.find(strTex) != texMap.end())
        return texMap[strTex];
    return NULL;
}
#if USE_CUBE_MAP
SbmCubeMapTexture* SbmTextureManager::findCubeMapTexture(const char* cubeMapName){
    std::string strTex = cubeMapName;
    if(cubeTextureMap.find(strTex) != cubeTextureMap.end())
        return cubeTextureMap[strTex];
    return NULL;
}

void SbmTextureManager::loadCubeMapTextures(const std::string cubeMapName, const std::vector<std::string> &textureNames, const std::vector<std::string> &textureFileNames){
    if(textureNames.size() != 6 || textureFileNames.size() != 6){
        LOG("Textures provided are not enough to build cube map!\n");
        return;
    }
    
    // If the texture does not exist in the texture map, create a new one
    if (cubeTextureMap.find(cubeMapName) == cubeTextureMap.end()) 
    {
        SbmCubeMapTexture* texture = new SbmCubeMapTexture(textureNames, textureFileNames);
        for(int i = 0; i < textureNames.size(); ++i){
            if(!texture->loadImage(textureFileNames[i].c_str()))
            {
                LOG("ERROR: Can't load image %s. Invalid path? Is it an 8-bit image?", textureFileNames[i].c_str());
            }
        }
        
        cubeTextureMap[cubeMapName] = texture;
    }

}
#endif
/************************************************************************/
/* Sbm Texture                                                          */
/************************************************************************/

SbmTexture::SbmTexture( const char* texName )
{
    textureName			= texName;
    texID				= 0;
    buffer				= NULL;
    finishBuild			= false;
    transparentTexture	= false;
    width				= 1;
    height				= 1;
    channels			= 4;
	dataType = GL_UNSIGNED_BYTE;
	internal_format = GL_RGBA8;
	texture_format = GL_RGBA;
}

SbmTexture::~SbmTexture(void)
{
    if (buffer)
           delete [] buffer;
    if (texID != 0)
       glDeleteTextures(1,&texID);	
}

bool SbmTexture::loadHDRImage(const char* fileName)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float *data = stbi_loadf(fileName, &width, &height, &nrComponents, 0);
	if (!data) return false;

	dataType = GL_FLOAT;
	int dataSize = width*height*nrComponents * 4;
	imgBuffer.resize(dataSize);
	unsigned char* ucdata = (unsigned char*)data;
	for (unsigned int i = 0; i < dataSize; i++)
	{
		imgBuffer[i] = ucdata[i];
	}
	stbi_image_free(data);
	return true;
}

bool SbmTexture::loadImage( const char* fileName )
{
#if 0
	if (stbi_is_hdr(fileName)) // is an HDR image, load into float point buffer
	{
		return loadHDRImage(fileName);		
	}
#endif

	stbi_hdr_to_ldr_gamma(2.2f);
	stbi_hdr_to_ldr_scale(1.0f);
	int transparentPixel = 0;
	stbi_set_flip_vertically_on_load(true);
	buffer = stbi_load(fileName, &width, &height, &channels, 0);
	if (!buffer)
	{
		LOG("Image %s failed to load.", fileName);
		return false;
	}
#if 0
    buffer = SOIL_load_image(fileName, &width, &height, &channels, SOIL_LOAD_AUTO);	
	if (width < 0 || height < 0 || channels < 0)
	{
		const char* errorString = SOIL_last_result();
		LOG("Image %s failed to load: [%s]", fileName, errorString);
		return false;
	}
    else
	{
        LOG("Loading image       :%s\t%d\t%d\t%d", fileName, width, height, channels );
    }
    //std::string testOutFileName = fileName;
    //testOutFileName += ".bmp";
    //SOIL_save_image(testOutFileName.c_str(),SOIL_SAVE_TYPE_BMP,width,height,channels,buffer);
    int transparentPixel = 0;
    // invert the image in y-axis
    for(int j = 0; j*2 < height; ++j )
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;
        for(int i = width * channels ; i > 0; --i )
        {
            unsigned char temp = buffer[index1];
            buffer[index1] = buffer[index2];
            buffer[index2] = temp;
            ++index1;
            ++index2;			
        }
    }
#endif

    if (channels == 4)
    {
        for (int j=0;j<height;j++)
        {
            for (int i=0;i<width;i++)
            {
                unsigned char alphaVal = buffer[j*width*channels+i*channels+3];
                if (alphaVal < 255)
                    transparentPixel++;
            }
        }
    }
    

    if (transparentPixel*50 > height*width)
    {
        transparentTexture = true;
        LOG("Texture %s is transparent.",fileName);
    }
    else
    {
        LOG("Texture %s is opaque",fileName);
    }

    imgBuffer.resize(width*height*channels);

    for (int i=0;i<width*height*channels;i++)
    {
        imgBuffer[i] = buffer[i];
    }

    // set the texture file name
    textureFileName = fileName;	
	stbi_image_free(buffer);
    //SOIL_free_image_data(buffer);
    buffer = NULL;
    return true;
}

void SbmTexture::buildTexture(bool buildMipMap, bool recreateTexture)
{	
    //LOG("Start Build Texture");
    if (!getBuffer()) return;
#if !defined(__native_client__)
    //SbmShaderProgram::printOglError("SbmTexture.cpp:10");		
    GLuint iType = GL_TEXTURE_2D;
#if !defined(EMSCRIPTEN)    //OpenGL ES 2.0 no EMUN for GL_TEXTURE_2D
    myGLEnable(GL_TEXTURE_2D);
#endif
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if !defined(EMSCRIPTEN)
    myGLEnable(iType);	
#endif

	if (texID == -1 || recreateTexture)
		glGenTextures(1,&texID);

    glBindTexture(iType,texID);

#if defined(__ANDROID__) || defined(SB_IPHONE)
#define GL_CLAMP GL_CLAMP_TO_EDGE
#define GL_RGB8 GL_RGB
#define GL_RGBA8 GL_RGBA
#endif
    
    //glTexParameteri(iType,GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(iType,GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(iType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(iType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //LOG("After Texture parameters : GL_TEXTURE_WRAP_S");

#if !defined (__FLASHPLAYER__) && !defined(__ANDROID__) && !defined(SB_IPHONE)
    if (buildMipMap)
        glTexParameteri(iType, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    else
        glTexParameteri(iType, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
#if !defined(EMSCRIPTEN)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif
#else
    glTexParameteri(iType, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
#endif

	glTexParameteri(iType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(iType, GL_TEXTURE_MAG_FILTER,GL_LINEAR); 

	bool isFloatTexture = (dataType == GL_FLOAT);
	//if (isFloatTexture)
	//	LOG("Texture %s, is float texture.", getName().c_str());
    if (channels == 3)
    {
#if !defined(EMSCRIPTEN)
        internal_format = isFloatTexture ? GL_RGB32F : GL_RGB8;
        texture_format = GL_RGB;
#else 
        internal_format = GL_RGB;
        texture_format = GL_RGB;
#endif
    }
    else if (channels == 4)
    {
#if !defined(EMSCRIPTEN)
        internal_format = isFloatTexture ? GL_RGBA32F : GL_RGBA8;
        texture_format = GL_RGBA;
#else 
        internal_format = GL_RGBA;
        texture_format = GL_RGBA;
#endif			
    }
	else if (channels == 1) // single channel
	{
		internal_format = isFloatTexture ? GL_DEPTH_COMPONENT32F : GL_R8;
		texture_format = GL_DEPTH_COMPONENT;
	}
	unsigned char* buffer = 0;
	if (imgBuffer.size() > 0)
		buffer = &imgBuffer[0];
    //glTexImage2D(iType,0,texture_format,width,height,0,texture_format,GL_UNSIGNED_BYTE,buffer);	
#if !defined (__FLASHPLAYER__) && !defined(__ANDROID__) && !defined(SB_IPHONE) && !defined(__linux__) &!defined(EMSCRIPTEN)
    //if (buildMipMap)
    //    gluBuild2DMipmaps(iType, channels, width, height, texture_format, dataType, buffer);
    //else
        glTexImage2D(iType,0,internal_format, width,height,0,texture_format, dataType, buffer);
		glGenerateMipmap(GL_TEXTURE_2D);
#elif defined(EMSCRIPTEN)
    glTexImage2D(iType,0,texture_format,width,height,0,texture_format,GL_UNSIGNED_BYTE, buffer);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);
#else
    glTexImage2D(iType,0,texture_format,width,height,0,texture_format,GL_UNSIGNED_BYTE, buffer);
    //glGenerateMipmap(GL_TEXTURE_2D);
#endif

    //LOG("texture id = %u, texture name = %s, width = %d, height = %d, channel = %d",texID, textureName.c_str(), width, height, channels);

    //glGenerateMipmap(iType);
    //SbmShaderProgram::printOglError("Sb!defined(SB_IPHONE)mTexture.cpp:200");
#if !defined(__ANDROID__) && !defined(SB_IPHONE) && !defined(EMSCRIPTEN)
    GLclampf iPrority = 1.0;
    glPrioritizeTextures(1,&texID,&iPrority);
#endif
    //TextureDebug();	
    glBindTexture(iType,0);	
    finishBuild = true;
    //LOG("Finish build texture");
    //SbmShaderProgram::printOglError("SbmTexture.cpp:300");
    //LOG("Texture name = %s, texture ID = %d",textureName.c_str(),texID);	
    //imdebug("rgb w=%d h=%d %p", width, height, buffer);
#endif
}

unsigned char* SbmTexture::getBuffer()
{
    if (imgBuffer.size() == 0) return NULL;
    
    return &imgBuffer[0];
}

int SbmTexture::getBufferSize()
{
    return imgBuffer.size();
}

void SbmTexture::setBuffer(unsigned char* buffer, int size)
{
    imgBuffer.clear();
    for (int i = 0; i < size; ++i)
    {
        imgBuffer.push_back(buffer[i]);
    }
}

void SbmTexture::setTextureSize(int w, int h, int numChannels)
{
    width		= w;
    height		= h;
    channels	= numChannels;
}

void SbmTexture::createWhiteTexture(int w, int h)
{
#if 0
    unsigned char* data;

    width		= w;
    height		= h;
    channels	= 4;

    data = new unsigned char[width * height * channels * sizeof(unsigned char)];

    for(int i = 0; i < (int)(width * height * channels * sizeof(unsigned char)); i++)
    {
        data[i] = 255;
    }

    imgBuffer.resize(width*height*channels);

    for (int i=0;i<width*height*channels;i++)
    {
        imgBuffer[i] = data[i];
    }
#endif
	createEmptyTexture(w, h, 4, GL_UNSIGNED_BYTE, SrColor::white);
    //textureFileName		= "white";	
    //textureName			= "white";
}


SBAPI void SbmTexture::createEmptyTexture(int w /*= 1*/, int h /*= 1*/, int numChannels /*= 1*/, GLenum type /*= GL_UNSIGNED_BYTE*/, SrColor initColor)
{
	unsigned char* data;

	width = w;
	height = h;
	channels = numChannels;
	dataType = type;
	int bytesPerChannel = 1; // assume unsigned byte
	if (dataType == GL_FLOAT)
		bytesPerChannel = 4; // set to float for g-buffer application


	data = new unsigned char[width * height * channels * sizeof(unsigned char)*bytesPerChannel];

	if (dataType == GL_FLOAT)
	{
		float color[4];
		initColor.get(color);
		float* floatData = (float*)data;
		for (unsigned int i = 0; i < width*height; i++)
		{
			for (int j = 0; j < channels; j++)
			{
				floatData[i*channels + j] = color[j];
			}			
		}
	}
	else // (dataType == GL_UNSIGNED_BYTE)
	{
		unsigned char color[4];
		initColor.get(color);
		for (unsigned int i = 0; i < width*height; i++)
		{
			for (int j = 0; j < channels; j++)
			{
				data[i*channels + j] = color[j];
			}
		}
	}

	imgBuffer.resize(width*height*channels*bytesPerChannel);

	for (int i = 0; i < width*height*channels; i++)
	{
		imgBuffer[i] = data[i];
	}
	delete [] data;	
}



SBAPI void SbmTexture::bakeAlphaIntoTexture(SbmTexture* alphaTex)
{
	unsigned char* alphaBuf = alphaTex->getBuffer();	
	if (alphaTex->getWidth() != getWidth() || alphaTex->getHeight() != getHeight())
	{
		LOG("Warning! Alpha texture size (%d, %d) do not match diffuse texture '%s' size (%d, %d)", alphaTex->getWidth(), alphaTex->getHeight(), this->getName().c_str(), getWidth(), getHeight());
		return;
	}

	if (getNumChannels() != 4) // set back to 4 channel texture
	{
		std::vector<unsigned char> tempBuffer;
		tempBuffer.resize(getWidth()*getHeight() * 4);
		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < width; i++)
			{
				for (int k=0;k<getNumChannels();k++)
					tempBuffer[j*width*4 + i*4 + k] = imgBuffer[j*width*channels + i*channels + k];				
			}
		}
		imgBuffer = tempBuffer;
		channels = 4;
	}
	
	int numAlphaChannel = alphaTex->getNumChannels();
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			
			imgBuffer[j*width * 4 + i * 4 + 3] = alphaBuf[j*width*numAlphaChannel + i*numAlphaChannel + 0];
		}
	}
	transparentTexture = true;
}

#if USE_CUBE_MAP
SbmCubeMapTexture::SbmCubeMapTexture(const std::vector<std::string>& texNames, const std::vector<std::string> &fileNames){
    if(texNames.size() != 6 || fileNames.size())
        LOG("Wrong number of textures!");
    textureNames = texNames;
    textureFileNames = fileNames;
    texID				= 0;
    buffer				= NULL;
    finishBuild			= false;
    transparentTexture	= false;
    width				= -1;
    height				= -1;
    channels			= -1;

}
SbmCubeMapTexture::~SbmCubeMapTexture(){
    if (buffer)
        delete [] buffer;
    if (texID != 0)
        glDeleteTextures(1,&texID);	
}



bool SbmCubeMapTexture::loadImage(const char* fileName){
    
	buffer = SOIL_load_image(fileName, &width, &height, &channels, SOIL_LOAD_AUTO);	
    if (width < 0 || height < 0 || channels < 0)
        return false;
    else {
        LOG("Loading image       :%s\t%d\t%d\t%d", fileName, width, height, channels );
    }
    
    int transparentPixel = 0;
    // invert the image in y-axis
    for(int j = 0; j*2 < height; ++j )
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;
        for(int i = width * channels ; i > 0; --i )
        {
            unsigned char temp = buffer[index1];
            buffer[index1] = buffer[index2];
            buffer[index2] = temp;
            ++index1;
            ++index2;			
        }
    }

    if (channels == 4)
    {
        for (int j=0;j<height;j++)
        {
            for (int i=0;i<width;i++)
            {
                unsigned char alphaVal = buffer[j*width*channels+i*channels+3];
                if (alphaVal < 250)
                    transparentPixel++;
            }
        }
    }


    if (transparentPixel*20 > height*width)
        transparentTexture = true;

    //append the new image to the image buffer
    size_t offset = imgBuffer.size();
    imgBuffer.resize(offset + width * height * channels);

    for (size_t i=0;i<width*height*channels;i++)
    {
        imgBuffer[i + offset] = buffer[i];
    }

    //TODO: set the texture file name

    SOIL_free_image_data(buffer);
    buffer = NULL;
    return true;
}
void SbmCubeMapTexture::buildCubeMapTexture(bool buildMipMap){
    if (!getBuffer()) return;	
    GLuint iType = GL_TEXTURE_CUBE_MAP;
    glGenTextures ( 1, &texID );
    // Bind the texture object
    glBindTexture ( iType, texID );
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //we bind them into one texId
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    if (channels == 3)
    {
        internal_format = GL_RGB;
        texture_format = GL_RGB;		
    }
    else if (channels == 4)
    {
        internal_format = GL_RGBA;
        texture_format = GL_RGBA;				
    }
    //loading texture to the cube

    for(int i = 0, offset = 0; i < 6; ++i, offset += width * height * channels){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, texture_format, width, height, 0, texture_format, GL_UNSIGNED_BYTE, &(imgBuffer[offset]));
    }
    //glBindTexture(iType, 0);	
    finishBuild = true;
}
unsigned char* SbmCubeMapTexture::getBuffer()
{
    if (imgBuffer.size() == 0) return NULL;

    return &imgBuffer[0];
}

int SbmCubeMapTexture::getBufferSize()
{
    return imgBuffer.size();
}

void SbmCubeMapTexture::setBuffer(unsigned char* buffer, int size)
{
    imgBuffer.clear();
    for (int i = 0; i < size; ++i)
    {
        imgBuffer.push_back(buffer[i]);
    }
}

void SbmCubeMapTexture::setTextureSize(int w, int h, int numChannels)
{
    width		= w;
    height		= h;
    channels	= numChannels;
}

#endif