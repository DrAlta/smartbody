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
#include	<iostream>
#include	<list>

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
		srCmdLine( int len = 256 );
		virtual ~srCmdLine( void );
		int pending_cmd( void );	
		char *read_cmd( void );
	
	private:
		int realloc_buffer( int len );
		char *cmd_buffer;
		int buffer_len;
		int buffer_use;
	
		// storing the cmd lines
		unsigned int max_cmdlines;
		std::list<std::string>					*cmds;
		std::list<std::string>::iterator		iter;
};

////////////////////////////////////////////////////////////////////
#endif
