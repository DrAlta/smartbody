#ifndef DOBSERVER_H
#define DOBSERVER_H

#include <set>

namespace SmartBody {

class SBSubject;

class SBObserver
{
	public:
		SBObserver();
		~SBObserver();

		virtual void addDependency(SBSubject* subject);
		virtual void removeDependency(SBSubject* subject);

		virtual void notify(SBSubject* subject);

	protected:
		std::set<SBSubject*> _subjects;
};

};

#endif