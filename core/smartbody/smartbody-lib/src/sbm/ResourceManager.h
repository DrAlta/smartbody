#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "Resource.h"
#include <vector>

class ResourceManager
{
	public:
		ResourceManager();
		~ResourceManager();

		void addResource(Resource* r);
		int getNumResources();
		Resource* getResource(unsigned int index);

		static ResourceManager* getResourceManager();
	
	private:
		std::vector<Resource*> resources;
		static ResourceManager* manager;
};
#endif
