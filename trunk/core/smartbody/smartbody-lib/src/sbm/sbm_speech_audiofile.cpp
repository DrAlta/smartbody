/*
 *  sbm_speech_audiofile.cpp - part of SmartBody-lib
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

#include "vhcl.h"
#include "sbm_speech_audiofile.hpp"
#include "mcontrol_util.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <fstream>

using std::string;
using std::vector;
using stdext::hash_map;
using namespace SmartBody;


AudioFileSpeech::AudioFileSpeech()
{
   m_xmlParser = new XercesDOMParser();
   m_xmlHandler = new HandlerBase();
   m_xmlParser->setErrorHandler( m_xmlHandler );

   m_requestIdCounter = 1;  // start with 1, in case 0 is a special case
}


AudioFileSpeech::~AudioFileSpeech()
{
   delete m_xmlParser;  m_xmlParser = NULL;
   delete m_xmlHandler;  m_xmlHandler = NULL;
}


RequestId AudioFileSpeech::requestSpeechAudio( const char * agentName, const std::string voiceCode, const DOMNode * node, const char * callbackCmd )
{
   // TODO: Test this function with a variety of XML documents

   // TODO: transcode() leaks here
   string encoding = XMLString::transcode( node->getOwnerDocument()->getXmlEncoding() );  // XMLStringconverts XML to cString; encoding
   string version = XMLString::transcode( node->getOwnerDocument()->getXmlVersion () ); //the xml version number

   encoding = "UTF-8";  // getXmlEncoding() doesn't work on  'test bml char doctor <speech ref="hey"/>'

   string xmlConverted = "<?xml version=\"" + version.substr( 0, 6 )+ "\" ";
   xmlConverted += "encoding=\"" + encoding.substr( 0, 7 ) + "\"?>";

   xml_utils::xmlToString( node, xmlConverted ); //Xml to string recursively searches DOM tree and returns a string of the xml document

   return requestSpeechAudio( agentName, voiceCode, xmlConverted, callbackCmd );
}

/*
RequestId AudioFileSpeech::requestSpeechAudio( const char * agentName, std::string voiceCode, std::string text, const char * callbackCmd )
{
	
    mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.mark("requestSpeechAudio", 4, "begin");
	rapidxml::xml_document<> doc;
	std::vector<char> xml(text.begin(), text.end());
    xml.push_back('\0');

	std::string ref = "";
	try {
		doc.parse<0>(&xml[0]);

		std::string name = doc.name();
		rapidxml::xml_node<>* node = doc.first_node(); 

		rapidxml::xml_attribute<>* refAttr = node->first_attribute("ref");
		
		if (refAttr)
			ref = refAttr->value();

		rapidxml::xml_attribute<>* speechIdAttr = node->first_attribute("id");
		std::string speechId = "";
		if (speechIdAttr)
			speechId = speechIdAttr->value();

		m_speechRequestInfo[ m_requestIdCounter ].id = speechId;
	} catch (...) {
		LOG("Problem parsing XML speech request.");
		return 0;
	}

   SbmCharacter * agent = mcu.character_map.lookup( agentName );
   if ( agent == NULL )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: insert AudioFile voice code lookup FAILED, msgId=%s\n", agentName ); 
	    mcu.mark("requestSpeechAudio");
      return 0;
   }


   char fullAudioPath[ _MAX_PATH ];
   string relativeAudioPath = mcu.speech_audiofile_base_path + voiceCode;
   if ( _fullpath( fullAudioPath, relativeAudioPath.c_str(), _MAX_PATH ) == NULL )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: _fullpath() returned NULL\n" );
	    mcu.mark("requestSpeechAudio");
      return 0;
   }

   m_speechRequestInfo[ m_requestIdCounter ].audioFilename = (string)fullAudioPath + "\\" + ref + ".wav";


   // TODO: Should we fail if the .bml file isn't present?

   string bmlFilename = mcu.speech_audiofile_base_path + voiceCode + "/" + ref + ".bml";
//////////////////////////////////
   //ReadVisemeDataBML( bmlFilename.c_str(), m_speechRequestInfo[ m_requestIdCounter ].visemeData );
   m_speechRequestInfo[ m_requestIdCounter ].visemeData.clear();

   mcu.mark("requestSpeechAudio", 4, "lips");
   rapidxml::file<char> bmlFile(bmlFilename.c_str());
   mcu.mark("requestSpeechAudio", 4, "fileconstruction");
   rapidxml::xml_document<> bmldoc;
   mcu.mark("requestSpeechAudio", 4, "parse");
   bmldoc.parse< rapidxml::parse_declaration_node>(bmlFile.data());

   mcu.mark("requestSpeechAudio", 4, "traverse");

   bool useCurveMode = visemeCurveMode;
   if (useCurveMode)
   {
	   rapidxml::xml_node<>* bmlnode = bmldoc.first_node("bml");
	   if (!bmlnode)
	   {
		   LOG( "Could not find <bml> tag in %s.", bmlFilename.c_str());
	   }
	   else
	   {
		   rapidxml::xml_node<>* curvesnode = bmlnode->first_node("curves");
		   if (!curvesnode)
		   {
			   LOG( "Could not find <curves> tag in %s.", bmlFilename.c_str());
			   // revert to normal viseme mode if no curves are found
			   useCurveMode = false;
		   }
		   else
		   {
			   rapidxml::xml_node<>* node = curvesnode->first_node("curve");
			   while (node)
			   {
				  rapidxml::xml_attribute<>* nameAttr = node->first_attribute("name");
				  string name = "";
				  if (nameAttr)
					  name = nameAttr->value();

				  rapidxml::xml_attribute<>* numKeysAttr = node->first_attribute("num_keys");
				  string numKeys = "";
				  if (numKeysAttr)
					  numKeys = numKeysAttr->value();

				  rapidxml::xml_node<>* keyData = node->first_node();
				  if (keyData)
				  {
					  std::string curveInfo = keyData->value();
					  m_speechRequestInfo[ m_requestIdCounter ].visemeData.push_back(VisemeData(name.c_str(), atoi(numKeys.c_str()), curveInfo.c_str()));

				  }

				   node = node->next_sibling();
			   }
			   // revert to normal viseme mode if no curves are found
			   if (!node)
				useCurveMode = false;
		   }
	   }
   }

   if (!useCurveMode)
   {	      
	   rapidxml::xml_node<>* bmlnode = bmldoc.first_node("bml");
	   if (!bmlnode)
	   {
		   LOG( "Could not find <bml> tag in %s.", bmlFilename.c_str());
	   }

	   rapidxml::xml_node<>* node = bmlnode->first_node("lips");
	   while (node)
	   {
		  rapidxml::xml_attribute<>* visemeAttr = node->first_attribute("viseme");
		  string viseme = "";
		  if (visemeAttr)
			  viseme = visemeAttr->value();

		  rapidxml::xml_attribute<>* artiulationAttr = node->first_attribute("articulation");
		  string articulation = "";
		  if (artiulationAttr)
			  articulation = artiulationAttr->value();

		  rapidxml::xml_attribute<>* startAttr = node->first_attribute("start");
		  string start = "";
		  if (startAttr)
			  start = startAttr->value();


		  rapidxml::xml_attribute<>* readyAttr = node->first_attribute("start");
		  string ready = "";
		  if (readyAttr)
			  ready = readyAttr->value();


		  rapidxml::xml_attribute<>* relaxAttr = node->first_attribute("relax");
		  string relax = "";
		  if (relaxAttr)
			  relax = relaxAttr->value();

		  rapidxml::xml_attribute<>* endAttr = node->first_attribute("end");
		  string end = "";
		  if (endAttr)
			  end = endAttr->value();

		  m_speechRequestInfo[ m_requestIdCounter ].visemeData.push_back( VisemeData( viseme.c_str(), (float)atof( articulation.c_str() ), (float)atof( start.c_str() ) ) );
		  m_speechRequestInfo[ m_requestIdCounter ].visemeData.push_back( VisemeData( viseme.c_str(), 0.0f,                                (float)atof( end.c_str() ) ) );

		  node = node->next_sibling("lips");
	   }
   }
   else
   {
   }
//////////////////////////////////

   if ( m_speechRequestInfo[ m_requestIdCounter ].visemeData.size() == 0 )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: could not read visemes from file: %s\n", bmlFilename.c_str() );
      //return 0;
   }

   //ReadSpeechTiming( bmlFilename.c_str(), m_speechRequestInfo[ m_requestIdCounter ].timeMarkers );
//////////////////////////
    m_speechRequestInfo[ m_requestIdCounter ].timeMarkers.clear();


	mcu.mark("requestSpeechAudio", 4, "sync");
	rapidxml::xml_node<>* bmlnode = bmldoc.first_node("bml");
	rapidxml::xml_node<>* speechnode = bmlnode->first_node("speech");
	if (speechnode)
	{
		rapidxml::xml_node<>* textNode = speechnode->first_node("text");
		if (textNode)
		{
			rapidxml::xml_node<>* syncNode = textNode->first_node("sync");
			while (syncNode)
			{
				rapidxml::xml_attribute<>* idAttr = syncNode->first_attribute("id");
				string id = "";
				if (idAttr)
					id = idAttr->value();

				rapidxml::xml_attribute<>* timeAttr = syncNode->first_attribute("time");
				string time = "";
				if (timeAttr)
					time = timeAttr->value();

				 m_speechRequestInfo[ m_requestIdCounter ].timeMarkers[ id ] = (float)atof( time.c_str() );
				 syncNode = syncNode->next_sibling("sync");

			}
		}
	}
	
	
 
   /////////////////////////




   if ( m_speechRequestInfo[ m_requestIdCounter ].timeMarkers.size() == 0 )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: could not read time markers file: %s\n", bmlFilename.c_str() );
      //return 0;
   }


   mcu.execute_later( vhcl::Format( "%s %s %d %s", callbackCmd, agentName, m_requestIdCounter, "SUCCESS" ).c_str() );


     mcu.mark("requestSpeechAudio");
   return m_requestIdCounter++;

}
*/

