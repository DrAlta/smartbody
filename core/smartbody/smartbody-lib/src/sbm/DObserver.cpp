#include "DObserver.h"
#include "DSubject.h"

#include <vector>

DObserver::DObserver()
{
}

DObserver::~DObserver()
{
	std::vector<DSubject*> tmpSubjects;
	for (std::set<DSubject*>::iterator iter = _subjects.begin(); 
		 iter != _subjects.end();
		 iter++)
	{
		tmpSubjects.push_back(*iter);
	}
	
	for (std::vector<DSubject*>::iterator iter = tmpSubjects.begin(); 
		 iter != tmpSubjects.end();
		 iter++)
	{
		(*iter)->unregisterObserver(this);
	}
}

void DObserver::addDependency(DSubject* subject)
{
	if (_subjects.find(subject) == _subjects.end())
	{
		_subjects.insert(subject);
	}
}

void DObserver::removeDependency(DSubject* subject)
{
	std::set<DSubject*>::iterator iter = _subjects.find(subject);
	if (iter != _subjects.end())
	{
		_subjects.erase(iter);
	}
}

