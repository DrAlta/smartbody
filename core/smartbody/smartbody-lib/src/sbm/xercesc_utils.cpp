/*
 *  xercesc_utils.cpp - part of SmartBody-lib
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
 */

#include "vhcl.h"
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "xercesc_utils.hpp"

#include <xercesc/framework/MemBufInputSource.hpp>


#define USELOG (0)

using namespace std;

////////////////////////////////////////////////////////////////////////////////////

string xml_utils::xml_w2s( wstring w ) {

	string s( w.length(), 0 );
	s.assign( w.begin(), w.end() );
	return( s );
}

wstring xml_utils::xml_s2w( string s ) {

	wstring w( s.length(), 0 );
	copy( s.begin(), s.end(), w.begin() );
	return( w );
}

////////////////////////////////////////////////////////////////////////////////////

bool xml_utils::xml_translate( string *str_p, const XMLCh* xml_str )	{

	if( str_p == NULL ) return( false );
	if( xml_str == NULL ) return( false );

	char *tmp_cp = XMLString::transcode( xml_str );
	*str_p = string( tmp_cp );
	XMLString::release( &tmp_cp );
	return( true );
}

string xml_utils::xml_translate_string( const XMLCh* xml_str, string dfl )	{

	string tmp; if( xml_translate( &tmp, xml_str ) ) return( tmp );  return( dfl );
}

wstring xml_utils::xml_translate_wide( const XMLCh* xml_str, string dfl)	{

	return( xml_s2w( xml_translate_string( xml_str, dfl ) ) );
}

double xml_utils::xml_translate_double( const XMLCh* xml_str, double dfl )	{

	string tmp; if( xml_translate( &tmp, xml_str ) ) return( atof( tmp.c_str() ) );
	return( dfl );
}

float xml_utils::xml_translate_float( const XMLCh* xml_str, float dfl)	{

	return( (float)xml_translate_double( xml_str, (double)dfl ) );
}

int xml_utils::xml_translate_int( const XMLCh* xml_str, int dfl)	{

	return( (int)xml_translate_double( xml_str, (double)dfl ) );
}

////////////////////////////////////////////////////////////////////////////////////

void xml_utils::xml_parse_error( const XMLCh* attr, DOMElement* elem )	{

	string att = xml_translate_string( attr );
	string val = xml_translate_string( elem->getAttribute( attr ) );
	string tag = xml_translate_string( elem->getTagName() );
	LOG( "XML-ERR: att:'%s'; val:\"%s\"; tag:<%s .../>", att.c_str(), val.c_str(), tag.c_str() );
}

////////////////////////////////////////////////////////////////////////////////////

bool xml_utils::xml_parse_string( 
	string *str_p, 
	const XMLCh* attr, 
	DOMElement* elem,
	bool verbose
)	{

	if( str_p == NULL ) {
		LOG( "xml_parse_string ERR: bad str_p" );
		return( false );
	}
	if( attr == NULL ) {
		LOG( "xml_parse_string ERR: bad XMLCh attribute ptr" );
		return( false );
	}
	if( elem == NULL ) {
		LOG( "xml_parse_string ERR: bad DOMElement ptr" );
		return( false );
	}

	const XMLCh* value = elem->getAttribute( attr );
	if( value == NULL ) {
		LOG( "xml_parse_string ERR: bad value" );
		return( false );
	}
	if( value[ 0 ] == chNull ) {
		if( verbose ) xml_parse_error( attr, elem );
		return( false );
	}
	xml_translate( str_p, value );
	return( true );
}

bool xml_utils::xml_parse_double( 
	double *d_p,
	const XMLCh* attr, 
	DOMElement* elem,
	bool verbose
 )	{

	if( d_p == NULL )	{
		LOG( "xml_parse_double ERR: bad d_p" );
		return( false );
	}

	string str;
	if( xml_parse_string( &str, attr, elem, verbose ) )	{
		*d_p = atof( str.c_str() );
		return( true );
	}
	return( false );
}

