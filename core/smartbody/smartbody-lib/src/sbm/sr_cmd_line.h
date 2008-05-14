/*
 *  sr_cmd_line.h - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcus Thiebaux, USC
 *      Andrew n marshall, USC
 *      Ashok Basawapatna, USC (no longer)
 */

#ifndef SR_CMD_LINE_H
#define SR_CMD_LINE_H

#include	<stdio.h>
#include    <stdlib.h>
#include	<string.h>
#include	<sys/types.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include	<conio.h>
#else
#include	<unistd.h>
#include	<sys/time.h>
#define		KR_STDIN_FD		0
#endif

#include "sbm_constants.h"

////////////////////////////////////////////////////////////////////

class srCmdLine	{
	
	public:
		srCmdLine( int len = 256 )	{
			buffer_len = 0;
			buffer_use = 0;
			cmd_buffer = 0x0;
			int err = realloc_buffer( len );
		}
		virtual ~srCmdLine( void )	{
			if( cmd_buffer )	{
				delete [] cmd_buffer;
			}
		}
		
		int pending_cmd( void )	{
#ifdef WIN32_LEAN_AND_MEAN
			while( _kbhit() )	{
				char c = _getche();
				if( 
					( c == '\r' )||
					( c == '\n' )||
					( c == '\0' ) )	
					{
					fprintf( stdout, "\n" );
					cmd_buffer[ buffer_use++ ] = '\0';
					return( TRUE );
				}
				if( c == '\b' )	{
					fprintf( stdout, " " );
					if( buffer_use > 0 )	{
						fprintf( stdout, "\b" );
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
		
		char *read_cmd( void )	{
			buffer_use = 0;
			return( cmd_buffer );
		}
	
	private:
		int realloc_buffer( int len )	{
			
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
		
		char *cmd_buffer;
		int buffer_len;
		int buffer_use;
};

////////////////////////////////////////////////////////////////////
#endif
