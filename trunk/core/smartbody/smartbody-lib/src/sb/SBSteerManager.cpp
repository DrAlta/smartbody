#include "SBSteerManager.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <PPRAgent.h>
#include <sb/SBSteerAgent.h>
#include <sbm/PPRAISteeringAgent.h>
#include <sbm/SteerSuiteEngineDriver.h>
#include <SteerLib.h>

namespace SmartBody {

SBSteerManager::SBSteerManager() : SBService()
{
	setName("steering");
	_driver = new SteerSuiteEngineDriver();
#ifdef WIN32
			createStringAttribute("aimodule", "pprAI", true, "Basic", 60, false, false, false, "Agent module library");
#endif
#ifdef __linux__
			createStringAttribute("aimodule", "libpprAI", true, "Basic", 60, false, false, false, "Agent module library");
#endif
#ifdef __APPLE__
			createStringAttribute("aimodule", "libpprAI", true, "Basic", 60, false, false, false, "Agent module library");
#endif

	createStringAttribute("engineOptions.testCaseSearchPath", "../../../../core/smartbody/steersuite-1.3/testcases/", true, "Basic", 60, false, false, false, "Path to find agent shared libraries");
	createStringAttribute("engineOptions.moduleSearchPath", "../../../../core/smartbody/sbm/bin/", true, "Basic", 60, false, false, false, "Path to find test cases");
	createDoubleAttribute("gridDatabaseOptions.gridSizeX", 35, true, "Basic", 60, false, false, false, "Size of grid in x dimension.");
	createDoubleAttribute("gridDatabaseOptions.gridSizeZ", 35, true, "Basic", 60, false, false, false, "Size of grid in z dimension.");
	createIntAttribute("gridDatabaseOptions.numGridCellsX", 70, true, "Basic", 60, false, false, false, "Number of grid cells in x dimension.");
	createIntAttribute("gridDatabaseOptions.numGridCellsZ", 70, true, "Basic", 60, false, false, false, "Number of grid cells in z dimension.");
	createIntAttribute("gridDatabaseOptions.maxItemsPerGridCell", 7, true, "Basic", 60, false, false, false, "Max units per grid cell. If agent density is high, make sure increase this value.");
	createDoubleAttribute("initialConditions.radius", 0.4, true, "Basic", 60, false, false, false, "Initial radius of agents in meters.");
	createBoolAttribute("addBoundaryWalls", true, true, "Basic", 60, false, false, false, "Adds boundaries around the perimeter of the grid to prevent agents from leaving grid area.");
	createBoolAttribute("useEnvironmentCollisions", true, true, "Basic", 60, false, false, false, "Determines whether to include the environment (pawns) when determining steering path. If set to false, objects in the environment will be ignored.");
	createDoubleAttribute("maxUpdateFrequency", 60.0, true, "Basic", 60, false, false, false, "Maximum frequency of steering updates.");	
}

SBSteerManager::~SBSteerManager()
{
	std::map<std::string, SBSteerAgent*>::iterator iter = _steerAgents.begin();
	for (; iter != _steerAgents.end(); iter++)
	{
		delete iter->second;
	}
	_steerAgents.clear();

	delete _driver;

	// TODO: boundaryObstacles
}

SteerSuiteEngineDriver* SBSteerManager::getEngineDriver()
{
	return _driver;
}

void SBSteerManager::setEnable(bool enable)
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

void SBSteerManager::beforeUpdate(double time)
{
	
}

void SBSteerManager::update(double time)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SBScene* scene = mcu._scene;
	if (getEngineDriver()->isInitialized())
	{
		if (!getEngineDriver()->isDone())
		{

			if (getEngineDriver()->getStartTime() == 0.0)
			{
				getEngineDriver()->setStartTime(mcu.time);
				getEngineDriver()->setLastUpdateTime(mcu.time - _maxUpdateFrequency - .01);
			}

			double timeDiff = mcu.time - getEngineDriver()->getLastUpdateTime();
			if (timeDiff >= _maxUpdateFrequency)
			{ // limit steering to 60 fps
				mcu.mark("SteeringUpdate",0,"Update");
				getEngineDriver()->setLastUpdateTime(mcu.time);
				for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
					iter != mcu.getCharacterMap().end();
					iter++)
				{
					SbmCharacter* character = (*iter).second;
					SmartBody::SBSteerAgent* steerAgent = getSteerAgent(character->getName());
					if (steerAgent)
						steerAgent->evaluate(timeDiff);
				}

				bool running = getEngineDriver()->_engine->update(false, true, (float) (mcu.time - getEngineDriver()->getStartTime()));
				if (!running)
					getEngineDriver()->setDone(true);
				mcu.mark("SteeringUpdate");
			}

		}
	}
}

