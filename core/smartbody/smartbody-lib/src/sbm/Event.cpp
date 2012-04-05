//#include "vhcl_log.h"
#include "Event.h"
#include "mcontrol_util.h"

#include <boost/algorithm/string/replace.hpp>

namespace SmartBody {

BasicHandler::BasicHandler() : EventHandler() 
{
}

void BasicHandler::setAction(const std::string& action)
{ 
	m_action = action;
}

const std::string& BasicHandler::getAction()
{ 
	return m_action;
}

void BasicHandler::executeAction(Event* event) 
{
	std::string action = getAction();
	boost::replace_all(action, "$1", event->getParameters());

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*) action.c_str());
}

EventManager* EventManager::_eventManager = NULL;

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
	for (std::map<std::string, EventHandler*>::iterator iter = eventHandlers.begin();
		 iter != eventHandlers.end();
		 iter++)
	{
		EventHandler* handler = (*iter).second;
		delete handler;
	}
}

EventManager* EventManager::getEventManager()
{ 
	if (_eventManager == NULL)
	{
		_eventManager = new EventManager();
	}

	return _eventManager;
}


void EventManager::handleEvent(Event* e, double time)
{
	// find the appropriate event handler
	std::map<std::string, EventHandler*>::iterator iter = eventHandlers.find(e->getType());
	if (iter == eventHandlers.end())
		return;

	EventHandler* handler = (*iter).second;
	handler->executeAction(e);	
}

Event* EventManager::createEvent(const std::string& type, const std::string parameters)
{
	Event* event = new Event();
	event->setType(type);
	event->setParameters(parameters);

	return event;
}

void EventManager::addEventHandler(const std::string& type, EventHandler* handler)
{
	removeEventHandler(type);
	eventHandlers.insert(std::pair<std::string, EventHandler*>(type, handler));
}

void EventManager::removeEventHandler(const std::string& type)
{
	std::map<std::string, EventHandler*>::iterator iter = eventHandlers.find(type);
	if (iter != eventHandlers.end())
	{
		EventHandler* oldHandler = (*iter).second;
		eventHandlers.erase(iter);
		//delete oldHandler; // deleting old handler causes crash when handler is created with Python - need to fix this
	}
}

int EventManager::getNumEventHandlers()
{
	return eventHandlers.size();
}

EventHandler* EventManager::getEventHandlerByIndex(int num)
{
	int counter = 0;
	for (std::map<std::string, EventHandler*>::iterator iter = eventHandlers.begin();
		 iter != eventHandlers.end();
		 iter++)
	{
		if (counter == num)
		{
			EventHandler* handler = (*iter).second;
			return handler;
		}
	}

	return NULL;
}

EventHandler* EventManager::getEventHandler(const std::string& type)
{
	std::map<std::string, EventHandler*>::iterator iter = eventHandlers.find(type);
	if (iter != eventHandlers.end())
	{
		EventHandler* handler = (*iter).second;
		return handler;
	}

	return NULL;
}

}
