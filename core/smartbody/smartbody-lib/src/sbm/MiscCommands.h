#ifndef _MISC_COMMANDS_H
#define _MISC_COMMANDS_H

#include <string>
#include <sbm/sr_arg_buff.h>

class mcuCBHandle;
class SbmPawn;

int pawn_set_cmd_funcx( srArgBuffer& args, mcuCBHandle* mcu_p);
int pawn_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p);
int pawn_print_cmd_funcx( srArgBuffer& args, mcuCBHandle* mcu_p);
int pawn_parse_pawn_command( SbmPawn* pawn, std::string cmd, srArgBuffer& args);

int character_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p);
int create_remote_pawn_func( srArgBuffer& args, mcuCBHandle* mcu_p);
int character_set_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p);
int character_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p);
int character_print_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p);

int set_world_offset_cmd( SbmPawn* pawn, srArgBuffer& args );


#endif