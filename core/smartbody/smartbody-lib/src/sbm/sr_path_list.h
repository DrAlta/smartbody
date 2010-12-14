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
#include <sstream>

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

		void setPathPrefix(std::string pre) {
			prefix = pre;
		}
		
		std::string getPathPrefix() {
			return prefix;
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
		
		std::string next_path( void )	{
			if( iterator )	{
				iterator = iterator->next;
				if( iterator )	{
					std::stringstream strstr;
					// if the path is an absolute path, don't prepend the media path
					SrString p = iterator->path;
					std::string mediaPath = getPathPrefix();
					if (mediaPath.size() > 0 && !p.has_absolute_path())
						strstr << getPathPrefix() << "\\";
					strstr << iterator->path;

					SrString candidatePath;
					candidatePath.make_valid_path(strstr.str().c_str());
					std::string finalPath = candidatePath;
					// remove the final '/'
					finalPath = finalPath.substr(0, finalPath.size() - 1);
					
					return( finalPath );
				}
			}
			return( "" );
		}

		std::string next_filename( 
			char *buffer, 
			const char *name, 
			char **path_pp = NULL
		)	{
			std::string path = next_path();
			if( path.size() > 0 )	{
				std::stringstream strstr;
				strstr << path << "/" << name;
				if( path_pp )	{
					//*path_pp = path;
					// what to do here?
				}
				return( strstr.str() );
			}
			return( "" );
		}

	private:
		int		link_count;
		sr_path_link_t *head;
		sr_path_link_t *tail;
		sr_path_link_t *iterator;
		std::string prefix;
};

//////////////////////////////////////////////////////////////////////////////////
#endif
