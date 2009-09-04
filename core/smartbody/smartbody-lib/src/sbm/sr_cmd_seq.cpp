/*
 *  sr_cmd_seq.cpp - part of SmartBody-lib
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
 */

#include "sr_cmd_seq.h"
#include "sr_arg_buff.h"

#include <string.h>

#define MAX_CMD_ARGL 2048

//////////////////////////////////////////////////////////////////////////////////

srCmdSeq::srCmdSeq( void )	{
		
	event_count = 0;
	event_offset = 0.0;
	handle = new sr_command_event_t;
	handle->next = NULL;
	handle->time = 0.0;
	handle->cmd = NULL;
	iterator = NULL;
}

srCmdSeq::~srCmdSeq( void )	{

	if( event_count > 0 )	{
		//printf( "srCmdSeq::~srCmdSeq NOTE: deleting %d events\n", event_count );
		char *cmd;
		reset();
		while( cmd = pull() )	{
			delete [] cmd;
		}
	}
	delete handle;
}

void srCmdSeq::print( FILE *fp )	{
	sr_command_event_t *event;
	
	event = handle->next;
	while( event != NULL )	{
		fprintf( fp, "%f  %s\n", event->time, event->cmd );
		event = event->next;
	}
}

//////////////////////////////////////////////////////////////////////////////////

int srCmdSeq::write_file( char *seq_file )	{
	FILE *out_fp;

	if( seq_file == NULL )	{
	    fprintf( stdout, "srCmdSeq::write_file ERR: NULL filename\n" ); 
		return( CMD_FAILURE );
	}

	if( ( out_fp = fopen( seq_file, "w" ) ) == NULL ) { 
	    fprintf( stdout, "srCmdSeq::write_file ERR: file '%s' not opened\n", seq_file ); 
		return( CMD_FAILURE );
	}
	
	fprintf( out_fp, "# srCmdSeq::write_file '%s'\n", seq_file );
	fprintf( out_fp, "\n" );
	print( out_fp );
	fclose( out_fp );
	return( CMD_SUCCESS );
}

int srCmdSeq::read_file( FILE *seq_fp )	{

	if( seq_fp == NULL )	{
	    fprintf( stdout, "srCmdSeq::read_file ERR: NULL fp\n" ); 
		return( CMD_FAILURE );
	}
	char fill_buff[ MAX_CMD_ARGL ] = "";
	char line_buff[ MAX_CMD_ARGL ] = "";
	int line_count = 0;
	int done = FALSE;
	while( !done )	{

		if( fscanf( seq_fp, "%[ \t\n]", fill_buff ) == EOF )	{
			done = TRUE;
		}
		else	{
			if( fscanf( seq_fp, "%[^\n]", line_buff ) == EOF )	{
				done = TRUE;
			}
			else	{
				if( line_buff[ 0 ] != '#' )	{
					
					srArgBuffer args( line_buff );
					int token_count = args.calc_num_tokens();
					if( token_count >= 2 )	{
						
						float t = args.read_float();
						char *cmd = args.read_remainder_raw();
						if( insert( t, cmd ) == CMD_FAILURE )	{
							fprintf( stdout, "srCmdSeq::read_file ERR: insert FAILED: line # %d\n", line_count + 1 );
							done = TRUE;
						}
					}
					else	{
						fprintf( stdout, "srCmdSeq::read_file ERR: line # %d\n", line_count + 1 );
					}
				}
			}
		}
		line_count++;
	}
	return( CMD_SUCCESS );
}

int srCmdSeq::read_file( char *seq_file, int report_open_fail )	{
	FILE *in_fp;
	
	if( seq_file == NULL )	{
	    fprintf( stdout, "srCmdSeq::read_file ERR: NULL filename\n" ); 
		return( CMD_FAILURE );
	}
	
	if( ( in_fp = fopen( seq_file, "r" ) ) == NULL ) { 
		if( report_open_fail )	{
		    fprintf( stdout, "srCmdSeq::read_file ERR: file '%s' not found\n", seq_file ); 
		}
		return( CMD_FAILURE );
	}

	int err = read_file( in_fp );
	if( err != CMD_SUCCESS )	{
		return( err );
	}
	fclose( in_fp );
	return( CMD_SUCCESS );
}

//////////////////////////////////////////////////////////////////////////////////

int srCmdSeq::insert( float time, const char *cmd_ref )	{
	sr_command_event_t *event;

	event = new sr_command_event_t;
	event->next = NULL;
	event->time = time;
	event->cmd = new char[ strlen( cmd_ref ) + 1 ];
	sprintf( event->cmd, "%s", cmd_ref );
	return( insert( event ) );
}

int srCmdSeq::insert_ref( float time, char *cmd_ref )	{
	sr_command_event_t *event;

	event = new sr_command_event_t;
	event->next = NULL;
	event->time = time;
	event->cmd = cmd_ref;
	return( insert( event ) );
}

char *srCmdSeq::pop( float time )	{
	sr_command_event_t *event;
	float offset_time = time - event_offset;
	
	event = handle->next;
	if( event != NULL )	{
		if( event->time <= offset_time )	{
			handle->next = event->next;
			char *cmd = event->cmd;
			delete event;
			event_count--;
			return( cmd );
		}
	}
	return( NULL );
}

//////////////////////////////////////////////////////////////////////////////////

float srCmdSeq::duration() {
	float last_time = 0;

	sr_command_event_t *i = handle;
	while( i != NULL ) {
		if( i->time > last_time )
			last_time = i->time;
		i = i->next;
	}

	return last_time;
}

//////////////////////////////////////////////////////////////////////////////////

char *srCmdSeq::next( float *t )	{
		
	if( iterator )	{
		iterator = iterator->next;
		if( iterator )	{
			if( t )	{
				*t = iterator->time;
			}
			return( iterator->cmd );
		}
	}
	return( NULL );
}

char *srCmdSeq::pull( float *t )	{

	sr_command_event_t *event = remove();
	if( event )	{
		char *cmd = event->cmd;
		if( t )	{
			*t = event->time;
		}
		delete event;
		return( cmd );
	}
	return( NULL );
}

//////////////////////////////////////////////////////////////////////////////////

int srCmdSeq::insert( sr_command_event_t *event )	{
	sr_command_event_t *prev, *tmp;
	float time;
		
	if( event == NULL )	{
		return( CMD_FAILURE );
	}
	prev = handle;
	tmp = handle->next;
	time = event->time;
	
	while( tmp != NULL )	{
		if( time < tmp->time )	{ // add after, if same time
			prev->next = event;
			event->next = tmp;
			event_count++;
			return( CMD_SUCCESS );
		}
		prev = tmp;
		tmp = tmp->next;
	}
	prev->next = event;
	event->next = NULL;
	event_count++;
	return( CMD_SUCCESS );
}

srCmdSeq::sr_command_event_t *srCmdSeq::remove( void )	{
	sr_command_event_t *event;

	event = handle->next;
	if( event != NULL )	{
		handle->next = event->next;
		event->next = NULL;
		event_count--;
	}
	return( event );
}

//////////////////////////////////////////////////////////////////////////////////
