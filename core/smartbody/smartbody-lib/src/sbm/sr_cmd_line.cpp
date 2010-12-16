#include "sr_cmd_line.h"
#include <sstream>
#include <vhcl_log.h>
#include "mcontrol_util.h"

srCmdLine::srCmdLine( int len )
{
	buffer_len = 0;
	buffer_use = 0;
	cmd_buffer = 0x0;
	max_cmdlines = 10;
	int err = realloc_buffer( len );
	cmds = new std::list<std::string>;
	iter = cmds->end();
}

srCmdLine::~srCmdLine()
{
	if( cmd_buffer )
		delete [] cmd_buffer;
}

char * srCmdLine::read_cmd()
{
	buffer_use = 0;
	return( cmd_buffer );
}

int srCmdLine::pending_cmd(void)
{
#ifdef WIN32_LEAN_AND_MEAN	// what's this stands for
	while( _kbhit() )	{
		char c = _getch();
		if (c == '\x09') // tab - use auto completion for commands
		{
			mcuCBHandle& mcu = mcuCBHandle::singleton();
			srHashMapBase* map = NULL;
			// get the current partial command
			std::string partialCommand(cmd_buffer, buffer_use);
			std::string commandPrefix = "";
			// only use tab completion on the first word
			size_t index = partialCommand.find_first_of(" ");
			if (index != std::string::npos)
			{
				// if the command matches 'set', 'print' or 'test' use those maps
				std::string firstToken = partialCommand.substr(0, index);
				if (firstToken == "set")
				{
					map = &mcu.set_cmd_map.getHashMap();
					partialCommand = partialCommand.substr(index + 1);
					commandPrefix = "set ";
				}
				else if (firstToken == "print")
				{
					map = &mcu.print_cmd_map.getHashMap();
					partialCommand = partialCommand.substr(index + 1);
					commandPrefix = "print ";
				}
				else if (firstToken == "test")
				{
					map = &mcu.test_cmd_map.getHashMap();
					partialCommand = partialCommand.substr(index + 1);
					commandPrefix = "test ";
				}
				else
				{
					// transform tabs into a space
					std::cout << " ";
					cmd_buffer[ buffer_use++ ] = ' ';
					continue;
				}
				
			}
			
			// find a match against the current list of commands
			if (!map)
				map = &mcu.cmd_map.getHashMap();
			int numEntries = map->get_num_entries();
			map->reset();
			int numMatches = 0;
			char* key = NULL;
			int numChecked = 0;
			map->next(&key);
			std::vector<std::string> options;
			while (key)
			{
				bool match = false;
				std::string keyString = key;
				numChecked++;
				if (partialCommand.size() <= keyString.size())
				{
					match = true;
					for (size_t a = 0; a < partialCommand.size() && a < keyString.size(); a++)
					{
						if (partialCommand[a] != keyString[a])
						{
							match = false;
							break;
						}
					}
					if (match)
					{
						options.push_back(keyString);
						numMatches++;
					}
				}
				map->next(&key);
				std::string nextKey = key;
				if (nextKey == keyString)
					break; // shouldn't map.next(key) make key == NULL? This doesn't seem to happen.
			} 
			if (numMatches == 1)
			{
				int numChars = buffer_use;
				std::string final = commandPrefix + options[0] + " ";
				strcpy(cmd_buffer, final.c_str());
				buffer_use = final.size();
				for (size_t n = numChars; n <  final.size(); n++)
				{
					std::cout << final[n];
				}
				continue;
			}
			else if (numMatches > 1)
			{ // more than one match, show the options on the line below
				printf("\n");
				// sort the matches
				std::sort(options.begin(), options.end());
				for (size_t x = 0; x < options.size(); x++)
				{
					std::cout << options[x] << " ";
				}
				printf("\n> ");
				for (int x = 0; x < buffer_use; x++)
					std::cout << cmd_buffer[x];

				continue;
			}
			else if (numMatches == 0)
			{
				// transform tabs into a space
				std::cout << " ";
				cmd_buffer[ buffer_use++ ] = ' ';
				continue;
			}
		}
		else if (c == '\x0' || c == '\xE0')
		{
			char c_next = _getch();
			if (c_next == '\x48') {
				// uparrow
				for(int i = 0 ; i < buffer_use; i++)
					fprintf(stdout, "\b \b" );
				if( !cmds->empty() )
				{
					if( iter != cmds->begin() )	iter--;
					std::cout<< (*iter).c_str();
					memcpy( cmd_buffer, (*iter).c_str(), (*iter).size());
					buffer_use = (*iter).size();
				}
			}
			if(c_next == '\x50'){
				for(int i = 0 ; i < buffer_use; i++)
					fprintf(stdout, "\b \b" );
				if( iter != cmds->end() )
				{
					iter ++;
					if( iter != cmds->end() )
					{
						std::cout<< (*iter).c_str();
						memcpy( cmd_buffer, (*iter).c_str(), (*iter).size());
						buffer_use = (*iter).size();
					}
					else
					{
						memset( cmd_buffer, 0, buffer_use);
						buffer_use = 0;
					}
				}
			}
			continue;
		}
		std::cout << c; 
		if( 
			( c == '\r' )||
			( c == '\n' )||
			( c == '\0' ) )	{
			if( buffer_use!= 0 )
			{
				std::stringstream stream;
				for(int i = 0 ; i < buffer_use; i++)
					stream << cmd_buffer[i];
				if( cmds->size() >= max_cmdlines )
					cmds->pop_front();
				cmds->push_back(stream.str());
			}
			iter = cmds->end();
			printf("\n" );
			cmd_buffer[ buffer_use++ ] = '\0';
			return( TRUE );
		}
		if( c == '\b' )	{
			printf(" " );
			if( buffer_use > 0 )	{
				printf("\b" );
				buffer_use--;
			}
			return( FALSE );
		}
		cmd_buffer[ buffer_use++ ] = c;
		if( buffer_use == buffer_len )	{
			int err = realloc_buffer( 2 * buffer_len );
		}
	}
	return( FALSE );
#else
	struct timeval tp; 
	fd_set fds;
	tp.tv_sec = 0; 
	tp.tv_usec = 0;
	FD_ZERO( &fds );
	FD_SET( KR_STDIN_FD, &fds );
	
	if( select( 1, &fds, NULL, NULL, &tp ) )	{
		while( 1 )	{
			char c = (char)fgetc( stdin );
			if( 
				( c == '\r' )||
				( c == '\n' )||
				( c == '\0' ) )	
				{
				cmd_buffer[ buffer_use++ ] = '\0';
				return( TRUE );
			}
			cmd_buffer[ buffer_use++ ] = c;
			if( buffer_use == buffer_len )	{
				int err = realloc_buffer( 2 * buffer_len );
			}
		}
	}
	return( FALSE );
#endif
}

int srCmdLine::realloc_buffer(int len)
{
	char *new_buff = new char[ len ];
	if( new_buff == NULL )	{
		return( CMD_FAILURE );
	}
	if( len < ( buffer_use + 1 ) )	{
		buffer_use = len - 1;
	}
	if( cmd_buffer )	{
		memcpy( new_buff, cmd_buffer, buffer_use );
		delete [] cmd_buffer;
	}
	else	{
		new_buff[ 0 ] = '\0';
	}
	cmd_buffer = new_buff;
	buffer_len = len;
	return( CMD_SUCCESS );
}

