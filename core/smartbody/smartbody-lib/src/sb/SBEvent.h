#ifndef _SBMEVENT_H
#define _SBMEVENT_H

#include <map>
#include <string>

namespace SmartBody {

class SBEvent
{
	public:
		SBEvent() : m_type(""), m_params("") {};
		~SBEvent() {}
		virtual void setParameters(std::string params) { m_params = params; }
		virtual std::string getParameters() { return m_params; };
		virtual void setType(std::string type) { m_type = type; }
		virtual std::string getType() { return m_type; }
	
	protected:
		std::string m_type;
		std::string m_params;
};

class SBEventHandler
{
	public:
		SBEventHandler() : m_type(""), m_action("") {}
		~SBEventHandler() {}

	//	void setType(const std::string& type) { m_type = type; }
	//	const std::string& getType() { return m_type; }
		virtual void executeAction(SBEvent* event) {}

	protected:
		std::string m_type;
		std::string m_action;
};

class SBBasicHandler : public SBEventHandler
{
	public:
		SBBasicHandler();
		void setAction(const std::string& action);
		const std::string& getAction();
		virtual void executeAction(SBEvent* event);
};

typedef std::map<std::string, SBEventHandler*> SBEventHandlerMap;

class SBEventManager
{
	public:
		SBEventManager();
		~SBEventManager();

		void handleEvent(SBEvent* e, double time);
		SBEvent* createEvent(const std::string& type, const std::string parameters);
		void addEventHandler(const std::string& type, SBEventHandler* handle);
		void removeEventHandler(const std::string& type);
		int getNumEventHandlers();
		SBEventHandler* getEventHandlerByIndex(int num);
		SBEventHandler* getEventHandler(const std::string& type);
		static SBEventManager* getEventManager();
		SBEventHandlerMap& getEventHandlers() { return eventHandlers; }

	private:
		static SBEventManager* _eventManager;
		SBEventHandlerMap eventHandlers;
};

class SBMotionEvent : public SBEvent
{
	public:
		SBMotionEvent() : SBEvent(), m_time(0.0), m_isOnce(false), m_enabled(true) {};
		~SBMotionEvent() {};

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
