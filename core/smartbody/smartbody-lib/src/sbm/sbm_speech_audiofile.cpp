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


using std::string;
using std::vector;
using stdext::hash_map;
using namespace SmartBody;


AudioFileSpeech::AudioFileSpeech()
{
   m_xmlParser = new XercesDOMParser();
   m_xmlParser->setErrorHandler( new HandlerBase() );

   m_requestIdCounter = 1;  // start with 1, in case 0 is a special case
}


AudioFileSpeech::~AudioFileSpeech()
{
   delete m_xmlParser;  m_xmlParser = NULL;
}


RequestId AudioFileSpeech::requestSpeechAudio( const char * agentName, const DOMNode * node, const char * callbackCmd )
{
   // TODO: Test this function with a variety of XML documents

   // TODO: transcode() leaks here
   string encoding = XMLString::transcode( node->getOwnerDocument()->getXmlEncoding() );  // XMLStringconverts XML to cString; encoding
   string version = XMLString::transcode( node->getOwnerDocument()->getXmlVersion () ); //the xml version number

   encoding = "UTF-8";  // getXmlEncoding() doesn't work on  'test bml char doctor <speech ref="hey"/>'

   string xmlConverted = "<?xml version=\"" + version.substr( 0, 6 )+ "\" ";
   xmlConverted += "encoding=\"" + encoding.substr( 0, 7 ) + "\"?>";

   xml_utils::xmlToString( node, xmlConverted ); //Xml to string recursively searches DOM tree and returns a string of the xml document

   return requestSpeechAudio( agentName, xmlConverted.c_str(), callbackCmd );
}


RequestId AudioFileSpeech::requestSpeechAudio( const char * agentName, const char * text, const char * callbackCmd )
{
   // TODO:  Does return 0 signify error code?
   // TODO:  Handle xerces exceptions?

   // sample input
   // +		text	0x01c5a318 "<?xml version="1.0" encoding="UTF-8"?><speech ref="off-top-dont-know" type="text/plain">I don't know the answer to that question</speech>"	const char *
   /*
    +		text	0x04883548 "<?xml version="1.0" encoding="UTF-8"?><speech id="sp1" ref="rio-i_like_shootin_things" type="application/ssml+xml">
         <mark name="sp1:T0" />And
         <mark name="sp1:T1" />
         <mark name="sp1:T2" />I
         <mark name="sp1:T3" />
         <mark name="sp1:T4" />like
         <mark name="sp1:T5" />
         <mark name="sp1:T6" />shooting
         <mark name="sp1:T7" />
         <mark name="sp1:T8" />things,
         <mark name="sp1:T9" />
         <mark n	const char *
   */

   // parse text to get the name of the audio file
   // parse .bml file to get viseme timings
   // parse .bml file to get mark timings


   DOMDocument * doc = xml_utils::parseMessageXml( m_xmlParser, text );

   DOMElement * speech = doc->getDocumentElement();

   // TODO: make sure it's "speech"


   char * xmlRef = XMLString::transcode( speech->getAttribute( L"ref" ) );
   string ref = xmlRef;
   XMLString::release( &xmlRef );

   char * xmlSpeechId = XMLString::transcode( speech->getAttribute( L"id" ) );
   string speechId = xmlSpeechId;
   XMLString::release( &xmlSpeechId );


   m_speechRequestInfo[ m_requestIdCounter ].id = speechId;


   mcuCBHandle& mcu = mcuCBHandle::singleton();

   SbmCharacter * agent = mcu.character_map.lookup( agentName );
   if ( agent == NULL )
   {
      printf( "AudioFileSpeech::requestSpeechAudio ERR: insert AudioFile voice code lookup FAILED, msgId=%s\n", agentName ); 
      return 0;
   }


   char fullAudioPath[ _MAX_PATH ];
   string relativeAudioPath = (string)"../../../../" + agent->get_voice_code();
   if ( _fullpath( fullAudioPath, relativeAudioPath.c_str(), _MAX_PATH ) == NULL )
   {
      printf( "AudioFileSpeech::requestSpeechAudio ERR: _fullpath() returned NULL\n" );
      return 0;
   }

   m_speechRequestInfo[ m_requestIdCounter ].audioFilename = (string)fullAudioPath + "\\" + ref + ".wav";


   // TODO: Should we fail if the .bml file isn't present?

   string bmlFilename = "../../../../" + agent->get_voice_code() + "/" + ref + ".bml";

   ReadVisemeDataBML( bmlFilename.c_str(), m_speechRequestInfo[ m_requestIdCounter ].visemeData );
   if ( m_speechRequestInfo[ m_requestIdCounter ].visemeData.size() == 0 )
   {
      printf( "AudioFileSpeech::requestSpeechAudio ERR: could not read visemes from file: %s\n", bmlFilename.c_str() );
      //return 0;
   }

   ReadSpeechTiming( bmlFilename.c_str(), m_speechRequestInfo[ m_requestIdCounter ].timeMarkers );
   if ( m_speechRequestInfo[ m_requestIdCounter ].timeMarkers.size() == 0 )
   {
      printf( "AudioFileSpeech::requestSpeechAudio ERR: could not read time markers file: %s\n", bmlFilename.c_str() );
      //return 0;
   }


   mcu.execute_later( vhcl::Format( "%s %s %d %s", callbackCmd, agentName, m_requestIdCounter, "SUCCESS" ).c_str() );


   return m_requestIdCounter++;
}


const vector<VisemeData *> * AudioFileSpeech::getVisemes( RequestId requestId )
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
         visemeCopy->push_back( new VisemeData( v.id(), v.weight(), v.time() ) );
      }

      return visemeCopy;
   }

   return NULL;
}


char * AudioFileSpeech::getSpeechPlayCommand( RequestId requestId, const SbmCharacter * character )
{
   // TODO: Wrap up this SASO/PlaySound specific audio command string generation
   // into a class that can abstracted and shared between speech implementations.
   // The SpeechInterface should only need to provide the audio filename.

   // TODO: fix return type

   hash_map< RequestId, SpeechRequestInfo >::iterator it = m_speechRequestInfo.find( requestId );
   if ( it != m_speechRequestInfo.end() )
   {
      int characterId = 0;
      if ( character && character->bonebusCharacter )
      {
         characterId = character->bonebusCharacter->m_charId;
      }

      it->second.playCommand = vhcl::Format( "send PlaySound %s %d", it->second.audioFilename.c_str(), characterId );
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
      int characterId = 0;
      if ( character && character->bonebusCharacter )
      {
         characterId = character->bonebusCharacter->m_charId;
      }

      it->second.stopCommand = vhcl::Format( "send StopSound %s %d", it->second.audioFilename.c_str(), characterId );
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


void AudioFileSpeech::ReadVisemeDataBML( const char * filename, std::vector< VisemeData > & visemeData )
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


      visemeData.push_back( VisemeData( viseme.c_str(), (float)atof( articulation.c_str() ), (float)atof( start.c_str() ) ) );
      visemeData.push_back( VisemeData( viseme.c_str(), 0.0f,                                (float)atof( end.c_str() ) ) );
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


      // prefix the id with the speech id so that it matches the ids that come in from NVB
      id = m_speechRequestInfo[ m_requestIdCounter ].id + ":" + id;


      timeMarkers[ id ] = (float)atof( time.c_str() );
   }
}
