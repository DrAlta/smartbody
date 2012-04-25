#include "SBCollisionManager.h"
#include <sbm/SBScene.h>
#include <sbm/SBCharacter.h>
#include <sbm/SBSteerManager.h>
#include <sbm/SBSimulationManager.h>

namespace SmartBody {

SBCollisionManager::SBCollisionManager()
{
	setName("collision");

	createIntAttribute("maxNumIterations", 5, true, "Basic", 60, false, false, false, "Max number collision response iterations.");

	_maxIterations = 5;
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
	std::vector<std::string> characterNames = scene->getCharacterNames();
	for (std::vector<std::string>::iterator iter = characterNames.begin();
		 iter != characterNames.end();
		 iter++)
	{
		_positions.insert(std::pair<std::string, SrVec>((*iter), SrVec()));
		_velocities.insert(std::pair<std::string, SrVec>((*iter), SrVec()));
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

	while (needMoreIterations && curIteration < _maxIterations)
	{
		needMoreIterations = false;
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
				SrVec position2 = character2->getPosition();
				position2[2] = 0.0;
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
	}

}

void SBCollisionManager::stop()
{
}

void SBCollisionManager::notify(SBSubject* subject)
{
	SBService::notify(subject);

	SmartBody::SBAttribute* attribute = dynamic_cast<SmartBody::SBAttribute*>(subject);
	if (!attribute)
	{
		return;
	}

	const std::string& name = attribute->getName();
	if (name == "maxNumIterations")
	{
		_maxIterations = getIntAttribute("maxNumIterations");
		return;
	}
}

}