RequestId AudioFileSpeech::requestSpeechAudio( const char * agentName, std::string voiceCode, std::string text, const char * callbackCmd )
{

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.mark("requestSpeechAudio", 4, "begin");
   // TODO:  Does return 0 signify error code?
   // TODO:  Handle xerces exceptions?


   // parse text to get the name of the audio file
   // parse .bml file to get viseme timings
   // parse .bml file to get mark timings


   DOMDocument * doc = xml_utils::parseMessageXml( m_xmlParser, text.c_str() );

   DOMElement * speech = doc->getDocumentElement();

   // TODO: make sure it's "speech"


   char * xmlRef = XMLString::transcode( speech->getAttribute( L"ref" ) );
   string ref = xmlRef;
   XMLString::release( &xmlRef );

   char * xmlSpeechId = XMLString::transcode( speech->getAttribute( L"id" ) );
   string speechId = xmlSpeechId;
   XMLString::release( &xmlSpeechId );


   m_speechRequestInfo[ m_requestIdCounter ].id = speechId;


   

   SbmCharacter * agent = mcu.character_map.lookup( agentName );
   if ( agent == NULL )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: insert AudioFile voice code lookup FAILED, msgId=%s\n", agentName ); 
	  mcu.mark("requestSpeechAudio");
      return 0;
   }

   char fullAudioPath[ _MAX_PATH ];
   // if an audio path is present, use it
   bool useAudioPaths = true;
   mcu.audio_paths.reset();
   std::string relativeAudioPath = mcu.audio_paths.next_path();
   if (relativeAudioPath.size() == 0 || relativeAudioPath == mcu.media_path)
   {
       relativeAudioPath = mcu.speech_audiofile_base_path + voiceCode;
	   useAudioPaths = false;
	   // add a one-time deprecation message
	   static bool deprecationMessageAudioPath = true;
	   if (deprecationMessageAudioPath)
	   {
		   LOG("No audio path has been specified, so using default: %s.", mcu.speech_audiofile_base_path.c_str());
		   LOG("Instead, use: path audio <audiopath>.");
		   deprecationMessageAudioPath = false;
	   }
   }
   else
   {
	   relativeAudioPath = relativeAudioPath + "/" + voiceCode;
   }

   if ( _fullpath( fullAudioPath, relativeAudioPath.c_str(), _MAX_PATH ) == NULL )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: _fullpath() returned NULL\n" );
	  mcu.mark("requestSpeechAudio");
      return 0;
   }

   m_speechRequestInfo[ m_requestIdCounter ].audioFilename = (string)fullAudioPath + "\\" + ref + ".wav";


   // TODO: Should we fail if the .bml file isn't present?
   string bmlFilename = relativeAudioPath + "/" + ref + ".bml";

    mcu.mark("requestSpeechAudio", 4, "lips");
   ReadVisemeDataBML( bmlFilename.c_str(), m_speechRequestInfo[ m_requestIdCounter ].visemeData, agent );
   if ( m_speechRequestInfo[ m_requestIdCounter ].visemeData.size() == 0 )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: could not read visemes from file: %s\n", bmlFilename.c_str() );
      return 0;
   }

   mcu.mark("requestSpeechAudio", 4, "sync");
   ReadSpeechTiming( bmlFilename.c_str(), m_speechRequestInfo[ m_requestIdCounter ].timeMarkers );
   if ( m_speechRequestInfo[ m_requestIdCounter ].timeMarkers.size() == 0 )
   {
      LOG( "AudioFileSpeech::requestSpeechAudio ERR: could not read time markers file: %s\n", bmlFilename.c_str() );
      return 0;
   }


   mcu.execute_later( vhcl::Format( "%s %s %d %s", callbackCmd, agentName, m_requestIdCounter, "SUCCESS" ).c_str() );


   mcu.mark("requestSpeechAudio");
   return m_requestIdCounter++;
}

