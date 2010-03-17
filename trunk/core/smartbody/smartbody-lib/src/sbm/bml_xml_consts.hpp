/*
 *  bml_xml.hpp - part of SmartBody-lib
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
 *      Andrew n marshall, USC
 *      Corne Versloot, while at USC
 *      Ed Fast, USC
 */

#ifndef BML_XML_HPP
#define BML_XML_HPP

#include <xercesc/util/XMLString.hpp>
XERCES_CPP_NAMESPACE_USE


namespace BML {
	//  Common XML Identifiers
	const XMLCh ATTR_ID[]    = L"id";
	const XMLCh ATTR_TYPE[]  = L"type";
	const XMLCh ATTR_NAME[]  = L"name";
	const XMLCh ATTR_LEVEL[] = L"level";

	const XMLCh ATTR_START[]        = L"start";
	const XMLCh ATTR_READY[]        = L"ready";
	const XMLCh ATTR_STROKE_START[] = L"stroke_start";
	const XMLCh ATTR_STROKE[]       = L"stroke";
	const XMLCh ATTR_STROKE_END[]   = L"stroke_end";
	const XMLCh ATTR_RELAX[]        = L"relax";
	const XMLCh ATTR_END[]          = L"end";

	const XMLCh TM_START[]        = L"start";
	const XMLCh TM_READY[]        = L"ready";
	const XMLCh TM_STROKE_START[] = L"stroke_start";
	const XMLCh TM_STROKE[]       = L"stroke";
	const XMLCh TM_STROKE_END[]   = L"stroke_end";
	const XMLCh TM_RELAX[]        = L"relax";
	const XMLCh TM_END[]          = L"end";
}

#endif  // BML_XML_HPP