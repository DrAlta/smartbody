#pragma once
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include <Ogre.h>
#include <RTShaderSystem/OgreRTShaderSystem.h>
//#include <sbm/SBSkeleton.h>
//#include <sbm/SBCharacter.h>
//#include <sbm/sbm_deformable_mesh.h>

class ExampleFrameListener;
class OgreListener;
class DeformableMesh;

namespace SmartBody
{
	class SBSkeleton;
	class SBCharacter;
}

class EmbeddedOgre
{
public:
	EmbeddedOgre(void);
	~EmbeddedOgre(void);
	void createOgreWindow(void* windowHandle, void* parentHandle, unsigned long glContext, int width, int height, std::string windowName);
	void setupResource();
	void createDefaultScene();	
	void update();
	void finishRender();
	void setCharacterVisibility(bool bVisible);
	bool getCharacterVisiblility();
	void updateOgreLights();
	void updateOgreCharacterRenderMode();
	void resetOgreScene();

	Ogre::SceneManager* getSceneManager() { return ogreSceneMgr; }
	Ogre::RenderWindow* getRenderWindow() { return ogreWnd; }
	ExampleFrameListener*  getOgreFrameListener() { return ogreFrameListener; }
	Ogre::Camera*       getCamera() { return ogreCamera; }
	unsigned long getGLContext() { return ogreGLContext; }
	Ogre::Entity* createOgreCharacter(SmartBody::SBCharacter* sbChar); // create a ogre character from a smartbody character
	void addSBSkeleton(SmartBody::SBSkeleton* skel); // convert a SB skeleton to ogre
	void addDeformableMesh(std::string meshName, DeformableMesh* mesh);
	void addTexture(std::string texName);	
	

protected:
	static unsigned long getCurrentGLContext();	
	void setCharacterVisible(bool bVisible, std::string charName);
protected:
	Ogre::Root*           ogreRoot;
	Ogre::RenderWindow*   ogreWnd;
	Ogre::SceneManager*   ogreSceneMgr;
	Ogre::Camera*		  ogreCamera;
	ExampleFrameListener* ogreFrameListener;
	Ogre::RTShader::ShaderGenerator* ogreShaderGenerator;   
	Ogre::RTShader::SubRenderState* ogreSrsHardwareSkinning;
	OgreListener*         ogreListener;
	unsigned long         ogreGLContext;
	bool                  ogreCharacterVisible;
};