vector<VisemeData *> * AudioFileSpeech::getVisemes( RequestId requestId, const SbmCharacter* character )
{
   // TODO: Change the return type data structure, so that I can simply do this:
   //return m_speechRequestInfo[ requestId ].visemeData


   hash_map< RequestId, SpeechRequestInfo >::iterator it = m_speechRequestInfo.find( requestId );
   if ( it != m_speechRequestInfo.end() )
   {
	      vector< VisemeData * > * visemeCopy = new vector< VisemeData * >;
      for ( size_t i = 0; i < it->second.visemeData.size(); i++ )
      {
         VisemeData & v = it->second.visemeData[ i ];
		 if (!v.isCurveMode() && !v.isTrapezoidMode())
			 visemeCopy->push_back( new VisemeData( v.id(), v.weight(), v.time() ) );
		 else if (v.isTrapezoidMode())
			 visemeCopy->push_back( new VisemeData( v.id(), v.weight(), v.time(), v.duration(), v.rampin(), v.rampout() ) );
		 else
			 visemeCopy->push_back( new VisemeData( v.id(), v.getNumKeys(), v.getCurveInfo() ));
      }

      return visemeCopy;
   }

   return NULL;
}


char * AudioFileSpeech::getSpeechPlayCommand( RequestId requestId, const SbmCharacter * character )
{
	if (!character)
		return NULL;
   // TODO: Wrap up this SASO/PlaySound specific audio command string generation
   // into a class that can abstracted and shared between speech implementations.
   // The SpeechInterface should only need to provide the audio filename.

   // TODO: fix return type

   hash_map< RequestId, SpeechRequestInfo >::iterator it = m_speechRequestInfo.find( requestId );
   if ( it != m_speechRequestInfo.end() )
   {
      it->second.playCommand = vhcl::Format( "send PlaySound \"%s\" %s", it->second.audioFilename.c_str(), character->name );
      return (char *)it->second.playCommand.c_str();
   }

   return NULL;
}


