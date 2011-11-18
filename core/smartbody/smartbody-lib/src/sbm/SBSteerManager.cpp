#include "SBSteerManager.h"
#include <sbm/mcontrol_util.h>

SBSteerManager::SBSteerManager()
{
}

SBSteerManager::~SBSteerManager()
{
}

void SBSteerManager::start()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.steerEngine.isInitialized())
	{
		LOG("STEERSIM ALREADY STARTED");
	}

	SteerLib::SimulationOptions* steerOptions = new SteerLib::SimulationOptions();
	steerOptions->moduleOptionsDatabase["testCasePlayer"]["testcase"] = "3-way-confusion-1.xml";
	std::string ai = dynamic_cast<SmartBody::StringAttribute*>( mcu.steerEngine.getAttribute("aimodule") )->getValue();

	if (ai == "")
		return;
	steerOptions->moduleOptionsDatabase["testCasePlayer"]["ai"] = ai;
	steerOptions->engineOptions.startupModules.insert("testCasePlayer");
	std::string testCases = dynamic_cast<SmartBody::StringAttribute*>( mcu.steerEngine.getAttribute("engineOptions.testCaseSearchPath") )->getValue();
	steerOptions->engineOptions.testCaseSearchPath = testCases;
	std::string moduleSearchPath = dynamic_cast<SmartBody::StringAttribute*>( mcu.steerEngine.getAttribute("engineOptions.moduleSearchPath") )->getValue();
	steerOptions->engineOptions.moduleSearchPath = moduleSearchPath;
	double gridSizeX = dynamic_cast<SmartBody::DoubleAttribute*>( mcu.steerEngine.getAttribute("gridDatabaseOptions.gridSizeX") )->getValue();
	double gridSizeZ = dynamic_cast<SmartBody::DoubleAttribute*>( mcu.steerEngine.getAttribute("gridDatabaseOptions.gridSizeZ") )->getValue();
	steerOptions->gridDatabaseOptions.gridSizeX = float(gridSizeX);
    steerOptions->gridDatabaseOptions.gridSizeZ = float(gridSizeZ);
	int numGridCellsX = dynamic_cast<SmartBody::IntAttribute*> (mcu.steerEngine.getAttribute("gridDatabaseOptions.numGridCellsX"))->getValue();
	int numGridCellsZ = dynamic_cast<SmartBody::IntAttribute*> (mcu.steerEngine.getAttribute("gridDatabaseOptions.numGridCellsZ"))->getValue();
	int maxItemsPerGridCell = dynamic_cast<SmartBody::IntAttribute*> (mcu.steerEngine.getAttribute("gridDatabaseOptions.maxItemsPerGridCell"))->getValue();
	steerOptions->gridDatabaseOptions.numGridCellsX = numGridCellsX;
	steerOptions->gridDatabaseOptions.numGridCellsZ = numGridCellsZ;
	steerOptions->gridDatabaseOptions.maxItemsPerGridCell = maxItemsPerGridCell;

	//	customize the item per grid cell
	//	steerOptions->gridDatabaseOptions.maxItemsPerGridCell = maxItemPerCell;


	LOG("INIT STEERSIM");
	try {
		mcu.steerEngine.init(steerOptions);
	} catch (exception& e) {
		if (e.what())
			LOG("Problem starting steering engine: %s", e.what()); 
		else
			LOG("Unknown problem starting steering engine: %s", e.what()); 

		mcu.steerEngine.finish();
		delete steerOptions;
		return;
	}

	LOG("LOADING STEERSIM");
	mcu.steerEngine.loadSimulation();

	// create an agent based on the current characters and positions
	SteerLib::ModuleInterface* pprAIModule = mcu.steerEngine._engine->getModule(ai);
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;

		if (!character->steeringAgent)
			continue;

		float x, y, z;
		float yaw, pitch, roll;
		character->get_world_offset(x, y, z, yaw, pitch, roll);
		SteerLib::AgentInitialConditions initialConditions;
		initialConditions.position = Util::Point( x * mcu.steeringScale, 0.0f, z * mcu.steeringScale );
		Util::Vector orientation = Util::rotateInXZPlane(Util::Vector(0.0f, 0.0f, 1.0f), yaw * float(M_PI) / 180.0f);
		initialConditions.direction = orientation;
		double initialRadius = dynamic_cast<SmartBody::DoubleAttribute*>( mcu.steerEngine.getAttribute("initialConditions.radius") )->getValue();
		//initialConditions.radius = float(initialRadius);
		initialConditions.radius = 0.3f;//0.2f;//0.4f;
		initialConditions.speed = 0.0f;
		initialConditions.goals.clear();
		initialConditions.name = character->getName();
		SteerLib::AgentInterface* agent = mcu.steerEngine._engine->createAgent( initialConditions, pprAIModule );
		character->steeringAgent->setAgent(agent);
		agent->reset(initialConditions, dynamic_cast<SteerLib::EngineInterface*>(pprAIModule));
	}
	// adding obstacles to the steering space
	for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		if ((*iter).second->colObj_p)
			(*iter).second->initSteeringSpaceObject();
	}

	LOG("STARTING STEERSIM");
	mcu.steerEngine.startSimulation();
	mcu.steerEngine.setStartTime(0.0f);
	return;
}

void SBSteerManager::stop()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.steerEngine.isInitialized())
	{
		mcu.steerEngine.stopSimulation();
		mcu.steerEngine.unloadSimulation();
		mcu.steerEngine.finish();

		for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
			iter != mcu.getCharacterMap().end();
			iter++)
		{
			(*iter).second->steeringAgent->setAgent(NULL);
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
	}
}

void SBSteerManager::setSteerUnit(std::string unit)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (unit == "meter")
		mcu.steeringScale = 1.0f;
	else if (unit == "centimeter")
		mcu.steeringScale = 0.01f;
	else
		LOG("Unit %s not supported yet", unit.c_str());
}

std::string SBSteerManager::getSteerUnit()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.steeringScale == 1.0)
		return "meter";
	if (mcu.steeringScale == 0.01)
		return "centimeter";

	return "";
}

SBSteerAgent* SBSteerManager::createSteerAgent(std::string name)
{
	SBSteerAgent* agent = new SBSteerAgent();
	std::map<std::string, SBSteerAgent*>::iterator iter = _steerAgents.find(name);
	if (iter != _steerAgents.end())
	{
		LOG("Steer agent with name %s already exists.", name.c_str());
		return NULL;
	}
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