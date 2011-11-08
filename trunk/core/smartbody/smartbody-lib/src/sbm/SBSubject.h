#ifndef DSUBJECT_H
#define DSUBJECT_H

#include <set>

namespace SmartBody {

class SBObserver;

class SBSubject
{
	public:
		SBSubject();
		~SBSubject();

		virtual void registerObserver(SBObserver* observer);
		virtual void unregisterObserver(SBObserver* observer);
		virtual void notifyObservers();

	protected:
		std::set<SBObserver*> m_observers;

};

};

#endif