char * AudioFileSpeech::getSpeechStopCommand( RequestId requestId, const SbmCharacter * character )
{
   // TODO: Wrap up this SASO/PlaySound specific audio command string generation
   // into a class that can abstracted and shared between speech implementations.
   // The SpeechInterface should only need to provide the audio filename.

   // TODO: fix return type

   hash_map< RequestId, SpeechRequestInfo >::iterator it = m_speechRequestInfo.find( requestId );
   if ( it != m_speechRequestInfo.end() )
   {
      string characterName;
      if ( character && character->bonebusCharacter )
      {
         characterName = character->bonebusCharacter->m_name;
      }

      it->second.stopCommand = vhcl::Format( "send StopSound \"%s\" %s", it->second.audioFilename.c_str(), characterName.c_str() );
      return (char *)it->second.stopCommand.c_str();
   }

   return NULL;
}


char * AudioFileSpeech::getSpeechAudioFilename( RequestId requestId )
{
   // TODO: fix return type

   hash_map< RequestId, SpeechRequestInfo >::iterator it = m_speechRequestInfo.find( requestId );
   if ( it != m_speechRequestInfo.end() )
   {
      return (char *)it->second.audioFilename.c_str();
   }

   return NULL;
}


float AudioFileSpeech::getMarkTime( RequestId requestId, const XMLCh * markId )
{
   hash_map< RequestId, SpeechRequestInfo >::iterator it = m_speechRequestInfo.find( requestId );
   if ( it != m_speechRequestInfo.end() )
   {
      char * xmlMarkId = XMLString::transcode( markId );
      string strMarkId = xmlMarkId;
      XMLString::release( &xmlMarkId );

      hash_map< string, float >::iterator markIt = it->second.timeMarkers.find( strMarkId );
      if ( markIt != it->second.timeMarkers.end() )
      {
         return markIt->second;
      }
   }

   return 0;
}


