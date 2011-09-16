#include "DAttributeManager.h"
#include <algorithm>

DAttributeManager::DAttributeManager() : DSubject()
{
}

DAttributeManager::~DAttributeManager()
{
	for (std::map<std::string, DAttributeGroup*>::iterator iter = m_groups.begin();
		 iter != m_groups.end();
		 iter++)
	{
		delete (*iter).second;
	}
}

void DAttributeManager::addGroup(const std::string& name)
{
	for (std::map<std::string, DAttributeGroup*>::iterator iter = m_groups.begin();
		 iter != m_groups.end();
		 iter++)
	{
		if (name == (*iter).second->getName())
			return;
	}

	DAttributeGroup* group = new DAttributeGroup(name);
	m_groups[name] = group;
	resortGroups();
}

DAttributeGroup* DAttributeManager::getGroup(const std::string& name, bool createIfNotFound)
{
	for (std::map<std::string, DAttributeGroup*>::iterator iter = m_groups.begin();
		 iter != m_groups.end();
		 iter++)
	{
		if (name == (*iter).second->getName())
		{
			return (*iter).second;
		}
	}

	if (createIfNotFound)
	{
		DAttributeGroup* group = new DAttributeGroup(name);
		m_groups[name] = group;
		resortGroups();
		return group;
	}
	else
	{
		return NULL;
	}
}

struct prioritySort
{
  bool operator()(DAttributeGroup* a, DAttributeGroup* b)
  {
	  return a->getName() < b->getName();
  }
};

void DAttributeManager::resortGroups()
{
	m_groupsByPriority.clear();
	for (std::map<std::string, DAttributeGroup*>::iterator iter = m_groups.begin();
		iter != m_groups.end();
		iter++)
	{
		m_groupsByPriority.push_back((*iter).second);
	}

	// for now, sort alphabetically and then assign the priority numbers later
	// in the future, set the priority manually
	std::sort(m_groupsByPriority.begin(), m_groupsByPriority.end(), prioritySort());
	for (size_t g = 0; g < m_groupsByPriority.size(); g++)
		m_groupsByPriority[g]->setPriority(g);
}

std::map<std::string, DAttributeGroup*>& DAttributeManager::getGroups()
{
	return m_groups;
}

std::vector<DAttributeGroup*>& DAttributeManager::getAttributeGroups()
{
	return m_groupsByPriority;
}

void DAttributeManager::notifyCreateAttribute(DAttribute* attr)
{
	notifyObservers();
}

void DAttributeManager::notifyRemoveAttribute(DAttribute* attr)
{
	notifyObservers();
}

void DAttributeManager::notifyPrioritizeAttribute(DAttribute* attr)
{
	notifyObservers();
}

void DAttributeManager::notifyHideAttribute(DAttribute* attr)
{
	notifyObservers();
}
