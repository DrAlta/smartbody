#include "ResourceManager.h"


SBResourceManager* SBResourceManager::manager = NULL;

SBResourceManager::SBResourceManager()
{
	resource_limit = 1000;
	last_resource = NULL;
	is_seq_cmd = false;
	last_seq_cmd_name = "";
}

SBResourceManager::~SBResourceManager()
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

	last_resource = NULL;
}

void SBResourceManager::addResource(Resource* r)
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

void SBResourceManager::addCommandResource(CmdResource* cmd)
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

void SBResourceManager::addControllerResource(ControllerResource* r)
{
		if(controllerResources.size() >= resource_limit)
		{
			ControllerResource* last = controllerResources.front();
			controllerResources.pop_front();
			delete last;
		}
		controllerResources.push_back(r);
}

void SBResourceManager::removeParent()
{
	if (cur_cmd_parent.size() > 0)
		cur_cmd_parent.pop();
}


int SBResourceManager::getNumResources()
{
	return resources.size();
}

int SBResourceManager::getNumCommandResources()
{
	return commandResources.size();
}

int SBResourceManager::getNumControllerResources()
{
	return controllerResources.size();
}

Resource* SBResourceManager::getResource(unsigned int index)
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

CmdResource* SBResourceManager::getCommandResource(unsigned int index)
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

ControllerResource* SBResourceManager::getControllerResource(unsigned int index)
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

SBResourceManager* SBResourceManager::getResourceManager()
{
	if (manager == NULL)
	{
		manager = new SBResourceManager();
	}

	return manager;
}

void SBResourceManager::cleanup()
{
	if (manager)
		delete manager;
	manager = NULL;
}
void SBResourceManager::addParent(CmdResource* parent)
{
	cur_cmd_parent.push(parent);
}

Resource* SBResourceManager::getParent()
{
	return cur_cmd_parent.top();
}

CmdResource* SBResourceManager::getCmdResource(std::string id)
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

CmdResource* SBResourceManager::getCmdResourceRecurse(std::string id, CmdResource* r)
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


void SBResourceManager::setLimit(unsigned int l)
{
	resource_limit = l;
	while(l < resources.size())
	{
		Resource* last = resources.front();
		resources.pop_front();
		delete last;
	}
}

int SBResourceManager::getLimit()
{
	return resource_limit;
}
