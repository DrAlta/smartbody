#include "SBOgreListener.h"
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBScene.h>

#ifdef INTMAX_C 
#undef INTMAX_C
#endif
#ifdef UINTMAX_C
#undef UINTMAX_C
#endif

#include "EmbeddedOgre.h"
#include "OgreFrameListener.h"


using namespace Ogre;
using namespace SmartBody;

OgreListener::OgreListener(EmbeddedOgre* ogre)
{
	ogreInterface = ogre;
}

OgreListener::~OgreListener(void)
{
}

void OgreListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	

}

void OgreListener::OnCharacterDelete( const std::string & name )
{

}

void OgreListener::OnCharacterUpdate( const std::string & name, const std::string & objectClass )
{
	SceneNode * node = (SceneNode *)ogreInterface->getSceneManager()->getRootSceneNode()->getChild(name);
	if (!node) return;

	node->detachAllObjects();
	ogreInterface->getSceneManager()->destroyEntity(name);
	ogreInterface->getSceneManager()->getRootSceneNode()->removeAndDestroyChild(name);

	OgreFrameListener* frameListener = dynamic_cast<OgreFrameListener*>(ogreInterface->getOgreFrameListener());
	if (frameListener)
	{
		// delete from character list
		int eraseId = -1;
		for (unsigned int i = 0; i < frameListener->m_characterList.size(); i++)
		{
			if (frameListener->m_characterList[i] == name)
			{
				eraseId = i;
				break;
			}
		}
		if (eraseId >= 0)
			frameListener->m_characterList.erase(frameListener->m_characterList.begin() + eraseId);

		// delete from initial bone position map
		std::map<std::string, std::map<std::string, Ogre::Vector3> >::iterator iter = frameListener->m_initialBonePositions.find(name);
		if (iter != frameListener->m_initialBonePositions.end())
			frameListener->m_initialBonePositions.erase(iter);
	}
}

void OgreListener::OnCharacterChangeMesh( const std::string& name )
{
	// created a ogre entity only when the character is changed and valid
	std::string logMsg = "Character " + name ;
	LogManager::getSingleton().logMessage(logMsg.c_str());
	SBCharacter* sbChar = SBScene::getScene()->getCharacter(name);
	if (!sbChar) return; // no smartbody character exist ?

	Entity * ent = NULL;
	if (ogreInterface->getSceneManager()->hasEntity(name))
		return;

	try
	{
		//Create character from characterType
		//ent = ogreInterface->getSceneManager()->createEntity(name, name + ".mesh" );
		LOG("create ogre chracter = %s",name.c_str());
		ent = ogreInterface->createOgreCharacter(sbChar);
	}
	catch( Ogre::ItemIdentityException& )
	{
		;
	}
	catch( Ogre::Exception& e )
	{
		LOG("Can not create character %s ...",sbChar->getName().c_str());
		LOG("Exception %s",e.getDescription().c_str());
		if( e.getNumber() == Ogre::Exception::ERR_FILE_NOT_FOUND ) 
		{
			//Default to existing Brad character			
			ent = ogreInterface->getSceneManager()->createEntity(name, "Brad.mesh" );
		}
	}

	if (ent == NULL)
	{
		LOG("Can not create character %s ..., no mesh or skeleton exist",sbChar->getName().c_str());
		return;
	}		
	// Add entity to the scene node	
	SceneNode * mSceneNode = ogreInterface->getSceneManager()->getRootSceneNode()->createChildSceneNode(name);
	mSceneNode->attachObject(ent);
	//ent->setVisible(ogreInterface->getCharacterVisiblility());
	mSceneNode->setVisible(ogreInterface->getCharacterVisiblility());
	//mSceneNode->showBoundingBox(true);
	
	//ent->setVisible(false);
	Ogre::Skeleton* skel = ent->getSkeleton();
	if (!skel) return; // this should not happen at all ?

	//ent->setDisplaySkeleton(true);
	std::map<std::string, Ogre::Vector3> intialBonePositions;
	OgreFrameListener* frameListener = dynamic_cast<OgreFrameListener*>(ogreInterface->getOgreFrameListener());
	if (frameListener)
	{
		// insert into character list
		frameListener->m_characterList.push_back(name);
		// get intial bone position for every character
		for (int i = 0; i < skel->getNumBones(); i++)
		{
			Bone* bone = skel->getBone(i);
			intialBonePositions.insert(std::make_pair(bone->getName(), bone->getPosition()));
			frameListener->m_validJointNames.insert(bone->getName());

		}
		frameListener->m_initialBonePositions.insert(std::make_pair(name, intialBonePositions));
	}
}

void OgreListener::OnCharacterChanged( const std::string& name )
{	
	
}

void OgreListener::OnPawnCreate( const std::string & name )
{

}

void OgreListener::OnPawnDelete( const std::string & name )
{

}

void OgreListener::OnViseme( const std::string & name, const std::string & visemeName, const float weight, const float blendTime )
{

}

void OgreListener::OnChannel( const std::string & name, const std::string & channelName, const float value )
{

}

void OgreListener::OnReset()
{

}