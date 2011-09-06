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
        resources.clear();

	for (std::list<CmdResource*>::iterator iter = commandResources.begin();
		iter != commandResources.end();
		iter++)
	{
		delete (*iter);
	}
        commandResources.clear();
/*
	for (std::list<ControllerResource*>::iterator iter = controllerResources.begin();
		iter != controllerResources.end();
		iter++)
	{
		delete (*iter);
	}
        controllerResources.clear();
*/
	while (!cur_cmd_parent.empty())
	{
		Resource* r = cur_cmd_parent.top();
		delete r;
		cur_cmd_parent.pop();
	}
}

void ResourceManager::addResource(Resource* r)
{
	CmdResource* cmd = dynamic_cast<CmdResource*>(r);
	if (cmd)
	{
		addCommandResource(cmd);
		return;
	}
	ControllerResource* ctrl = dynamic_cast<ControllerResource*>(r);
	if (ctrl)
	{
		addControllerResource(ctrl);
		return;
	}
	if(resources.size() >= resource_limit)
	{
		Resource* last = resources.front();
		resources.pop_front();
		delete last;
	}
	resources.push_back(r);
}

void ResourceManager::addCommandResource(CmdResource* cmd)
{
	if (cur_cmd_parent.size() > 0)
	{
		cur_cmd_parent.top()->addChild(cmd);
		last_resource = cmd;
	}
	else
	{
		while(commandResources.size() >= resource_limit)
		{
			CmdResource* last = commandResources.front();
			commandResources.pop_front();
			delete last;
		}
		commandResources.push_back(cmd);
		last_resource = cmd;
	}
}

void ResourceManager::addControllerResource(ControllerResource* r)
{
		if(controllerResources.size() >= resource_limit)
		{
			ControllerResource* last = controllerResources.front();
			controllerResources.pop_front();
			delete last;
		}
		controllerResources.push_back(r);
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

int ResourceManager::getNumCommandResources()
{
	return commandResources.size();
}

int ResourceManager::getNumControllerResources()
{
	return controllerResources.size();
}

Resource* ResourceManager::getResource(unsigned int index)
{
	std::list<Resource *>::iterator iter= resources.begin();
	for(unsigned int i = 0 ; i < index; i++)
	{
		iter ++;
		if (iter == resources.end())
			return NULL;
	}
	return *iter;
}

CmdResource* ResourceManager::getCommandResource(unsigned int index)
{
	std::list<CmdResource *>::iterator iter= commandResources.begin();
	for (unsigned int i = 0 ; i < index; i++)
	{
		iter ++;
		if(iter == commandResources.end())
			return NULL;
	}
	return *iter;
}

ControllerResource* ResourceManager::getControllerResource(unsigned int index)
{
	std::list<ControllerResource *>::iterator iter= controllerResources.begin();
	for (unsigned int i = 0 ; i < index; i++)
	{
		iter ++;
		if(iter == controllerResources.end())
			return NULL;
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
void ResourceManager::addParent(CmdResource* parent)
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
	{
		Resource* last = resources.front();
		resources.pop_front();
		delete last;
	}
}

int ResourceManager::getLimit()
{
	return resource_limit;
}
