#include "ResourceManager.h"


ResourceManager* ResourceManager::manager = NULL;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	for (unsigned int r = 0; r < resources.size(); r++)
		delete resources[r];
}

void ResourceManager::addResource(Resource* r)
{
	resources.push_back(r);
}

int ResourceManager::getNumResources()
{
	return resources.size();
}

Resource* ResourceManager::getResource(unsigned int index)
{
	if (index < resources.size())
		return resources[index];
	else
		return NULL;
}

ResourceManager* ResourceManager::getResourceManager()
{
	if (manager == NULL)
	{
		manager = new ResourceManager();
	}

	return manager;
}