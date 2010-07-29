#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "Resource.h"
#include <list>
#include <stack>

class ResourceManager
{
	public:
		ResourceManager();
		~ResourceManager();

		void addResource(Resource* r);
		int getNumResources();
		Resource* getResource(unsigned int index);

		void addParent(Resource* parent);
		void removeParent();
		Resource* getParent();
		CmdResource* getCmdResource(std::string id);

		void setLimit(unsigned int l);
		int getLimit();

		static ResourceManager* getResourceManager();
		static void cleanup();
		
	
	private:
		CmdResource* getCmdResourceRecurse(std::string id, CmdResource* r);
		
		std::list<Resource*> resources;
		static ResourceManager* manager;
		std::stack<Resource*> cur_cmd_parent;
		CmdResource* last_resource;
		std::string last_seq_cmd_name;
		bool is_seq_cmd;
		unsigned int resource_limit;
};
#endif
