#include "sr_cmd_line.h"
#include <sstream>
#include <vhcl_log.h>

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
		if (c == '\x0' || c == '\xE0')
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

