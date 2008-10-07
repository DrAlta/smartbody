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

#include <stdlib.h>
#include <iostream>


#include "xercesc_utils.hpp"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4DOMInputSource.hpp>


#define LOG (0)

using namespace std;



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
			str[i] = 127;
		c = wstr[++i];
	}
	str[i] = 0;

	return str;
}

const float xml_utils::xcstof( const XMLCh* str ) {
	int len = XMLString::stringLen(str);
	const XMLCh* endp = str+len;  // \0 terminator 
	errno = 0;
	return (float) wcstod( str, (wchar_t**)&endp );
}

const double xml_utils::xcstod( const XMLCh* str ) {
	int len = XMLString::stringLen(str);
	const XMLCh* endp = str+len;  // \0 terminator 
	errno = 0;
	return wcstod( str, (wchar_t**)&endp );
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
			if(LOG) cout<<"Parsing inline XML."<<endl;
			xml_utils::parseCString( str, xmlParser );
		} else {
			if(LOG) cout<<"Parsing XML from file \""<<str<<"\""<<endl;
			xmlParser->parse( str );
		}
		int errorCount = xmlParser->getErrorCount();
		if( errorCount > 0 ) {
			//char* message = XMLString::transcode(e.getMessage());
			cerr << "xml_utils::parseMessageXml(): "<<errorCount<<" errors while parsing xml: "<< endl;
			// TODO: print errors
			return NULL;
		}
		return xmlParser->getDocument();
	} catch( const XMLException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		cerr << "xml_utils::parseMessageXml(): XMLException while parsing xml: "<<message<< endl;
		return NULL;
	} catch( const SAXParseException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		cerr << "xml_utils::parseMessageXml(): SAXException while parsing xml: "<<message
				<< " (line "<<e.getLineNumber()<<", col "<<e.getColumnNumber()<<")"<< endl;
		return NULL;
	} catch( const SAXException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		cerr << "xml_utils::parseMessageXml(): SAXException while parsing xml: "<<message<< endl;
		return NULL;
	} catch( const DOMException& e ) {
		char* message = XMLString::transcode(e.getMessage());
		cerr << "xml_utils::parseMessageXml(): DOMException while parsing xml: "<<message<< endl;
		return NULL;
	}
}
