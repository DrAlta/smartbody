/*************************************************************
Copyright (C) 2017 University of Southern California

This file is part of Smartbody.

Smartbody is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Smartbody is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Smartbody.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************/

#ifndef _SBMEVENT_H
#define _SBMEVENT_H

#include <sb/SBTypes.h>
#include <map>
#include <string>
#include <sb/SBObject.h>

namespace SmartBody {

class SBEvent
{
	public:
		SBAPI SBEvent() : m_type(""), m_params("") {};
		SBAPI ~SBEvent() {}
		SBAPI virtual void setParameters(std::string params) { m_params = params; }
		SBAPI virtual std::string getParameters() { return m_params; };
		SBAPI virtual void setType(std::string type) { m_type = type; }
		SBAPI virtual std::string getType() { return m_type; }
	
	protected:
		std::string m_type;
		std::string m_params;
};

class SBEventHandler : public SBObject
{
	public:
		SBAPI SBEventHandler();
		SBAPI ~SBEventHandler() {}

		SBAPI void setEnable(bool val);
		SBAPI bool isEnable();

	//	void setType(const std::string& type) { m_type = type; }
	//	const std::string& getType() { return m_type; }
		SBAPI virtual void executeAction(SBEvent* event) {}
		SBAPI virtual void notify(SBSubject* subject);

	protected:
		std::string m_type;
		std::string m_action;
		bool _enabled;
};

class SBBasicHandler : public SBEventHandler
{
	public:
		SBAPI SBBasicHandler();
		SBAPI void setAction(const std::string& action);
		SBAPI const std::string& getAction();
		SBAPI virtual void executeAction(SBEvent* event);
		SBAPI void notify(SBSubject* subject);
};

class SBEventManager
{
	public:
		SBAPI SBEventManager();
		SBAPI ~SBEventManager();

		SBAPI void handleEvent(SBEvent* e);
		SBAPI SBEvent* createEvent(const std::string& type, const std::string parameters);
		SBAPI void addEventHandler(const std::string& type, SBEventHandler* handle);
		SBAPI void removeEventHandler(const std::string& type);
		SBAPI int getNumEventHandlers();
		SBAPI SBEventHandler* getEventHandlerByIndex(int num);
		SBAPI SBEventHandler* getEventHandler(const std::string& type);
		SBAPI std::map<std::string, SBEventHandler*>& getEventHandlers();

	private:
		std::map<std::string, SBEventHandler*> eventHandlers;
};

class SBMotionEvent : public SBEvent
{
	public:
		SBAPI SBMotionEvent() : SBEvent(), m_time(0.0), m_isOnce(false), m_enabled(true) {};
		SBAPI ~SBMotionEvent() {};

		SBAPI void setTime(double time) { m_time = time; }
		SBAPI double getTime() { return m_time; }
		SBAPI void setIsOnceOnly(bool val) { m_isOnce = val; }
		SBAPI bool getIsOnceOnly() { return m_isOnce; }

		SBAPI bool isEnabled() { return m_enabled; }
		SBAPI void setEnabled(bool val) { m_enabled = val; }
		SBAPI void setMotionName(const std::string& name) { m_name = name; }
		SBAPI const std::string& getMotionName() { return m_name; }

	protected:
		double m_time;
		bool m_isOnce;
		bool m_enabled;
		std::string m_name;
};

}

#endif
