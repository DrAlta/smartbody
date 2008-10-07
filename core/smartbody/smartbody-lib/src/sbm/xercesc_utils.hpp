/*
 *  xercesc_utils.hpp - part of SmartBody-lib
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
 *      Ashok Basawapatna, USC (no longer)
 */

#ifndef XERCESC_UTILS_HPP
#define XERCESC_UTILS_HPP

#include <string>


#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/sax/HandlerBase.hpp>

#include <xercesc/sax/SAXException.hpp>

#include <boost/shared_ptr.hpp>



XERCES_CPP_NAMESPACE_USE


namespace xml_utils {

	//  Get first child DOMElement
	DOMElement* getFirstChildElement( const DOMNode* node );

	//  Get next sibling DOMElement
	DOMElement* getNextElement( const DOMNode* node );

	//  parse a c-string as XML
	int parseCString( const char* data, AbstractDOMParser* parser );

	//  Convert XMLCh string to ascii c-string.
	//  chars > 127 are represented as 127 (an unused character in both ascii and Unicode)
	const char* asciiString( const XMLCh* wstr );

	//  Read XMLCh string as a float
	//  assumes the presence of wcstod(..).
	//  resets, but does not check errno for errors
	const float xcstof( const XMLCh* str );

	//  Read XMLCh string as a double
	//  assumes the presence of wcstod(..).
	//  resets, but does not check errno for errors
	const double xcstod( const XMLCh* str );

	//  Convert Text and Elements of DOMNode (and children) into a 8bit-per-char string
	void xmlToString( const DOMNode* node, std::string& converted );

    //  Takes in a C string (nth element from srArgBuffer, most likely).
    //  If is begins with <, assumes it is an XML string and parses it
    //  (returning a DOMDocument), otherwise assumes it is a filename 
    //  and parses the file contents as XML (again, returning the DOMDocument).
    //
    //  Returns NULL if there was an error during parsing.
	DOMDocument* parseMessageXml( XercesDOMParser* xmlParser, const char *str );

	//  STL Comparason operator for XMLCh* using XMLString::compareString(..)
	class XMLStringCmp {
	public:
		bool operator() (const XMLCh*, const XMLCh*) const; 
	};
};

#endif  //  XERCESC_UTILS_HPP
