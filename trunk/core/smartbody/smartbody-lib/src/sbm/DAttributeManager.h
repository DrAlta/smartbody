#ifndef DATTRIBUTEMANAGER_H
#define DATTRIBUTEMANAGER_H

#include "DSubject.h"
#include "DAttribute.h"
#include <map>

class DAttributeManager : public DSubject
{
	public:
		DAttributeManager();
		~DAttributeManager();

		void addGroup(std::string name);
		DAttributeGroup* getGroup(std::string name, bool createIfNotFound = false);
		std::map<std::string, DAttributeGroup*>& getGroups();
		std::vector<DAttributeGroup*>& getAttributeGroups();

		void notifyCreateAttribute(DAttribute* attr);
		void notifyRemoveAttribute(DAttribute* attr);
		void notifyPrioritizeAttribute(DAttribute* attr);
		void notifyHideAttribute(DAttribute* attr);

	protected:
		void resortGroups();

		std::map<std::string, DAttributeGroup*> m_groups;
		std::vector<DAttributeGroup*> m_groupsByPriority;
};
#endif
