#ifndef _SBMEVENT_H
#define _SBMEVENT_H

#include <map>
#include <string>

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

		void setType(const std::string& type) { m_type = type; }
		const std::string& getType() { return m_type; }
		void setAction(const std::string& action) { m_action = action; }
		const std::string& getAction() { return m_action; }

	protected:
		std::string m_type;
		std::string m_action;
};

typedef std::map<std::string, EventHandler*> EventHandlerMap;

class EventManager
{
	public:
		EventManager();
		~EventManager();

		void handleEvent(Event* e, double time);
		void addHandler(EventHandler* handle);
		void removeHandler(std::string type);
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

	protected:
		double m_time;
		bool m_isOnce;
		bool m_enabled;
};

#endif
