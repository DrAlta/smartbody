#include <iostream>
#include <string>

#include "resource_cmds.h"


int resource_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p  )
{
/*	// for now, show all the resources that have been loaded
	int numResources = mcu_p->resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		std::cout << mcu_p->resource_manager->getResource(r)->dump() << std::endl;
	}*/

	std::string arg = args.read_token();
	if( arg.empty() || arg=="help" ) {
		LOG("Syntax:");
		LOG("\t resource [command|path|file|motion|controller|limit]");
		return CMD_SUCCESS;
	}
	
	int numResources = mcu_p->resource_manager->getNumResources();

	if(arg=="command")
	{
		for (int r = 0; r < numResources; r++)
		{
			CmdResource * res = dynamic_cast<CmdResource  *>(mcu_p->resource_manager->getResource(r));
			if(res)
				LOG("%s", res->dump().c_str());
		}		
		return CMD_SUCCESS;
	}

	if(arg=="path")
	{
		for (int r = 0; r < numResources; r++)
		{
			PathResource * res = dynamic_cast<PathResource  *>(mcu_p->resource_manager->getResource(r));
			if(res)
				LOG("%s", res->dump().c_str());
		}		
		return CMD_SUCCESS;
	}

	if(arg=="file")
	{
		for (int r = 0; r < numResources; r++)
		{
			FileResource * res = dynamic_cast<FileResource  *>(mcu_p->resource_manager->getResource(r));
			if(res)
				LOG("%s", res->dump().c_str());
		}				
		return CMD_SUCCESS;
	}

	if(arg == "motion")
	{
		for (int r = 0; r < numResources; r++)
		{
			MotionResource * res = dynamic_cast<MotionResource  *>(mcu_p->resource_manager->getResource(r));
			if(res)
				LOG("%s", res->dump().c_str());

		}				
		return CMD_SUCCESS;		
	}
	if(arg == "controller")
	{
		for (int r = 0; r < numResources; r++)
		{
			ControllerResource * res = dynamic_cast<ControllerResource  *>(mcu_p->resource_manager->getResource(r));
			if(res)
				LOG("%s", res->dump().c_str());
		}				
		return CMD_SUCCESS;		
	}

	if(arg == "limit")
	{
		int num = args.read_int();
		if( num != 0 )
			mcu_p->resource_manager->setLimit(num);
	}
	
	return CMD_SUCCESS;
}
