#include "DSubject.h"
#include "DObserver.h"

DSubject::DSubject()
{
}

DSubject::~DSubject()
{
	for (std::set<DObserver*>::iterator iter = m_observers.begin();
		 iter !=  m_observers.end();
		 iter++)
	{
		(*iter)->removeDependency(this);
	}
}

void DSubject::registerObserver(DObserver* observer)
{
	std::set<DObserver*>::iterator iter = m_observers.find(observer);
	if (iter == m_observers.end())
	{
		m_observers.insert(observer);
		observer->addDependency(this);
	}
}

void DSubject::unregisterObserver(DObserver* observer)
{
	std::set<DObserver*>::iterator iter = m_observers.find(observer);
	if (iter != m_observers.end())
	{
		m_observers.erase(iter);
		observer->removeDependency(this);
	}
}

void DSubject::notifyObservers()
{
	for (std::set<DObserver*>::iterator iter = m_observers.begin();
		 iter != m_observers.end();
		 iter++)
	{
		(*iter)->notify(this);
	}
}
