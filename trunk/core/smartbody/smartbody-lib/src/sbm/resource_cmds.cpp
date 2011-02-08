
#include "vhcl.h"

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
		LOG("\t resource [command <max>|path <exprmatch>|file <exprmatch>|motion <exprmatch>|controller <exprmatch>|limit]");
		return CMD_SUCCESS;
	}
	
	int numResources = mcu_p->resource_manager->getNumResources();
	int numCommandResources = mcu_p->resource_manager->getNumCommandResources();
	int numControllerResources = mcu_p->resource_manager->getNumControllerResources();

	if(arg=="command")
	{
		int resourceLimit = numCommandResources;
		if (args.calc_num_tokens() > 0)
		{
			resourceLimit = args.read_int();
			if (resourceLimit < 0 || resourceLimit > numCommandResources)
				resourceLimit = numCommandResources;
		}
		for (int r = numCommandResources - resourceLimit; r < numCommandResources; r++)
		{
			CmdResource* res =mcu_p->resource_manager->getCommandResource(r);
			if(res)
				LOG("%s", res->dump().c_str());
		}		
		return CMD_SUCCESS;
	}

	if(arg=="path")
	{
		std::string match = "";
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
	if(arg == "skeleton")
	{
		for (int r = 0; r < numResources; r++)
		{
			SkeletonResource * res = dynamic_cast<SkeletonResource  *>(mcu_p->resource_manager->getResource(r));
			if(res)
				LOG("%s", res->dump().c_str());

		}				
		return CMD_SUCCESS;		
	}
	if(arg == "controller")
	{
		int resourceLimit = numControllerResources;
		if (args.calc_num_tokens() > 0)
		{
			resourceLimit = args.read_int();
			if (resourceLimit < 0 || resourceLimit > numControllerResources)
				resourceLimit = numControllerResources;
		}
		for (int r = numControllerResources - resourceLimit; r < numControllerResources; r++)
		{
			ControllerResource* res =mcu_p->resource_manager->getControllerResource(r);
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