void SBSteerManager::afterUpdate(double time)
{
}

void SBSteerManager::start()
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (scene->getSteerManager()->getEngineDriver()->isInitialized())
	{
		LOG("STEERSIM ALREADY STARTED");
		return;
	}

	SteerLib::SimulationOptions* steerOptions = new SteerLib::SimulationOptions();
	steerOptions->moduleOptionsDatabase["testCasePlayer"]["testcase"] = "3-way-confusion-1.xml";
	std::string ai = dynamic_cast<SmartBody::StringAttribute*>( mcu._scene->getSteerManager()->getAttribute("aimodule") )->getValue();

	if (ai == "")
		return;
	steerOptions->moduleOptionsDatabase["testCasePlayer"]["ai"] = ai;
	steerOptions->engineOptions.startupModules.insert("testCasePlayer");
	std::string testCases = dynamic_cast<SmartBody::StringAttribute*>( mcu._scene->getSteerManager()->getAttribute("engineOptions.testCaseSearchPath") )->getValue();
	steerOptions->engineOptions.testCaseSearchPath = testCases;
	std::string moduleSearchPath = dynamic_cast<SmartBody::StringAttribute*>( mcu._scene->getSteerManager()->getAttribute("engineOptions.moduleSearchPath") )->getValue();
	steerOptions->engineOptions.moduleSearchPath = moduleSearchPath;
	double gridSizeX = dynamic_cast<SmartBody::DoubleAttribute*>( mcu._scene->getSteerManager()->getAttribute("gridDatabaseOptions.gridSizeX") )->getValue();
	double gridSizeZ = dynamic_cast<SmartBody::DoubleAttribute*>( mcu._scene->getSteerManager()->getAttribute("gridDatabaseOptions.gridSizeZ") )->getValue();
	steerOptions->gridDatabaseOptions.gridSizeX = float(gridSizeX);
    steerOptions->gridDatabaseOptions.gridSizeZ = float(gridSizeZ);
	int numGridCellsX = dynamic_cast<SmartBody::IntAttribute*> (mcu._scene->getSteerManager()->getAttribute("gridDatabaseOptions.numGridCellsX"))->getValue();
	int numGridCellsZ = dynamic_cast<SmartBody::IntAttribute*> (mcu._scene->getSteerManager()->getAttribute("gridDatabaseOptions.numGridCellsZ"))->getValue();
	int maxItemsPerGridCell = dynamic_cast<SmartBody::IntAttribute*> (mcu._scene->getSteerManager()->getAttribute("gridDatabaseOptions.maxItemsPerGridCell"))->getValue();
	LOG("max Items per grid cell = %d",maxItemsPerGridCell);
	steerOptions->gridDatabaseOptions.numGridCellsX = numGridCellsX;
	steerOptions->gridDatabaseOptions.numGridCellsZ = numGridCellsZ;
	steerOptions->gridDatabaseOptions.maxItemsPerGridCell = maxItemsPerGridCell;

	bool setBoundaries = mcu._scene->getSteerManager()->getBoolAttribute("addBoundaryWalls");
	if (setBoundaries)
	{
		for (std::vector<SteerLib::BoxObstacle*>::iterator iter = _boundaryObstacles.begin();
			 iter != _boundaryObstacles.end();
			 iter++)
		{
			getEngineDriver()->_engine->removeObstacle((*iter));
			getEngineDriver()->_engine->getSpatialDatabase()->removeObject((*iter), (*iter)->getBounds());
			delete (*iter);
		}
		SteerLib::BoxObstacle* top = new SteerLib::BoxObstacle((float) -gridSizeX / 2.0f, (float) gridSizeX / 2.0f, 0.0f,  1.0f, (float) -gridSizeZ / 2.0f, (float) -gridSizeZ / 2.0f + 1.0f);
		_boundaryObstacles.push_back(top);
		SteerLib::BoxObstacle* bottom = new SteerLib::BoxObstacle((float) -gridSizeX / 2.0f, (float) gridSizeX / 2.0f, 0.0f,  1.0f, (float) gridSizeZ / 2.0f - 1.0f, (float) gridSizeZ / 2.0f);
		_boundaryObstacles.push_back(bottom);
		SteerLib::BoxObstacle* left = new SteerLib::BoxObstacle((float) -gridSizeX / 2.0f, (float) -gridSizeX / 2.0f + 1.0f, 0.0f,  1.0f, (float) -gridSizeZ / 2.0f, (float) gridSizeZ / 2.0f);
		_boundaryObstacles.push_back(left);
		SteerLib::BoxObstacle* right = new SteerLib::BoxObstacle((float) gridSizeX / 2.0f - 1.0f, (float) gridSizeX / 2.0f, 0.0f,  1.0f, (float) -gridSizeZ / 2.0f, (float) gridSizeZ / 2.0f);
		_boundaryObstacles.push_back(right);
	}

	//	customize the item per grid cell
	//	steerOptions->gridDatabaseOptions.maxItemsPerGridCell = maxItemPerCell;


	LOG("INIT STEERSIM");
	try {
		mcu._scene->getSteerManager()->getEngineDriver()->init(steerOptions);
	} catch (Util::GenericException& ge) {
		LOG("Problem starting steering engine: %s", ge.what()); 
		mcu._scene->getSteerManager()->getEngineDriver()->finish();
		delete steerOptions;
		return;
	} catch (std::exception& e) {
		if (e.what())
			LOG("Problem starting steering engine: %s", e.what()); 
		else
			LOG("Unknown problem starting steering engine: %s", e.what()); 

		mcu._scene->getSteerManager()->getEngineDriver()->finish();
		delete steerOptions;
		return;
	}

	LOG("LOADING STEERSIM");
	mcu._scene->getSteerManager()->getEngineDriver()->loadSimulation();

	int numSetup = 0;
	// create an agent based on the current characters and positions
	SteerLib::ModuleInterface* pprAIModule = mcu._scene->getSteerManager()->getEngineDriver()->_engine->getModule(ai);
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		SmartBody::SBSteerManager* steerManager = SmartBody::SBScene::getScene()->getSteerManager();
		SmartBody::SBSteerAgent* steerAgent = steerManager->getSteerAgent(character->getName());
		if (!steerAgent)
		{
			LOG("No steering agent for character %s", character->getName().c_str());
			continue;
		}

		float x, y, z;
		float yaw, pitch, roll;
		character->get_world_offset(x, y, z, yaw, pitch, roll);
		SteerLib::AgentInitialConditions initialConditions;
		initialConditions.position = Util::Point( x * scene->getScale(), 0.0f, z * scene->getScale() );
		Util::Vector orientation = Util::rotateInXZPlane(Util::Vector(0.0f, 0.0f, 1.0f), yaw * float(M_PI) / 180.0f);
		initialConditions.direction = orientation;
		double initialRadius = dynamic_cast<SmartBody::DoubleAttribute*>( mcu._scene->getSteerManager()->getAttribute("initialConditions.radius") )->getValue();
		if (initialRadius == 0.0)
			initialConditions.radius = 0.3f;//0.2f;//0.4f;
		else
			initialConditions.radius = (float) initialRadius;
		initialConditions.speed = 0.0f;
		initialConditions.goals.clear();
		initialConditions.name = character->getName();		
		SteerLib::AgentInterface* agent = mcu._scene->getSteerManager()->getEngineDriver()->_engine->createAgent( initialConditions, pprAIModule );			
		PPRAISteeringAgent* ppraiAgent = dynamic_cast<PPRAISteeringAgent*>(steerAgent);
		ppraiAgent->setAgent(agent);
		agent->reset(initialConditions, dynamic_cast<SteerLib::EngineInterface*>(pprAIModule));
		LOG("Setting up steering agent for character %s", character->getName().c_str());
		numSetup++;
	}
	if (numSetup == 0)
	{
		LOG("No characters set up with steering. Steering will need to be restarted when new characters are available.");
	}

	bool useEnvironment = getBoolAttribute("useEnvironmentCollisions");

	if (useEnvironment)
	{
		// adding obstacles to the steering space
		for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
			iter != mcu.getPawnMap().end();
			iter++)
		{
			SBPawn* pawn = dynamic_cast<SBPawn*>(iter->second);
			SBCharacter* character = dynamic_cast<SBCharacter*>(iter->second);
			if (character) continue; // do not set obstacle for the character, it will mess up the steering
// 			if ((*iter).second->getGeomObject())
// 				(*iter).second->initSteeringSpaceObject();
			if (pawn && pawn->getGeomObject()->geomType() != "null")
				pawn->initSteeringSpaceObject();
		}

	}

	// add any boundary walls, if applicable
	for (std::vector<SteerLib::BoxObstacle*>::iterator iter = _boundaryObstacles.begin();
			 iter != _boundaryObstacles.end();
			 iter++)
	{
		getEngineDriver()->_engine->addObstacle((*iter));
		getEngineDriver()->_engine->getSpatialDatabase()->addObject((*iter), (*iter)->getBounds());
	}

	LOG("STARTING STEERSIM");
	mcu._scene->getSteerManager()->getEngineDriver()->startSimulation();
	mcu._scene->getSteerManager()->getEngineDriver()->setStartTime(0.0f);

	_maxUpdateFrequency = getDoubleAttribute("maxUpdateFrequency");
	if (_maxUpdateFrequency != 0.0)
		_maxUpdateFrequency = 1.0 / _maxUpdateFrequency;
	else
		_maxUpdateFrequency = .016;
}