bool xml_utils::xml_parse_float( 
	float *f_p,
	const XMLCh* attr, 
	DOMElement* elem,
	bool verbose
 )	{

	if( f_p == NULL )	{
		LOG( "xml_parse_double ERR: bad f_p" );
		return( false );
	}

	double d;
	if( xml_parse_double( &d, attr, elem, verbose ) )	{
		*f_p = (float)d;
		return( true );
	}
	return( false );
}

bool xml_utils::xml_parse_int( 
	int *i_p,
	const XMLCh* attr, 
	DOMElement* elem,
	bool verbose
 )	{

	if( i_p == NULL )	{
		LOG( "xml_parse_double ERR: bad i_p" );
		return( false );
	}

	double d;
	if( xml_parse_double( &d, attr, elem, verbose ) )	{
		*i_p = (int)d;
		return( true );
	}
	return( false );
}

////////////////////////////////////////////////////////////////////////////////////

string 
xml_utils::xml_parse_string( const XMLCh* A, DOMElement* E, string dfl, bool v )	{

	string tmp; if( xml_parse_string( &tmp, A, E, v ) ) return( tmp ); return( dfl );
}

double 
xml_utils::xml_parse_double( const XMLCh* A, DOMElement* E, double dfl, bool v )	{

	double tmp; if( xml_parse_double( &tmp, A, E, v ) ) return( tmp ); return( dfl );
}

float 
xml_utils::xml_parse_float( const XMLCh* A, DOMElement* E, float dfl, bool v )	{

	float tmp; if( xml_parse_float( &tmp, A, E, v ) ) return( tmp ); return( dfl );
}

int 
xml_utils::xml_parse_int( const XMLCh* A, DOMElement* E, int dfl, bool v )	{

	int tmp; if( xml_parse_int( &tmp, A, E, v ) ) return( tmp ); return( dfl );
}
	
	// end of recent additions... Apr 2011
////////////////////////////////////////////////////////////////////////////////////

bool xml_utils::XMLStringCmp::operator() (const XMLCh* x, const XMLCh* y ) const {
	int result = XMLString::compareString( x, y );
	return result<0;
}

DOMElement* xml_utils::getFirstChildElement( const DOMNode* node ) {
    node=node->getFirstChild();

    if( node!=NULL &&
		node->getNodeType()!=DOMNode::ELEMENT_NODE ) {
        node=getNextElement( node );
    }

    return (DOMElement*)node;
}

DOMElement* xml_utils::getNextElement( const DOMNode* node ) {
    node=node->getNextSibling();

    while( node!=NULL &&
		   node->getNodeType()!=DOMNode::ELEMENT_NODE ) {
        node=node->getNextSibling();
    }

    return (DOMElement*)node;
}


/**
 *   Parse a C String (char[]) as a XML Input Source.
 *   Check the parser for the actual DOMDocument.
 *
 *   returns the error count in the parser.
 *
 *   Potentially throws OutOfMemoryException, XMLException
 */
int xml_utils::parseCString( const char *str, AbstractDOMParser* parser ) {
	//if( str[0]=='<' && str[1]=='?' && str[2]=='x' && str[3]=='m' && str[4]=='l' ) {
	//	//  Xerces doesn't seem to like the XML prolog in C strings
	//	char *orig = str;
	//	str = &str[5];
	//	while( str[0] != '>' && str[0] != 0 )
	//		str++;
	//	if( str[0] == 0 )
	//      str=orig;  // revert
	//	else
	//		str++;
	//}

	const MemBufInputSource inputSource( (const XMLByte*)str, (unsigned int)strlen(str), "XMLBuffer" );
	parser->parse( inputSource );

	return parser->getErrorCount();
}

const char* xml_utils::asciiString( const XMLCh* wstr ) {
	char* str = new char[ XMLString::stringLen(wstr)+1 ];
	int i = 0;
    XMLCh c = wstr[0];
	while( c!=0 ) {
		if( c<128 )
			str[i] = (unsigned char)c;
		else
			str[i] = 127; // unrepresented character
		c = wstr[++i];
	}
	str[i] = 0;

	return str;
}

const float xml_utils::xcstof( const XMLCh* value ) {

	char *cp = XMLString::transcode( value );
	float f = (float)atof( cp );
	XMLString::release( &cp );
	return( f );
#if 0
	int len = XMLString::stringLen(str);
	const XMLCh* endp = str+len;  // \0 terminator 
	errno = 0;
	return (float) wcstod( str, (wchar_t**)&endp );
#endif
}

