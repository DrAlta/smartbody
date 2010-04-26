#include "ResourceManager.h"


ResourceManager* ResourceManager::manager = NULL;

ResourceManager::ResourceManager()
{
	last_resource = NULL;
	is_seq_cmd = false;
	last_seq_cmd_name = "";
}

ResourceManager::~ResourceManager()
{
	for (unsigned int r = 0; r < resources.size(); r++)
	{
		delete resources[r];
	}
}

void ResourceManager::addResource(Resource* r)
{
	// set up a limit to the number of resources that can be stored
	// since too many resources stored may cause excessive memory 
	// problems for a long-running application
	//if (maxNumResources > 1000) // or something like that
	//{
	//}

	CmdResource* cmd = dynamic_cast<CmdResource*>(r);
	if (cmd)
	{
		if (cur_cmd_parent.size() > 0)
		{
			cur_cmd_parent.top()->addChild(r);
			last_resource = cmd;
		}
		else
		{
			resources.push_back(r);
			last_resource = cmd;
		}
	}
	else
	{
		resources.push_back(r);
	}
}

void ResourceManager::removeParent()
{
	if (cur_cmd_parent.size() > 0)
		cur_cmd_parent.pop();
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

void ResourceManager::addParent(Resource* parent)
{
	cur_cmd_parent.push(parent);
}

Resource* ResourceManager::getParent()
{
	return cur_cmd_parent.top();
}

CmdResource* ResourceManager::getCmdResource(std::string id)
{

	for (unsigned int r = 0; r < resources.size(); r++)
	{
		CmdResource* cmd = dynamic_cast<CmdResource*>(resources[r]);
		if (cmd)
		{
			CmdResource* c = getCmdResourceRecurse(id, cmd);
			if (c)
				return c;
		}
		
	}
	// ...
	// ...
	return NULL;
}

CmdResource* ResourceManager::getCmdResourceRecurse(std::string id, CmdResource* r)
{
	if (r->getId() == id)
		return r;


	for (int c = 0; c < r->getNumChildren(); c++)
	{	
		CmdResource* childCmd = dynamic_cast<CmdResource*>(r->getChild(c));
		CmdResource* cmd = getCmdResourceRecurse(id, childCmd);
		if (cmd)
			return cmd;
	}

	return NULL;
}