void SBSteerManager::stop()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu._scene->getSteerManager()->getEngineDriver()->isInitialized())
	{
		mcu._scene->getSteerManager()->getEngineDriver()->stopSimulation();
		mcu._scene->getSteerManager()->getEngineDriver()->unloadSimulation();
		mcu._scene->getSteerManager()->getEngineDriver()->finish();

		for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
			iter != mcu.getCharacterMap().end();
			iter++)
		{
			SmartBody::SBSteerAgent* steerAgent = getSteerAgent((*iter).second->getName());
		
			if (steerAgent)
			{
				PPRAISteeringAgent* ppraiAgent = dynamic_cast<PPRAISteeringAgent*>(steerAgent);
				ppraiAgent->setAgent(NULL);
			}
				
		}

		for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
			iter != mcu.getPawnMap().end();
			iter++)
		{
			if ((*iter).second->steeringSpaceObj_p)
			{
				delete (*iter).second->steeringSpaceObj_p;
				(*iter).second->steeringSpaceObj_p = NULL;
			}
		}

		for (std::vector<SteerLib::BoxObstacle*>::iterator iter = _boundaryObstacles.begin();
			 iter != _boundaryObstacles.end();
			 iter++)
		{
			delete (*iter);
		}
		_boundaryObstacles.clear();
	}
}

