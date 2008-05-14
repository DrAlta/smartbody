/*
 *  sr_cmd_seq.h - part of SmartBody-lib
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

#ifndef SR_COMMAND_SEQUENCE_H
#define SR_COMMAND_SEQUENCE_H

#include <stdio.h>
#include "sbm_constants.h"

//////////////////////////////////////////////////////////////////////////////////

class srCmdSeq	{

	typedef struct sr_command_event_s  {
		
		float			time;
		char			*cmd;

		sr_command_event_s	*next;
		
	} sr_command_event_t;

	public:
		srCmdSeq(void);
		virtual ~srCmdSeq(void);
		
		void	print( FILE *fp = stdout );						/* print events */
		int		get_count(void)	{ return( event_count ); }
		
		int		write_file( char *seq_file );
		int		read_file( FILE *seq_fp );
		int		read_file( char *seq_file, int report_open_fail = TRUE );

		int		insert( float time, const char *cmd_ref );			/* copy (with new[]), sort by time */
		void	offset( float t ) { event_offset = t; }			/* set pop by event.time + offset */
		char	*pop( float time );								/* remove first event passed */

		void	reset(void) { iterator = handle; }
		char	*next( float *t = NULL );	// return pointer, time
		char	*pull( float *t = NULL );	// return instance, time

	protected:
		int		insert( sr_command_event_t *event );			/* sort by event.time */
		sr_command_event_t *remove( void );
		
	private:
		int		event_count;
		float	event_offset;
		sr_command_event_t *handle;
		sr_command_event_t *iterator;
};

//////////////////////////////////////////////////////////////////////////////////
#endif
