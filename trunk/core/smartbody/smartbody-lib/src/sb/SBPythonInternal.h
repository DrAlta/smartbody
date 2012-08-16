#ifndef _SBMPYTHONINTERNAL_
#define _SBMPYTHONINTERNAL_




struct NvbgWrap :  Nvbg, boost::python::wrapper<Nvbg>
{
	virtual void objectEvent(std::string character, std::string name, bool isAnimate, SrVec position, SrVec velocity, SrVec relativePosition, SrVec relativeVelocity)
	{
		if (boost::python::override o = this->get_override("objectEvent"))
		{
			try {
				o(character, name, isAnimate, position, velocity, relativePosition, relativeVelocity);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_objectEvent(std::string character, std::string name, bool isAnimate, SrVec position, SrVec velocity, SrVec relativePosition, SrVec relativeVelocity)
	{
		return Nvbg::objectEvent(character, name, isAnimate, position, velocity, relativePosition, relativeVelocity);
	}

	virtual bool execute(std::string character, std::string to, std::string messageId, std::string xml)
	{
		if (boost::python::override o = this->get_override("execute"))
		{
			try {
				return o(character, to, messageId, xml);
			} catch (...) {
				PyErr_Print();
			}
		}

		return Nvbg::execute(character, to, messageId, xml);
	}

	bool default_execute(std::string character, std::string to, std::string messageId, std::string xml)
	{
		return Nvbg::execute(character, to, messageId, xml);
	}

	virtual bool executeEvent(std::string character, std::string messageId, std::string state)
	{
		if (boost::python::override o = this->get_override("executeEvent"))
		{
			try {
				return o(character, messageId, state);
			} catch (...) {
				PyErr_Print();
			}
		}

		return Nvbg::executeEvent(character, messageId, state);
	}

	bool default_executeEvent(std::string character, std::string messageId, std::string state)
	{
		return Nvbg::executeEvent(character, messageId, state);
	}

	virtual bool executeSpeech(std::string character, std::string speechStatus, std::string speechId, std::string speaker)
	{
		if (boost::python::override o = this->get_override("executeSpeech"))
		{
			try {
				return o(character, speechStatus, speechId, speaker);
			} catch (...) {
				PyErr_Print();
			}
		}

		return Nvbg::executeSpeech(character, speechStatus, speechId, speaker);
	}


	bool default_executeSpeech(std::string character, std::string speechStatus, std::string speechId, std::string speaker)
	{
		return Nvbg::executeSpeech(character, speechStatus, speechId, speaker);
	}

	virtual void notifyAction(std::string name)
	{
		if (boost::python::override o = this->get_override("notifyAction"))
		{
			try {
				o(name);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyAction(std::string name)
	{
		notifyLocal(name);
	}

	virtual void notifyBool(std::string name, bool val)
	{
		if (boost::python::override o = this->get_override("notifyBool"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyBool(std::string name, bool val)
	{
		notifyLocal(name);
	}

	virtual void notifyInt(std::string name, int val)
	{
		if (boost::python::override o = this->get_override("notifyInt"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyInt(std::string name, int val)
	{
		notifyLocal(name);
	}

	virtual void notifyDouble(std::string name, double val)
	{
		if (boost::python::override o = this->get_override("notifyDouble"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyDouble(std::string name, double val)
	{
		notifyLocal(name);
	}

	virtual void notifyString(std::string name, std::string val)
	{
		if (boost::python::override o = this->get_override("notifyString"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyString(std::string name, std::string val)
	{
		notifyLocal(name);
	}

	virtual void notifyVec3(std::string name, SrVec val)
	{
		if (boost::python::override o = this->get_override("notifyVec3"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyVec3(std::string name, SrVec val)
	{
		notifyLocal(name);
	}

	virtual void notifyMatrix(std::string name, SrMat val)
	{
		if (boost::python::override o = this->get_override("notifyMatrix"))
		{
			try {
				o(name, val);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_notifyMatrix(std::string name, SrMat val)
	{
		notifyLocal(name);
	}


};


struct SBScriptWrap :  SBScript, boost::python::wrapper<SBScript>
{
	virtual void start()
	{
		if (boost::python::override o = this->get_override("start"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_start()
	{
		return SBScript::start();
	}

	virtual void stop()
	{
		if (boost::python::override o = this->get_override("stop"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_stop()
	{
		return SBScript::stop();
	}

	virtual void beforeUpdate(double time)
	{
		if (boost::python::override o = this->get_override("beforeUpdate"))
		{
			try {
				o(time);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_beforeUpdate(double time)
	{
		return SBScript::beforeUpdate(time);
	}

	virtual void update(double time)
	{
		if (boost::python::override o = this->get_override("update"))
		{
			try {
				o(time);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_update(double time)
	{
		return SBScript::update(time);
	}

	virtual void afterUpdate(double time)
	{
		if (boost::python::override o = this->get_override("afterUpdate"))
		{
			try {
				o(time);
			} catch (...) {
				PyErr_Print();
			}
		}
	}

	void default_afterUpdate(double time)
	{
		return SBScript::afterUpdate(time);
	}
};


struct EventHandlerWrap :  EventHandler, boost::python::wrapper<EventHandler>
{
	virtual void executeAction(Event* event)
	{
		if (boost::python::override o = this->get_override("executeAction"))
		{
			try {
				o(event);
			} catch (...) {
				PyErr_Print();
			}
		}

		return EventHandler::executeAction(event);
	}

	void default_executeAction(Event* event)
	{
		EventHandler::executeAction(event);
	}
};


struct PythonControllerWrap : SmartBody::PythonController, boost::python::wrapper<SmartBody::PythonController>
{
	virtual void start()
	{
		if (boost::python::override o = this->get_override("start"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}

		return PythonController::start();
	};

	void default_start()
	{
		SmartBody::PythonController::start();
	}

	virtual void init()
	{
		if (boost::python::override o = this->get_override("init"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}

		return PythonController::init();
	};

	void default_init()
	{
		SmartBody::PythonController::init();
	}

	virtual void evaluate()
	{
		if (boost::python::override o = this->get_override("evaluate"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}

		return PythonController::evaluate();
	};

	void default_evaluate()
	{
		SmartBody::PythonController::evaluate();
	}

	virtual void stop()
	{
		if (boost::python::override o = this->get_override("stop"))
		{
			try {
				o();
			} catch (...) {
				PyErr_Print();
			}
		}

		return PythonController::stop();
	};

	void default_stop()
	{
		SmartBody::PythonController::stop();
	}
};




// wrapper for std::map
template<class T>
struct map_item
{
	typedef typename T::key_type K;
	typedef typename T::mapped_type V;
	static V get(T const& x, K const& i)
	{
		V temp;
		if( x.find(i) != x.end() ) 
			return x.find(i)->second;
		PyErr_SetString(PyExc_KeyError, "Key not found");
		return temp;		
	}
	static void set(T & x, K const& i, V const& v)
	{
		x[i]=v; // use map autocreation feature
	}
	static void del(T & x, K const& i)
	{
		if( x.find(i) != x.end() ) x.erase(i);
		else PyErr_SetString(PyExc_KeyError, "Key not found");
	}
};


#endif
