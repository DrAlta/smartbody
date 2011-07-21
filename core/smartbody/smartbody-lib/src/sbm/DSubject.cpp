#include "DSubject.h"
#include "DObserver.h"

DSubject::DSubject()
{
}

DSubject::~DSubject()
{
}

void DSubject::registerObserver(DObserver* observer)
{
	std::set<DObserver*>::iterator iter = m_observers.find(observer);
	if (iter == m_observers.end())
	{
		m_observers.insert(observer);
	}
}

void DSubject::unregisterObserver(DObserver* observer)
{
	std::set<DObserver*>::iterator iter = m_observers.find(observer);
	if (iter != m_observers.end())
	{
		m_observers.erase(iter);
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
