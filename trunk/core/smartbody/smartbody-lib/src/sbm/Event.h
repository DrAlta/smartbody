#ifndef _SBMEVENT_H
#define _SBMEVENT_H

#include <map>
#include <string>

namespace SmartBody {

class Event
{
	public:
		Event() : m_type(""), m_params("") {};
		~Event() {}
		virtual void setParameters(std::string params) { m_params = params; }
		virtual std::string getParameters() { return m_params; };
		virtual void setType(std::string type) { m_type = type; }
		virtual std::string getType() { return m_type; }
	
	protected:
		std::string m_type;
		std::string m_params;
};

class EventHandler
{
	public:
		EventHandler() : m_type(""), m_action("") {}
		~EventHandler() {}

	//	void setType(const std::string& type) { m_type = type; }
	//	const std::string& getType() { return m_type; }
		virtual void executeAction(Event* event) {}

	protected:
		std::string m_type;
		std::string m_action;
};

class BasicHandler : public EventHandler
{
	public:
		BasicHandler();
		void setAction(const std::string& action);
		const std::string& getAction();
		virtual void executeAction(Event* event);
};

typedef std::map<std::string, EventHandler*> EventHandlerMap;

class EventManager
{
	public:
		EventManager();
		~EventManager();

		void handleEvent(Event* e, double time);
		Event* createEvent(const std::string& type, const std::string parameters);
		void addEventHandler(const std::string& type, EventHandler* handle);
		void removeEventHandler(const std::string& type);
		int getNumEventHandlers();
		EventHandler* getEventHandlerByIndex(int num);
		EventHandler* getEventHandler(const std::string& type);
		static EventManager* getEventManager();
		EventHandlerMap& getEventHandlers() { return eventHandlers; }

	private:
		static EventManager* _eventManager;
		EventHandlerMap eventHandlers;
};

class MotionEvent : public Event
{
	public:
		MotionEvent() : Event(), m_time(0.0), m_isOnce(false), m_enabled(true) {};
		~MotionEvent() {};

		void setTime(double time) { m_time = time; }
		double getTime() { return m_time; }
		void setIsOnceOnly(bool val) { m_isOnce = val; }
		bool getIsOnceOnly() { return m_isOnce; }

		bool isEnabled() { return m_enabled; }
		void setEnabled(bool val) { m_enabled = val; }
		void setMotionName(const std::string& name) { m_name = name; }
		const std::string& getMotionName() { return m_name; }

	protected:
		double m_time;
		bool m_isOnce;
		bool m_enabled;
		std::string m_name;
};

}

#endif