const double xml_utils::xcstod( const XMLCh* value ) {

	char *cp = XMLString::transcode( value );
	double d = atof( cp );
	XMLString::release( &cp );
	return( d );
#if 0
	int len = XMLString::stringLen(str);
	const XMLCh* endp = str+len;  // \0 terminator 
	errno = 0;
	return wcstod( str, (wchar_t**)&endp );
#endif
}

void xml_utils::xmlToString( const DOMNode* node, string& converted ){ //recursively called fcn that traverses DOM tree
	//This function is only meant to handle Text and Element Node types (Attributes are part of Elements)
	switch( node->getNodeType() ) {
		case DOMNode::TEXT_NODE:
		{
			converted += XMLString::transcode(node->getTextContent());
			break;
		}

		case DOMNode::ELEMENT_NODE:
		{
			DOMElement *element= (DOMElement *)node; //instantiate an element using this node
			string tag= XMLString::transcode(element->getTagName()); //find the element tag
			DOMNamedNodeMap* attributes= element->getAttributes(); //gets all attributes and places them in a Node map

			converted += "<"; 
			converted += tag; 
			for( unsigned int i=0; i< (attributes->getLength()); i++ ) { //iterates through and includes all attributes
				converted += " ";
				converted += XMLString::transcode(attributes->item(i)->getNodeName());
				converted += "=\"";
				converted += XMLString::transcode(attributes->item(i)->getNodeValue());
				converted += "\"";
			}
			
			node = node->getFirstChild();
			if( node ) { //check children first
				converted += ">";  // end of start tag

				xmlToString( node, converted );
				node = node->getNextSibling();
				while( node ) {
					xmlToString( node, converted );
					node = node->getNextSibling();
				}

				// end tag
				converted += "</";
				converted += tag;
				converted += ">";
			} else {
				converted += " />";  // end of empty element
			}
			break;
		}

		//default:	// ignore other node types
	}
}


DOMDocument* xml_utils::parseMessageXml( XercesDOMParser* xmlParser, const char *str ) {
	try {
		// xml in a file?
		if( str[0]=='<' ) {
			if (USELOG) LOG("Parsing inline XML.");
			int numErrors = xml_utils::parseCString( str, xmlParser );
			if (numErrors > 0)
			{
				LOG("Found %d errors when parsing %s.", numErrors, str);
			}
		} else {
			if (USELOG) LOG("Parsing XML from file \"%s\"", str);
			xmlParser->parse( str );
		}
		int errorCount = xmlParser->getErrorCount();
		if( errorCount > 0 ) {
			//char* message = XMLString::transcode(e.getMessage());
			stringstream strstr;
			strstr << "xml_utils::parseMessageXml(): "<<errorCount<<" errors while parsing xml: ";
			LOG(strstr.str().c_str());
			// TODO: print errors
			return NULL;
		}
		return xmlParser->getDocument();
	} catch( const XMLException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		std::stringstream strstr;
		strstr << "xml_utils::parseMessageXml(): XMLException while parsing xml: "<<message;
		LOG(strstr.str().c_str());
		return NULL;
	} catch( const SAXParseException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		std::stringstream strstr;
		strstr << "xml_utils::parseMessageXml(): SAXException while parsing xml: "<<message
				<< " (line "<<e.getLineNumber()<<", col "<<e.getColumnNumber()<<")";
		LOG(strstr.str().c_str());
		return NULL;
	} catch( const SAXException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		std::stringstream strstr;
		strstr << "xml_utils::parseMessageXml(): SAXException while parsing xml: "<<message;
		LOG(strstr.str().c_str());
		return NULL;
	} catch( const DOMException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		std::stringstream strstr;
		strstr << "xml_utils::parseMessageXml(): DOMException while parsing xml: "<<message;
		LOG(strstr.str().c_str());
		return NULL;
	}
}
		

std::string convertWStringToString(std::wstring w)
{
	std::string str(w.begin(), w.end());
	return str;
}
