
#include "vhcl.h"

#include <iostream>
#include <string>
#include <boost/regex.hpp>
#include "resource_cmds.h"
#include <sbm/mcontrol_util.h>

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
		LOG("\t resource [command <max> <exprmatch>|path <exprmatch>|file <exprmatch>|motion <exprmatch>|controller <max> <exprmatch>|limit]");
		return CMD_SUCCESS;
	}
	
	int numResources = mcu_p->resource_manager->getNumResources();
	int numCommandResources = mcu_p->resource_manager->getNumCommandResources();
	int numControllerResources = mcu_p->resource_manager->getNumControllerResources();

	if(arg=="command")
	{
		int resourceLimit = 100; // only show the last 100 commands
		if (args.calc_num_tokens() > 0)
		{
			resourceLimit = args.read_int();
			if (resourceLimit < 0 || resourceLimit > numCommandResources)
				resourceLimit = numCommandResources;
		}
		std::string match = "";
		if (args.calc_num_tokens() > 0)
		{
			match = args.read_token();
		}
		boost::regex pattern(match, boost::regex_constants::icase|boost::regex_constants::perl);
		for (int r = numCommandResources - resourceLimit; r < numCommandResources; r++)
		{
			CmdResource* res =mcu_p->resource_manager->getCommandResource(r);
			if(!res)
				continue;
			bool foundMatch = true;
			if (match.size() > 0)
			{
				foundMatch = boost::regex_search (res->getCommand(), pattern, boost::regex_constants::format_perl);
			}
			if(foundMatch)
				LOG("%s", res->dump().c_str());
		}		
		return CMD_SUCCESS;
	}

	if(arg=="path")
	{
		std::string match = "";
		if (args.calc_num_tokens() > 0)
		{
			match = args.read_token();
		}
		boost::regex pattern(match, boost::regex_constants::icase|boost::regex_constants::perl);
		for (int r = 0; r < numResources; r++)
		{
			PathResource * res = dynamic_cast<PathResource  *>(mcu_p->resource_manager->getResource(r));
			if(!res)
				continue;
			bool foundMatch = true;
			if (match.size() > 0)
			{
				foundMatch = boost::regex_search (res->getPath(), pattern, boost::regex_constants::format_perl);
			}
			if(foundMatch)
				LOG("%s", res->dump().c_str());
		}		
		return CMD_SUCCESS;
	}

	if(arg=="file")
	{
		std::string match = "";
		if (args.calc_num_tokens() > 0)
		{
			match = args.read_token();
		}
		boost::regex pattern(match, boost::regex_constants::icase|boost::regex_constants::perl);
		for (int r = 0; r < numResources; r++)
		{
			FileResource * res = dynamic_cast<FileResource  *>(mcu_p->resource_manager->getResource(r));
			if (!res)
				continue;
			bool foundMatch = true;
			if (match.size() > 0)
			{
				foundMatch = boost::regex_search (res->getFilePath(), pattern, boost::regex_constants::format_perl);
			}
			if(foundMatch)
				LOG("%s", res->dump().c_str());
		}				
		return CMD_SUCCESS;
	}

	if(arg == "motion")
	{
		std::string match = "";
		if (args.calc_num_tokens() > 0)
		{
			match = args.read_token();
		}
		
		boost::regex pattern(match, boost::regex_constants::icase|boost::regex_constants::perl);
		for (int r = 0; r < numResources; r++)
		{
			MotionResource * res = dynamic_cast<MotionResource  *>(mcu_p->resource_manager->getResource(r));
			if (!res)
				continue;
			bool foundMatch = true;
			if (match.size() > 0)
			{
				foundMatch = boost::regex_search (res->getMotionFile(), pattern, boost::regex_constants::format_perl);
			}
			if(foundMatch)
				LOG("%s", res->dump().c_str());

		}				
		return CMD_SUCCESS;		
	}
	if(arg == "skeleton")
	{
		std::string match = "";
		if (args.calc_num_tokens() > 0)
		{
			match = args.read_token();
		}
		boost::regex pattern(match, boost::regex_constants::icase|boost::regex_constants::perl);
		for (int r = 0; r < numResources; r++)
		{
			SkeletonResource * res = dynamic_cast<SkeletonResource  *>(mcu_p->resource_manager->getResource(r));
			if (!res)
				continue;
			bool foundMatch = true;
			if (match.size() > 0)
			{
				foundMatch = boost::regex_search (res->getSkeletonFile(), pattern, boost::regex_constants::format_perl);
			}
			if(foundMatch)
				LOG("%s", res->dump().c_str());

		}				
		return CMD_SUCCESS;		
	}
	if(arg == "controller")
	{
		int resourceLimit = 100;
		if (args.calc_num_tokens() > 0)
		{
			resourceLimit = args.read_int();
			if (resourceLimit < 0 || resourceLimit > numControllerResources)
				resourceLimit = numControllerResources;
		}
		std::string match = "";
		if (args.calc_num_tokens() > 0)
		{
			match = args.read_token();
		}
		boost::regex pattern(match, boost::regex_constants::icase|boost::regex_constants::perl);
		for (int r = numControllerResources - resourceLimit; r < numControllerResources; r++)
		{
			ControllerResource* res =mcu_p->resource_manager->getControllerResource(r);
			if (!res)
				continue;
			bool foundMatch = true;
			if (match.size() > 0)
			{
				foundMatch = boost::regex_search (res->getControllerName(), pattern, boost::regex_constants::format_perl);
			}
			if(foundMatch)
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
