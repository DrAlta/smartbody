#include "SBCollisionManager.h"
#include <sbm/SBScene.h>
#include <sbm/SBCharacter.h>
#include <sbm/SBSteerManager.h>
#include <sbm/SBSimulationManager.h>
#include <sbm/Event.h>
#include <sbm/Physics/SbmPhysicsSimODE.h>

namespace SmartBody {

SBCollisionManager::SBCollisionManager()
{
	setName("collision");

	createIntAttribute("maxNumIterations", 5, true, "Basic", 60, false, false, false, "Max number collision response iterations.");

	_maxIterations = 5;
	collisionSpace = NULL;
}

SBCollisionManager::~SBCollisionManager()
{
}

void SBCollisionManager::setEnable(bool enable)
{
	SBService::setEnable(enable);
	if (enable)
		start();
	else
		stop();
	SmartBody::SBAttribute* attribute = getAttribute("enable");
	if (attribute)
	{
		SmartBody::BoolAttribute* enableAttribute = dynamic_cast<SmartBody::BoolAttribute*>(attribute);
		enableAttribute->setValueFast(enable);
	}
}

void SBCollisionManager::start()
{
	SBScene* scene = SmartBody::SBScene::getScene();
	SBSteerManager* steerManager = scene->getSteerManager();
	_characterRadius = (float) steerManager->getDoubleAttribute("initialConditions.radius");
	double sceneScale = scene->getDoubleAttribute("scale");
	if (sceneScale != 0.0)
	{
		_characterRadius *= (float) (1.0 / sceneScale);
	}
	_positions.clear();
	_velocities.clear();
	if (!collisionSpace)
	{
		collisionSpace = new SbmCollisionSpaceODE();
	}
	std::vector<std::string> characterNames = scene->getCharacterNames();
	for (std::vector<std::string>::iterator iter = characterNames.begin();
		 iter != characterNames.end();
		 iter++)
	{
		_positions.insert(std::pair<std::string, SrVec>((*iter), SrVec()));
		_velocities.insert(std::pair<std::string, SrVec>((*iter), SrVec()));
		SBCharacter* character = scene->getCharacter(*iter);
		if (!character->getGeomObject() || character->getGeomObject()->geomType() == "null") // no collision geometry setup for the character
		{
			//SbmGeomObject* obj = new SbmGeomCapsule()			
			SrBox bbox = character->getBoundingBox();	
			float yoffset = bbox.getMinimum().y - character->get_world_offset().get_translation().y;
			SrVec size = SrVec(0,_characterRadius,0);
			SbmGeomObject* obj = createCollisionObject(character->getGeomObjectName(),"capsule",size,SrVec(0,yoffset,0),SrVec(0,yoffset+character->getHeight(),0));
			obj->attachToObj(character);
			addObjectToCollisionSpace(character->getGeomObjectName());
			//new SbmGeomCapsule(SrVec(0,yoffset,0),SrVec(0,yoffset+character->getHeight(),0),_characterRadius);
			//character->setGeomObject(obj);
			//collisionSpace->addCollisionObjects(obj);
		}
	}
}

void SBCollisionManager::beforeUpdate(double time)
{
}

void SBCollisionManager::update(double time)
{
}

void SBCollisionManager::afterUpdate(double time)
{	
	// determine if any of the characters are currently in collision
	// horribly inefficient n^2 implementation for characters only
	SBScene* scene = SmartBody::SBScene::getScene();
	double timeDt = scene->getSimulationManager()->getTimeDt();
	if (timeDt == 0.0)
		timeDt = .016;
	
	std::vector<std::string> characters = scene->getCharacterNames();
	for (std::vector<std::string>::iterator iter = characters.begin();
		 iter != characters.end();
		 iter++)
	{
		SBCharacter* character =  scene->getCharacter((*iter));
		SrVec position = character->getPosition();
		position[1] = 0.0;
		SrVec& oldPosition = _positions[(*iter)];
		_positions[(*iter)] = position;
		_velocities[(*iter)] = (position - oldPosition) / (float) timeDt;

	}

	int curIteration = 0;
	bool needMoreIterations = true;	

	EventManager* eventManager = EventManager::getEventManager();	

	while (needMoreIterations && curIteration < _maxIterations)
	{
		needMoreIterations = false;
		SbmCollisionPairList potentialCollisions;		
		collisionSpace->getPotentialCollisionPairs(potentialCollisions);

		for (unsigned int i=0;i<potentialCollisions.size();i++)
		{
			//LOG("Collision Pair = %s %s",potentialCollisions[i].first.c_str(), potentialCollisions[i].second.c_str());
			SbmGeomObject* g1 = getCollisionObject(potentialCollisions[i].first);
			SbmGeomObject* g2 = getCollisionObject(potentialCollisions[i].second);

			std::vector<SbmGeomContact> contactPts;
			SbmCollisionUtil::collisionDetection(g1,g2,contactPts);
			if (contactPts.size() > 0)	
			{
				// collision handling here
				SBCharacter* c1 = dynamic_cast<SBCharacter*>(g1->getAttachObj());
				SBCharacter* c2 = dynamic_cast<SBCharacter*>(g2->getAttachObj());
				if (c1 && c2)
				{
					Event* collisionEvent = eventManager->createEvent("collision",c1->getName()+"/"+c2->getName());
					eventManager->handleEvent(collisionEvent, time);
					//LOG("Collision detected between character %s and character %s",c1->getName().c_str(), c2->getName().c_str());
					delete collisionEvent; // free the memory
				}
				needMoreIterations = true;
			}
		}
		curIteration++;
	}

		

#if 0
		for (std::vector<std::string>::iterator iter = characters.begin();
			 iter != characters.end();
			 iter++)
		{
			SBCharacter* character1 =  scene->getCharacter((*iter));
			SrVec position1 = character1->getPosition();
			position1[1] = 0.0;
			
			for (std::vector<std::string>::iterator iter2 = iter + 1;
			 iter2 != characters.end();
			 iter2++)
			{
				SBCharacter* character2 =  scene->getCharacter((*iter2));
				// determine if the two characters are in collision
				std::vector<SbmGeomContact> contactPts;
				SbmCollisionUtil::collisionDetection(character1->getGeomObject(),character2->getGeomObject(),contactPts);
				
				if (contactPts.size() > 0)
				{
					//LOG("Collision between character %s and character %s",character1->getName().c_str(), character2->getName().c_str());

				}

				
				SrVec position2 = character2->getPosition();
				position2[1] = 0.0;
				float distance = dist(position1, position2);
				if (distance < _characterRadius * 2)
				{
					float penetration = _characterRadius * 2 - distance;
					needMoreIterations = true;
					curIteration++;
					// collision resolution
					// who is moving faster?
					SrVec& velocity1 = _velocities[(*iter)];
					SrVec& velocity2 = _velocities[(*iter2)];
					float magnitude1 = velocity1.len();
					float magnitude2 = velocity2.len();
					// move the object with greater velocity
					if (magnitude1 > magnitude2)
					{
						SrVec diff = position1 - position2;
						diff.normalize();
						// move character1 in that direction
						position1 += diff * penetration;
						_positions[(*iter)] = position1;
						SrVec curPosition = character1->getPosition();
						curPosition.x = position1[0];
						curPosition.z = position1[2];
						character1->setPosition(curPosition);
					}
					else
					{
						SrVec diff = position2 - position1;
						diff.normalize();
						// move character1 in that direction
						position2 += diff * penetration;
						_positions[(*iter2)] = position2;
						SrVec curPosition = character2->getPosition();
						curPosition.x = position2[0];
						curPosition.z = position2[2];
						character2->setPosition(curPosition);
					}
				}				
				
			}			
		}
#endif
	

}

void SBCollisionManager::stop()
{
}

void SBCollisionManager::notify(SBSubject* subject)
{
	SBService::notify(subject);

// 	SmartBody::SBAttribute* attribute = dynamic_cast<SmartBody::SBAttribute*>(subject);
// 	if (!attribute)
// 	{
// 		return;
// 	}
// 
// 	const std::string& name = attribute->getName();
// 	if (name == "maxNumIterations")
// 	{
// 		_maxIterations = getIntAttribute("maxNumIterations");
// 		return;
// 	}
}

SbmGeomObject* SBCollisionManager::createCollisionObject( const std::string& geomName, const std::string& geomType, SrVec size, SrVec from, SrVec to )
{	
	SbmGeomObject* newObj = SbmGeomObject::createGeometry(geomType,size,from,to);
	if (newObj)
	{
		removeCollisionObject(geomName); // remove existing one
		geomObjectMap[geomName] = newObj;
	}	
	return newObj;
}

SbmGeomObject* SBCollisionManager::getCollisionObject( const std::string& geomName )
{
	SbmGeomObject* obj = NULL;
	if (geomObjectMap.find(geomName) != geomObjectMap.end())
	{
		obj = geomObjectMap[geomName];
	}
	return obj;
}

bool SBCollisionManager::removeCollisionObject( const std::string& geomName )
{
	SbmGeomObject* geomObj = getCollisionObject(geomName);
	if (geomObj)
	{		
		removeObjectFromCollisionSpace(geomName);
		geomObjectMap.erase(geomName);
		delete geomObj;
		return true;
	}
	return false;
}

bool SBCollisionManager::addObjectToCollisionSpace( const std::string& geomName )
{
	SbmGeomObject* geomObj = getCollisionObject(geomName);
	if (geomObj)
	{
		collisionSpace->addCollisionObjects(geomName);
		return true;
	}
	return false;

}

bool SBCollisionManager::removeObjectFromCollisionSpace( const std::string& geomName )
{
	SbmGeomObject* geomObj = getCollisionObject(geomName);
	if (geomObj)
	{
		collisionSpace->removeCollisionObjects(geomName);
		return true;
	}
	return false;
}
}


