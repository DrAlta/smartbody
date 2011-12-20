#include "SBSteerManager.h"
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBSteerManager::SBSteerManager() : SBService()
{
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
}

SBSteerManager::~SBSteerManager()
{

}

SteerSuiteEngineDriver* SBSteerManager::getEngineDriver()
{
	return &_driver;
}

void SBSteerManager::start()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu._scene->getSteerManager()->getEngineDriver()->isInitialized())
	{
		LOG("STEERSIM ALREADY STARTED");
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
	maxItemsPerGridCell = 30;
	steerOptions->gridDatabaseOptions.numGridCellsX = numGridCellsX;
	steerOptions->gridDatabaseOptions.numGridCellsZ = numGridCellsZ;
	steerOptions->gridDatabaseOptions.maxItemsPerGridCell = maxItemsPerGridCell;

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
	} catch (exception& e) {
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

	// create an agent based on the current characters and positions
	SteerLib::ModuleInterface* pprAIModule = mcu._scene->getSteerManager()->getEngineDriver()->_engine->getModule(ai);
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
		double initialRadius = dynamic_cast<SmartBody::DoubleAttribute*>( mcu._scene->getSteerManager()->getAttribute("initialConditions.radius") )->getValue();
		//initialConditions.radius = float(initialRadius);
		initialConditions.radius = 0.3f;//0.2f;//0.4f;
		initialConditions.speed = 0.0f;
		initialConditions.goals.clear();
		initialConditions.name = character->getName();
		SteerLib::AgentInterface* agent = mcu._scene->getSteerManager()->getEngineDriver()->_engine->createAgent( initialConditions, pprAIModule );
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
	mcu._scene->getSteerManager()->getEngineDriver()->startSimulation();
	mcu._scene->getSteerManager()->getEngineDriver()->setStartTime(0.0f);
	return;
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

}
