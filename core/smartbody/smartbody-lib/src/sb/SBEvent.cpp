//#include "vhcl_log.h"
#include "SBEvent.h"
#include "sbm/mcontrol_util.h"

#include <boost/algorithm/string/replace.hpp>

namespace SmartBody {

SBBasicHandler::SBBasicHandler() : SBEventHandler() 
{
}

void SBBasicHandler::setAction(const std::string& action)
{ 
	m_action = action;
}

const std::string& SBBasicHandler::getAction()
{ 
	return m_action;
}

void SBBasicHandler::executeAction(SBEvent* event) 
{
	std::string action = getAction();
	boost::replace_all(action, "$1", event->getParameters());

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*) action.c_str());
}

SBEventManager::SBEventManager()
{
}

SBEventManager::~SBEventManager()
{
	for (std::map<std::string, SBEventHandler*>::iterator iter = eventHandlers.begin();
		 iter != eventHandlers.end();
		 iter++)
	{
		SBEventHandler* handler = (*iter).second;
		delete handler;
	}
}

void SBEventManager::handleEvent(SBEvent* e, double time)
{
	// find the appropriate event handler
	std::map<std::string, SBEventHandler*>::iterator iter = eventHandlers.find(e->getType());
	if (iter == eventHandlers.end())
		return;

	SBEventHandler* handler = (*iter).second;
	handler->executeAction(e);	
}

SBEvent* SBEventManager::createEvent(const std::string& type, const std::string parameters)
{
	SBEvent* event = new SBEvent();
	event->setType(type);
	event->setParameters(parameters);

	return event;
}

void SBEventManager::addEventHandler(const std::string& type, SBEventHandler* handler)
{
	removeEventHandler(type);
	eventHandlers.insert(std::pair<std::string, SBEventHandler*>(type, handler));
}

void SBEventManager::removeEventHandler(const std::string& type)
{
	std::map<std::string, SBEventHandler*>::iterator iter = eventHandlers.find(type);
	if (iter != eventHandlers.end())
	{
		SBEventHandler* oldHandler = (*iter).second;
		eventHandlers.erase(iter);
		//delete oldHandler; // deleting old handler causes crash when handler is created with Python - need to fix this
	}
}

int SBEventManager::getNumEventHandlers()
{
	return eventHandlers.size();
}

SBEventHandler* SBEventManager::getEventHandlerByIndex(int num)
{
	int counter = 0;
	for (std::map<std::string, SBEventHandler*>::iterator iter = eventHandlers.begin();
		 iter != eventHandlers.end();
		 iter++)
	{
		if (counter == num)
		{
			SBEventHandler* handler = (*iter).second;
			return handler;
		}
	}

	return NULL;
}

SBEventHandler* SBEventManager::getEventHandler(const std::string& type)
{
	std::map<std::string, SBEventHandler*>::iterator iter = eventHandlers.find(type);
	if (iter != eventHandlers.end())
	{
		SBEventHandler* handler = (*iter).second;
		return handler;
	}

	return NULL;
}

}