SBSteerAgent* SBSteerManager::createSteerAgent(std::string name)
{
	
	std::map<std::string, SBSteerAgent*>::iterator iter = _steerAgents.find(name);
	if (iter != _steerAgents.end())
	{
		LOG("Steer agent with name %s already exists.", name.c_str());
		return iter->second;
	}
	SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(name);
	if (!character)
	{
		LOG("Character named '%s' does not exist, steering agent cannot be constructed.", name.c_str());
		return NULL;
	}
	SBSteerAgent* agent = new PPRAISteeringAgent(character);
	_steerAgents.insert(std::pair<std::string, SBSteerAgent*>(name, agent));
	return agent;
}

void SBSteerManager::removeSteerAgent(std::string name)
{
	std::map<std::string, SBSteerAgent*>::iterator iter = _steerAgents.find(name);
	if (iter != _steerAgents.end())
	{
		_steerAgents.erase(iter);
		return;
	}
	LOG("Steer agent with name %s does not exist.", name.c_str());
}

int SBSteerManager::getNumSteerAgents()
{
	return _steerAgents.size();
}

SBSteerAgent* SBSteerManager::getSteerAgent(std::string name)
{
	return _steerAgents[name];
}

std::vector<std::string> SBSteerManager::getSteerAgentNames()
{
	std::vector<std::string> steerAgentNames;

	for (std::map<std::string, SBSteerAgent*>::iterator iter = _steerAgents.begin();
		 iter != _steerAgents.end();
		 iter++)
	{
		steerAgentNames.push_back((*iter).first);
	}

	return steerAgentNames;
}

std::map<std::string, SBSteerAgent*>& SBSteerManager::getSteerAgents()
{
	return _steerAgents;
}

void SBSteerManager::onCharacterDelete(SBCharacter* character)
{
	removeSteerAgent(character->getName());
}

}
