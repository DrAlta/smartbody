#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "Resource.h"
#include <vector>
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
		Resource* getLastCmdResource();
		CmdResource* getCmdResource(std::string id);

		void setIsSeqCmd(bool val);
		bool isSeqCmd();
		std::string getSeqCmdName();
		void setSeqCmdName(std::string seqcmd);

		static ResourceManager* getResourceManager();
		
	
	private:
		CmdResource* getCmdResourceRecurse(std::string id, CmdResource* r);
		
		std::vector<Resource*> resources;
		static ResourceManager* manager;
		std::stack<Resource*> cur_parent;
		CmdResource* last_resource;
		std::string last_seq_cmd_name;
		bool is_seq_cmd;
};
#endif
