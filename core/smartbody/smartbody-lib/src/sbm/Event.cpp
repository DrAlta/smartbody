//#include "vhcl_log.h"
#include "Event.h"
#include "mcontrol_util.h"

#include <boost/algorithm/string/replace.hpp>


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
	std::string action = handler->getAction();

	boost::replace_all(action, "$1", e->getParameters());
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.execute((char*) action.c_str());
}

void EventManager::addHandler(EventHandler* handler)
{
	removeHandler(handler->getType());
	eventHandlers.insert(std::pair<std::string, EventHandler*>(handler->getType(), handler));
}

void EventManager::removeHandler(std::string type)
{
	std::map<std::string, EventHandler*>::iterator iter = eventHandlers.find(type);
	if (iter != eventHandlers.end())
	{
		EventHandler* oldHandler = (*iter).second;
		eventHandlers.erase(iter);
		delete oldHandler;

	}
}