void AudioFileSpeech::requestComplete( RequestId requestId )
{
   m_speechRequestInfo.erase( requestId );
}


void AudioFileSpeech::ReadVisemeDataLTF( const char * filename, std::vector< VisemeData > & visemeData )
{
   // phonemeIndex isn't needed unless you want to use the strings for debugging purposes
   // the 0-40 number indices are used in the .ltf file  (eg, index 10 equals phoneme "Oy", which is mapped to Viseme "oh")
   // phonemeToViseme was created by hand, using the chart below (taken from doctor.map).  Phonemes are along the side, Visemes are at the top.  Eat, Earth, etc are the Impersonator names, EE, Er, Ih are the names used by the Bonebus
/*
   //phoneme-viseme map Impersonator
   //                          Eat   Earth If    Ox    Oat   Wet   Size Church Fave Though Told Bump   New   Roar Cage
   const string phon2Vis[] = { "EE", "Er", "Ih", "Ao", "oh", "OO", "Z", "j",   "F", "Th",  "D", "BMP", "NG", "R", "KG" };

                           Iy=0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ih=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Eh=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ey=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ae=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Aa=0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Aw=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ay=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ah=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ao=0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Oy=0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ow=0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Uh=0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Uw=0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Er=0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Ax=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           S =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Sh=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Z =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Zh=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           F =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.70, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Th=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00
                           V =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.70, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Dh=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00
                           M =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00
                           N =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00
                           Ng=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00
                           L =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00
                           R =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00
                           W =0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Y =0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Hh=0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           B =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00
                           D =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00
                           Jh=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           G =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.20
                           P =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00
                           T =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00
                           K =0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.20
                           Ch=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.85, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00

                           Sil=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           ShortSil=0.00, 0.00, 0.20, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00
                           Flap=0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.50, 0.00, 0.00, 0.00, 0.00
*/

   //                                  0                             5                             10                            15                          20                           25                          30                            35                            40
// const char * phonemeIndex[]    = { "Iy", "Ih", "Eh", "Ey", "Ae", "Aa", "Aw", "Ay", "Ah", "Ao", "Oy", "Ow", "Uh", "Uw", "Er", "Ax", "S", "Sh", "Z", "Zh", "F", "Th", "V", "Dh", "M",   "N",  "Ng", "L", "R", "W",  "Y",  "Hh", "B",   "D", "Jh", "G",  "P",   "T", "K",  "Ch", "Sil", "ShortSil", "Flap" };
   const char * phonemeToViseme[] = { "EE", "Ih", "Ih", "Ih", "Ih", "Ao", "Ih", "Ih", "Ih", "Ao", "oh", "oh", "oh", "oh", "Er", "Ih", "Z", "j",  "Z", "j",  "F", "Th", "F", "Th", "BMP", "NG", "NG", "D", "R", "OO", "OO", "Ih", "BMP", "D", "j",  "KG", "BMP", "D", "KG", "j",  "_",   "_",        "_" };


   visemeData.clear();

   FILE * f = fopen( filename, "r" );
   if ( f == NULL )
   {
      return;
   }


   // search for the "phoneme list" section in the file

   char line[ 512 ];
   while ( fgets( line, 512, f ) != NULL )
   {
      string strLine = line;
      if ( strLine.find( "// Phoneme Timing List" ) != strLine.npos )
      {
         break;
      }
   }

   while ( fgets( line, 512, f ) != NULL )
   {
      string strLine = line;

      if ( strLine.find( "// Function Curve Data" ) != strLine.npos )
      {
         // we've reached the end of the section, we're done
         break;
      }

      if ( strLine.length() < 0 )
      {
         continue;
      }

      // we're looking for a line in the following format:
      //   <phoneme index> <start time> <end time>
      //   eg: 40 0 0.123

      vector<string> tokens;
      vhcl::Tokenize( strLine, tokens );

      if ( tokens.size() < 3 )
      {
         // line is in the wrong format
         continue;
      }

      string strPhonemeIndex = tokens[ 0 ];
      string strStartTime = tokens[ 1 ];
      string strEndTime = tokens[ 2 ];

      string strVisemeIndex = phonemeToViseme[ atoi( strPhonemeIndex.c_str() ) ];

      visemeData.push_back( VisemeData( strVisemeIndex.c_str(), 1.0f, (float)atof( strStartTime.c_str() ) ) );
      visemeData.push_back( VisemeData( strVisemeIndex.c_str(), 0.0f, (float)atof( strEndTime.c_str() ) ) );
   }

   fclose( f );
}


