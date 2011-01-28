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
		void addCommandResource(CmdResource* r);
		void addControllerResource(ControllerResource* r);
		int getNumResources();
		int getNumCommandResources();
		int getNumControllerResources();
		Resource* getResource(unsigned int index);
		CmdResource* getCommandResource(unsigned int index);
		ControllerResource* getControllerResource(unsigned int index);

		void addParent(CmdResource* parent);
		void removeParent();
		Resource* getParent();
		CmdResource* getCmdResource(std::string id);

		void setLimit(unsigned int l);
		int getLimit();

		static ResourceManager* getResourceManager();
		static void cleanup();
		
	
	private:
		static ResourceManager* manager;

		CmdResource* getCmdResourceRecurse(std::string id, CmdResource* r);		
		std::list<Resource*> resources;
		std::list<CmdResource*> commandResources;
		std::list<ControllerResource*> controllerResources;
		std::stack<CmdResource*> cur_cmd_parent;
		CmdResource* last_resource;
		std::string last_seq_cmd_name;
		bool is_seq_cmd;
		unsigned int resource_limit;
};
#endif
