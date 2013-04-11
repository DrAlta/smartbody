#include "SBListener.h"
#include "OgreFramework.h"
#include <OgreTagPoint.h>

SBListener::SBListener(OgreFramework* app)
{
	m_app = app;
}


void SBListener::OnCharacterCreate( const  std::string & name, const  std::string & objectClass )
{	
	std::string logMsg = "Character " + name + " Created. Type is " + objectClass;
	Ogre::LogManager::getSingleton().logMessage(logMsg.c_str());
	if (objectClass == "pawn")
		return;

	Ogre::Entity * ent;
	if (m_app->getSceneManager()->hasEntity(name))
		return;

	try
	{
		//Create character from characterType
		ent = m_app->getSceneManager()->createEntity(name, name + ".mesh" );
	}
	catch( Ogre::ItemIdentityException& )
	{
		;
	}
	catch( Ogre::Exception& e )
	{
		if( e.getNumber() == Ogre::Exception::ERR_FILE_NOT_FOUND ) 
		{
			//Default to existing Brad character
			ent = m_app->getSceneManager()->createEntity(name, "Brad.mesh" );
		}
	}

	if (ent == NULL)
	{
		return;
	}

	// Add entity to the scene node
	Ogre::SceneNode * mSceneNode = m_app->getSceneManager()->getRootSceneNode()->createChildSceneNode(name);
	//ent->setDisplaySkeleton(true);
	mSceneNode->attachObject(ent);
	Ogre::Skeleton* skel = ent->getSkeleton();
	m_app->m_characterNameList.push_back(name);
	Ogre::SceneNode * skelNode = m_app->getSceneManager()->getRootSceneNode()->createChildSceneNode(name+"_skel");

	for (int i = 0; i < skel->getNumBones(); i++)
	{
		Ogre::Bone* bone = skel->getBone(i);
		Ogre::Entity* e;
		Ogre::String jointName = name+"_joint#"+Ogre::StringConverter::toString(i);
		Ogre::SceneNode * jointNode = skelNode->createChildSceneNode(jointName);
		jointNode->setScale(0.03,0.03,0.03);
		e = m_app->getSceneManager()->createEntity(jointName,"cube.mesh");
		e->setMaterialName("lambert1");
		jointNode->attachObject(e);
		//Ogre::TagPoint* test = ent->attachObjectToBone(bone->getName(), e, Ogre::Quaternion::IDENTITY, Ogre::Vector3::ZERO);
		//test->setScale(0.1,0.1,0.1);
	}
	ent->setVisible(false);

	//ent->setVisible(false);
/*
	OgreFrameListener* frameListener = m_app->getOgreFrameListener();
	if (frameListener)
	{
		// insert into character list
		frameListener->m_characterList.push_back(name);
		
		// get intial bone position for every character
		std::map<std::string, Ogre::Vector3> intialBonePositions;

		for (int i = 0; i < skel->getNumBones(); i++)
		{
			Bone* bone = skel->getBone(i);
			intialBonePositions.insert(std::make_pair(bone->getName(), bone->getPosition()));
			frameListener->m_validJointNames.insert(bone->getName());

		}
		frameListener->m_initialBonePositions.insert(std::make_pair(name, intialBonePositions));
	}
*/
}

void SBListener::OnCharacterDelete( const  std::string & name )
{
	Ogre::SceneNode * node = (Ogre::SceneNode *)m_app->getSceneManager()->getRootSceneNode()->getChild(name);
	node->detachAllObjects();
	m_app->getSceneManager()->destroyEntity(name);
	m_app->getSceneManager()->getRootSceneNode()->removeAndDestroyChild(name);

/*
	OgreFrameListener* frameListener = m_app->getOgreFrameListener();
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
		std::map<std::string, std::map<std::string, Ogre::Vector3>>::iterator iter = frameListener->m_initialBonePositions.find(name);
		if (iter != frameListener->m_initialBonePositions.end())
			frameListener->m_initialBonePositions.erase(iter);
	}
*/
}

void SBListener::OnCharacterChange( const  std::string & name )
{
	LOG( "Character Changed!\n" );
}
