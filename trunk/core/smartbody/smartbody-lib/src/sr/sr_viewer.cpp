/*
 *  sr_viewer.cpp - part of SBM: SmartBody Module
 *  Copyright (C) 2008  University of Southern California
 *
 *  SBM is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SBM is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SBM.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently at UC Merced)
 */

#include "sr_viewer.h"

SrViewer::SrViewer( int x, int y, int w, int h, const char *label )
{
}

SrViewer::~SrViewer()
{
}

SrSn* SrViewer::root ()
{
	return NULL;
}

void SrViewer::root ( SrSn *r )
{
}

void SrViewer::view_all()
{
}

void  SrViewer::render()
{
}

void  SrViewer::label_viewer(const char* str)
{
}

void SrViewer::get_camera ( SrCamera &cam )
{
}

void SrViewer::set_camera ( const SrCamera &cam )
{
}

void SrViewer::show_viewer()
{
}

void SrViewer::hide_viewer()
{
}

void SrViewer::set_viewer_mode(int mode)
{
}

SrViewerFactory::SrViewerFactory()
{
}

SrViewer* SrViewerFactory::create(int x, int y, int w, int h)
{
	return new SrViewer(x, y, w, h);
}


//================================ End of File =================================================