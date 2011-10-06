#ifndef DOBSERVER_H
#define DOBSERVER_H

#include <set>

class DSubject;

class DObserver
{
	public:
		DObserver();
		~DObserver();

		virtual void addDependency(DSubject* subject);
		virtual void removeDependency(DSubject* subject);

		virtual void notify(DSubject* subject) = 0;

	protected:
		std::set<DSubject*> _subjects;
};

#endif