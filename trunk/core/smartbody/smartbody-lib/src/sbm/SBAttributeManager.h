#ifndef DATTRIBUTEMANAGER_H
#define DATTRIBUTEMANAGER_H

#include "SBSubject.h"
#include "SBAttribute.h"
#include <map>

namespace SmartBody {

class SBAttributeManager : public SBSubject
{
	public:
		SBAttributeManager();
		~SBAttributeManager();

		void addGroup(const std::string& name);
		SBAttributeGroup* getGroup(const std::string& name, bool createIfNotFound = false);
		std::map<std::string, SBAttributeGroup*>& getGroups();
		std::vector<SBAttributeGroup*>& getAttributeGroups();

		void notifyCreateAttribute(SBAttribute* attr);
		void notifyRemoveAttribute(SBAttribute* attr);
		void notifyPrioritizeAttribute(SBAttribute* attr);
		void notifyHideAttribute(SBAttribute* attr);

	protected:
		void resortGroups();

		std::map<std::string, SBAttributeGroup*> m_groups;
		std::vector<SBAttributeGroup*> m_groupsByPriority;
};

};

#endif