void AudioFileSpeech::ReadVisemeDataBML( const char * filename, std::vector< VisemeData > & visemeData, const SbmCharacter* character )
{
   visemeData.clear();

   DOMDocument * doc = xml_utils::parseMessageXml( m_xmlParser, filename );
   if ( doc == NULL )
   {
      return;
   }

   DOMElement * bml = doc->getDocumentElement();

   // TODO: make sure it's "bml"

   // <lips viseme="Ih" articulation="1.0" start="0.17" ready="0.17" relax="0.31" end="0.31" />
   bool useVisemeCurveMode = visemeCurveMode;
   if (useVisemeCurveMode)
   {
	   DOMNodeList* syncCurveList = bml->getElementsByTagName(L"curve");
	   for (XMLSize_t i = 0; i < syncCurveList->getLength(); i++)
	   {
		   DOMElement* e = (DOMElement*)syncCurveList->item(i);

		   char* xmlVisemeName = XMLString::transcode(e->getAttribute(L"name"));
		   string visemeName = xmlVisemeName;
		   XMLString::release(&xmlVisemeName);

		   char* xmlNumKeys = XMLString::transcode(e->getAttribute(L"num_keys"));
		   string numKeys = xmlNumKeys;
		   XMLString::release(&xmlNumKeys);

		   char* xmlCurveInfo = XMLString::transcode(e->getTextContent());
		   string curveInfo = xmlCurveInfo;
		   XMLString::release(&xmlCurveInfo);

		   visemeData.push_back(VisemeData(visemeName.c_str(), atoi(numKeys.c_str()), curveInfo.c_str()));
	   }
	   // revert to using normal viseme mode if curves are not found
	   if (syncCurveList->getLength() == 0)
		   useVisemeCurveMode = false;
   }
   
   if (!useVisemeCurveMode)
   {
	   DOMNodeList * syncList = bml->getElementsByTagName( L"lips" );
	   for ( XMLSize_t i = 0; i < syncList->getLength(); i++ )
	   {
		  DOMElement * e = (DOMElement *)syncList->item( i );

		  char * xmlViseme = XMLString::transcode( e->getAttribute( L"viseme" ) );
		  string viseme = xmlViseme;
		  XMLString::release( &xmlViseme );

		  char * xmlArticulation = XMLString::transcode( e->getAttribute( L"articulation" ) );
		  string articulation = xmlArticulation;
		  XMLString::release( &xmlArticulation );

		  char * xmlStart = XMLString::transcode( e->getAttribute( L"start" ) );
		  string start = xmlStart;
		  XMLString::release( &xmlStart );

		  char * xmlReady = XMLString::transcode( e->getAttribute( L"ready" ) );
		  string ready = xmlReady;
		  XMLString::release( &xmlReady );

		  char * xmlRelax = XMLString::transcode( e->getAttribute( L"relax" ) );
		  string relax = xmlRelax;
		  XMLString::release( &xmlRelax );

		  char * xmlEnd = XMLString::transcode( e->getAttribute( L"end" ) );
		  string end = xmlEnd;
		  XMLString::release( &xmlEnd );
		
		  float startTime = (float)atof( start.c_str() );
		  float endTime = (float)atof( end.c_str() );
		  float readyTime = (float)atof( ready.c_str() );
		  float relaxTime = (float)atof( relax.c_str() );		  
		  float rampIn = readyTime - startTime;
		  if (rampIn <= 0.0f)
			  rampIn = 0.1f;
		  float rampOut = endTime - relaxTime;
		  if (rampOut <= 0.0f)
			  rampOut = 0.1f;

		  float weight = (float) atof( articulation.c_str());
		  weight *= character->get_viseme_magnitude();
		  VisemeData* curViseme = new VisemeData(viseme.c_str(), weight, startTime,  endTime - startTime, rampIn, rampOut);

			if (visemeData.size() > 0)
			{
			  //VisemeData& prevViseme = visemeData.back();
			  VisemeData& prevViseme = visemeData.back();
			  //float blendTime = startTime - prevViseme.time();
			  //if (blendTime <= 0.1f)
				//  blendTime = .1f;
			  //prevViseme.setRampout(blendTime);
			  //prevViseme.setDuration(prevViseme.rampin() + prevViseme.rampout());
			  //rampIn = blendTime;
				if (character && !character->isVisemePlateau())
				{
					VisemeData& prevViseme = visemeData.back();
					float blendTime = (startTime - prevViseme.time());
					if (blendTime <= .1f)
						blendTime = .1f;
					curViseme->rampin(blendTime);
					prevViseme.rampout(blendTime);
					prevViseme.setDuration(prevViseme.rampin() + prevViseme.rampout());
				} else  {
					float blendIval = (startTime - prevViseme.time());
					if (blendIval <= .1f)
						blendIval = .1f;
					curViseme->rampin( blendIval * 0.5f );
					prevViseme.rampout( blendIval * 0.5f );
					if (visemeData.size() > 1)
					{
						VisemeData& prevPrevViseme = visemeData[visemeData.size() - 2];
					}
					prevViseme.setDuration( 1.5f * ( prevViseme.rampin() + prevViseme.rampout() ) );
				}
			}

			visemeData.push_back((*curViseme));
	   }
	   if (visemeData.size() > 0)
	   {
		   if (visemeData.size() > 1)
		   {
				// set the blend in duration for the first viseme
			   VisemeData& firstViseme = visemeData[0];
				// set the blend out duration for the last viseme
			   VisemeData& lastViseme = visemeData[visemeData.size() - 1];
			   if (character && !character->isVisemePlateau())
			   {
					firstViseme.rampin(firstViseme.rampout());
					firstViseme.setDuration(firstViseme.rampin() + firstViseme.rampout());
					
					lastViseme.rampout(lastViseme.rampin());
					lastViseme.setDuration(lastViseme.rampin() + lastViseme.rampout());
				} else  {
					firstViseme.rampin(firstViseme.rampout());
					firstViseme.setDuration(1.5f * (firstViseme.rampin() + firstViseme.rampout()));
					
					lastViseme.rampout(lastViseme.rampin());
					lastViseme.setDuration( 1.5f * ( lastViseme.rampin() + lastViseme.rampout()));
			   }
		   }
		   else
		   {
			   visemeData[0].setRampin(.1f);
			   visemeData[0].setRampout(.1f);
			   visemeData[0].setDuration(visemeData[0].rampin() + visemeData[0].rampout());
		   }
	   }
   }
}


