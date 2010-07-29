#include "ResourceManager.h"


ResourceManager* ResourceManager::manager = NULL;

ResourceManager::ResourceManager()
{
	resource_limit = 1000;
	last_resource = NULL;
	is_seq_cmd = false;
	last_seq_cmd_name = "";
}

ResourceManager::~ResourceManager()
{
	for (std::list<Resource*>::iterator iter = resources.begin();
		iter != resources.end();
		iter++)
	{
		delete (*iter);
	}
}

void ResourceManager::addResource(Resource* r)
{
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
			while(resources.size() >= resource_limit)
				resources.pop_front();
			resources.push_back(r);
			last_resource = cmd;
		}
	}
	else
	{
		if(resources.size() >= resource_limit)
			resources.pop_front();
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
	std::list<Resource *>::iterator iter= resources.begin();
	for(unsigned int i = 0 ; i < index; i++)
	{
		iter ++;
		if(iter == resources.end())	return NULL;
	}
	return *iter;
}

ResourceManager* ResourceManager::getResourceManager()
{
	if (manager == NULL)
	{
		manager = new ResourceManager();
	}

	return manager;
}

void ResourceManager::cleanup()
{
	if (manager)
		delete manager;
	manager = NULL;
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
	std::list<Resource *>::iterator iter= resources.begin();
	for (unsigned int r = 0; r < resources.size(); r++)
	{
		iter ++;
		if(iter == resources.end())	return NULL;
		CmdResource* cmd = dynamic_cast<CmdResource*>(*iter);
		if (cmd)
		{
			CmdResource* c = getCmdResourceRecurse(id, cmd);
			if (c)
				return c;
		}
		
	}
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


void ResourceManager::setLimit(unsigned int l)
{
	resource_limit = l;
	while(l < resources.size())
		resources.pop_front();
}

int ResourceManager::getLimit()
{
	return resource_limit;
}
