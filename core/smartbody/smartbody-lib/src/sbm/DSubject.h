#ifndef DSUBJECT_H
#define DSUBJECT_H

#include <set>

class DObserver;

class DSubject
{
	public:
		DSubject();
		~DSubject();

		virtual void registerObserver(DObserver* observer);
		virtual void unregisterObserver(DObserver* observer);
		virtual void notifyObservers();

	protected:
		std::set<DObserver*> m_observers;

};
#endif