void AudioFileSpeech::ReadSpeechTiming( const char * filename, stdext::hash_map< std::string, float > & timeMarkers )
{
   timeMarkers.clear();


   DOMDocument * doc = xml_utils::parseMessageXml( m_xmlParser, filename );
   if ( doc == NULL )
   {
      return;
   }

   DOMElement * bml = doc->getDocumentElement();

   // TODO: make sure it's "bml"

   // <sync id="T0" time="0.17" />If
   // <sync id="T1" time="0.36" />

   DOMNodeList * syncList = bml->getElementsByTagName( L"sync" );
   for ( XMLSize_t i = 0; i < syncList->getLength(); i++ )
   {
      DOMElement * e = (DOMElement *)syncList->item( i );

      char * xmlId = XMLString::transcode( e->getAttribute( L"id" ) );
      string id = xmlId;
      XMLString::release( &xmlId );

      char * xmlTime = XMLString::transcode( e->getAttribute( L"time" ) );
      string time = xmlTime;
      XMLString::release( &xmlTime );

      timeMarkers[ id ] = (float)atof( time.c_str() );
   }
}

stdext::hash_map< RequestId, AudioFileSpeech::SpeechRequestInfo >& AudioFileSpeech::getSpeechRequestInfo()
{
	return m_speechRequestInfo;
}
