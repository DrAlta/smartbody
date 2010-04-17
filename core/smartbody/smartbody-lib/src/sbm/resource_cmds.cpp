#include <iostream>
#include <string>

#include "resource_cmds.h"


int resource_cmd_func( srArgBuffer& args, mcuCBHandle *mcu_p  )
{
	// for now, show all the resources that have been loaded
	int numResources = mcu_p->resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		std::cout << mcu_p->resource_manager->getResource(r)->dump() << std::endl;
	}
	return CMD_SUCCESS;
}
