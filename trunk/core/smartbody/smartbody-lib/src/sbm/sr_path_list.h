/*
 *   - part of SmartBody-lib
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
 *      Ed Fast, USC
 */

#ifndef SR_PATH_LIST_H
#define SR_PATH_LIST_H

#include <stdio.h>
#include <string.h>

#include "sbm_constants.h"

//////////////////////////////////////////////////////////////////////////////////

class srPathList	{

	typedef struct sr_path_link_s  {
		
		char			*path;
		sr_path_link_s	*next;
		
	} sr_path_link_t;

	public:
		srPathList(void)	{
			link_count = 0;
			head = new sr_path_link_t;
			head->next = NULL;
			head->path = NULL;
			tail = head;
		}
		virtual ~srPathList(void)	{
			iterator = head;
			while( iterator )	{
				delete [] iterator->path;
				sr_path_link_t * iterator_copy = iterator;
				iterator = iterator->next;
				delete iterator_copy;
			}
			//delete head;
		}
		
		int insert( char *path )	{
			if( path )	{
				tail->next = new sr_path_link_t;
				tail = tail->next;
				tail->next = NULL;
				tail->path = new char[ strlen( path ) + 1 ];
				sprintf( tail->path, "%s", path );
				return( CMD_SUCCESS );	
			}
			return( CMD_FAILURE );	
		}
		
		void reset(void) {	
			iterator = head; 
		}
		
		char *next_path( void )	{
			if( iterator )	{
				iterator = iterator->next;
				if( iterator )	{
					return( iterator->path );
				}
			}
			return( NULL );
		}

		char *next_filename( 
			char *buffer, 
			const char *name, 
			char **path_pp = NULL
		)	{
			char *path = next_path();
			if( path )	{
				sprintf( buffer, "%s/%s", path, name );
				if( path_pp )	{
					*path_pp = path;
				}
				return( buffer );
			}
			return( NULL );
		}

	private:
		int		link_count;
		sr_path_link_t *head;
		sr_path_link_t *tail;
		sr_path_link_t *iterator;
};

//////////////////////////////////////////////////////////////////////////////////
#endif
