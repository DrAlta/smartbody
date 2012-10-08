/*
 *  local_speech.cpp - part of SmartBody-lib
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
 *      Ed Fast, USC
 */

#include "vhcl.h"

#include "local_speech.h"
#ifdef WIN32
#include <direct.h>
#define getFullPath(result,filename) _fullpath(result,filename,_MAX_PATH)
#define checkPermission(pathname,mode) _access(pathname,mode)
#define makeDirectory(pathname) _mkdir(pathname)
#else
#include <unistd.h>
#define _MAX_PATH 3000
#define getFullPath(result,filename) realpath(filename,result)
#define checkPermission(pathname,mode) access(pathname,mode)
#define makeDirectory(pathname) mkdir(pathname,0777)
#endif

#if 0
#include <festival.h>
#include <VHDuration.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sstream>
#include <float.h>
#include "time.h"

#include "sbm/xercesc_utils.hpp"
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include "sbm/BMLDefs.h"


using namespace std;
using namespace SmartBody;

class XStr
{
   public:
      // -----------------------------------------------------------------------
      //  Constructors and Destructor
      // -----------------------------------------------------------------------
      XStr( const char * const toTranscode )
      {
         // Call the private transcoding method
         //fUnicodeForm = xercesc_3_0::XMLString::transcode( toTranscode );
	  fUnicodeForm = XMLString::transcode( toTranscode );
      }

      ~XStr()
      {
         //xercesc_3_0::XMLString::release( &fUnicodeForm );
	  XMLString::release( &fUnicodeForm );
      }


      // -----------------------------------------------------------------------
      //  Getter methods
      // -----------------------------------------------------------------------
      const XMLCh * unicodeForm() const
      {
         return fUnicodeForm;
      }

   private:
       // -----------------------------------------------------------------------
       //  Private data members
       //
       //  fUnicodeForm
       //      This is the Unicode XMLCh format of the string.
       // -----------------------------------------------------------------------
       XMLCh * fUnicodeForm;
};




#define X( str ) XStr( str ).unicodeForm()
XERCES_CPP_NAMESPACE_USE
#if 0
extern SpeechRequestData xmlMetaData;
extern std::map<string,string> phonemeToViseme;
extern std::string mapping;

FestivalSpeechRelayLocal::FestivalSpeechRelayLocal()
{
	
}

FestivalSpeechRelayLocal::~FestivalSpeechRelayLocal()
{
}

std::string FestivalSpeechRelayLocal::generateReply(const char * utterance,const char * soundFileName)
{
	EST_Wave wave;

	string spoken_text = storeXMLMetaData( utterance );
	//LOG("after store XMLMetaData");
	//printf("done first time\n");
	if (!spoken_text.compare("") && spoken_text != "")
	{
		puts(spoken_text.append("\n").c_str());
	}

	//LOG("after spoken text compare");
	if (xmlMetaData.tags.size() <= 0)
	{
		spoken_text = TransformTextWithTimes(utterance);
		//LOG("done transforming\n");
		if (!spoken_text.compare("") && spoken_text != "")
		{
			puts(spoken_text.append("\n").c_str());
		}
		spoken_text = storeXMLMetaData(spoken_text);
		//LOG("done second time\n");
	}
	//LOG("before remove tab");
	removeTabsFromString(spoken_text);

	//LOG( "generateReply() - \nbefore: '%s'\nafter: '%s'\n'%s'\n", utterance, spoken_text.c_str(), soundFileName );

	//festival_say_text(spoken_text.c_str());
	festival_text_to_wave(spoken_text.c_str(),wave);
    wave.save(soundFileName,"riff");

	return xmlMetaData.replyString;
}
std::string FestivalSpeechRelayLocal::TransformTextWithTimes(std::string txt)
{
	//std::string text_string = "";

	//#define _PARSER_DEBUG_MESSAGES_ON
   std::stringstream txtstream;

   /// Start an XML parser to parse the message we have received
   XMLPlatformUtils::Initialize();
   XercesDOMParser *parser = new XercesDOMParser();

   std::string truncatedTxt = txt.substr(txt.find_first_of(">")+1);
   char * message = (char*)truncatedTxt.c_str();

   std::string actualText = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
#ifdef _PARSER_DEBUG_MESSAGES_ON
   fprintf(stderr, "Debug: Parsing message \"%s\"\n",message);
#endif
   /// Set up a parser for XML message in memory - code sourced from unknown online reference for Xerces XML library
   MemBufInputSource memIS((const XMLByte*)message, strlen(message), "XMLBuffer");
   parser->parse(memIS);
   DOMDocument *doc = parser->getDocument();
   if ( doc )
   {
	   DOMElement *root = doc->getDocumentElement();

	   if ( root ) 
	   {
		   /// Get all nodes which have the "mark" tag, from these we can extract the timing tags, and speech text
		   DOMNodeList *messageList = root->getElementsByTagName(XMLString::transcode("speech"));
		   if ( messageList && messageList->getLength() > 0)
		   {
			   DOMElement *speechElement = dynamic_cast<DOMElement*>(messageList->item(0));
			   char *speechID = XMLString::transcode(speechElement->getAttribute(XMLString::transcode("id")));

			   actualText = actualText.append("<speech id=\"" + std::string(speechID) + "\" ref=\"" + XMLString::transcode(speechElement->getAttribute(X("ref"))) + "\" type=\"" + XMLString::transcode(speechElement->getAttribute(X("type"))) + "\">\n\n");
			   XMLString::release(&speechID);
		   }
		   else if ( !strcmp( XMLString::transcode( root->getNodeName() ), "speech") ) {
			   /// Else, the message might contain only the speech tag, in which case the above code will fail, so we need a fallback
			   DOMElement *speechElement = root;
			   char *speechID = XMLString::transcode(speechElement->getAttribute(XMLString::transcode("id")));
			   char *text = XMLString::transcode(speechElement->getTextContent());

			   std::string textContent = CreateMarkTimeStamps(text);
			   /*if (!speechID)
			   {
				   speechID = "sp1";
			   }*/
			   //hard coding sp1
			   speechID = "sp1";
			   //XMLString::transcode(speechElement->getAttribute(X("type"))) 
			   actualText = actualText.append("<speech id=\"" + std::string(speechID) + "\" ref=\"" + XMLString::transcode(speechElement->getAttribute(X("ref"))) + "\" type=\"" + "application/ssml+xml" + "\">\n\n");
			   actualText = actualText.append(textContent);
			   actualText = actualText.append("</speech>");
			   //XMLString::release(&speechID);
		   }
	   }
   }
   return actualText;
}

std::string FestivalSpeechRelayLocal::CreateMarkTimeStamps(std::string text)
{
	std::string tempText = text;
	std::string markUp = "";
	int i = 0;
	while (tempText!= "")
	{
		std::string temp = tempText;
		temp = temp.substr(0, tempText.find_first_of(" "));

		char number[256];
		sprintf(number, "%d", i);
		markUp = markUp.append("<mark name=\"T");
		markUp = markUp.append(number);
		markUp = markUp.append("\" />");
		markUp = markUp.append(temp + "\n\n");
		sprintf(number, "%d", ++i);
		markUp = markUp.append("<mark name=\"T");
		markUp = markUp.append(number);
		markUp = markUp.append("\" />\n\n");
		++i;
		if (tempText.size() > temp.size())
		{
			tempText = tempText.substr(temp.size() + 1);
		}
		else
		{
			tempText = "";
		}
	}
	return markUp;
}
void FestivalSpeechRelayLocal::removeTabsFromString(string &spoken_text)
{
	std::string::size_type pos = 0;

	for (pos = 0; pos < spoken_text.length(); ++pos)
	{
	  if (spoken_text.at(pos)== 9)
	  {
		  if(spoken_text.at(pos-1) == 9)
			spoken_text.replace(pos, 1, "");
		  else
			spoken_text.replace(pos, 1, " ");
	  }
	}
}
std::string FestivalSpeechRelayLocal::storeXMLMetaData( const std::string & txt)
{
	/// Put in some defaults, and do basic cleanup just to be safe
   xmlMetaData.speechIdentifier = "sp1";
   xmlMetaData.tags.clear();
   xmlMetaData.words.clear();
   /// Start an XML parser to parse the message we have received
   //xercesc_3_0::XMLPlatformUtils::Initialize();
   //xercesc_3_0::XercesDOMParser *parser = new XercesDOMParser(); 
   XMLPlatformUtils::Initialize();
   XercesDOMParser *parser = new XercesDOMParser(); 

   std::string truncatedTxt = txt.substr(txt.find_first_of(">")+1);
   char * message = (char*)truncatedTxt.c_str();

   std::string actualText = "";

   /// Set up a parser for XML message in memory - code sourced from unknown online reference for Xerces XML library
   //xercesc_3_0::MemBufInputSource memIS((const XMLByte*)message, strlen(message), "XMLBuffer");
   MemBufInputSource memIS((const XMLByte*)message, strlen(message), "XMLBuffer");
   parser->parse(memIS);
   DOMDocument *doc = parser->getDocument();
   if ( doc )
   {
	   DOMElement *root = doc->getDocumentElement();

	   if ( root ) 
	   {
		   /// Get all nodes which have the "mark" tag, from these we can extract the timing tags, and speech text
		   DOMNodeList *messageList = root->getElementsByTagName(XMLString::transcode("speech"));
		   if ( messageList && messageList->getLength() > 0)
		   {
			   DOMElement *speechElement = dynamic_cast<DOMElement*>(messageList->item(0));
			   char *speechID = XMLString::transcode(speechElement->getAttribute(XMLString::transcode("id")));
			   xmlMetaData.speechIdentifier = std::string(speechID);			   
			   XMLString::release(&speechID);
		   }
		   else if ( !strcmp( XMLString::transcode( root->getNodeName() ), "speech") ) {
			   /// Else, the message might contain only the speech tag, in which case the above code will fail, so we need a fallback
			   DOMElement *speechElement = root;
			   char *speechID = XMLString::transcode(speechElement->getAttribute(XMLString::transcode("id")));
			   xmlMetaData.speechIdentifier = std::string(speechID);			   
			   XMLString::release(&speechID);
		   }
		   else
		   {
			   /// Oops, for some reason all of the above didn't work, default to the default speech id
			   fprintf(stderr, "Warning: Could not find speech tag in message, creating message beginning by default\n");
		   }
		   messageList = root->getElementsByTagName( X("mark"));

		   /// Store all the book marks in the input message, so that they can be retrieved later
		   if ( messageList ) 
		   {
			   XMLSize_t nLetters = messageList->getLength();

			   for ( XMLSize_t i = 0; i < nLetters; i++ )
			   {
				   DOMNode* node = messageList->item(i);
				   /// We only want to parse XML elements
				   if (node->getNodeType() && node->getNodeType() == DOMNode::ELEMENT_NODE)
				   {
					   DOMElement *element = dynamic_cast<DOMElement*>( node );


					   //print the DOM to a memory terminal 
						DOMLSSerializer* theSerializer2 = DOMImplementation::getImplementation()->createLSSerializer();
						XMLFormatTarget *myFormatTarget2 = new MemBufFormatTarget();
						DOMLSOutput* myLSOutput2 = DOMImplementation::getImplementation()->createLSOutput();
						myLSOutput2->setByteStream( myFormatTarget2 );

						// serialize a DOMNode to an internal memory buffer
						theSerializer2->write(element, myLSOutput2);

						// print the final bml code to the terminal
						string output =
						(char*)  ((MemBufFormatTarget*)myFormatTarget2)->getRawBuffer();		
					   

					   XMLCh *mark = (XMLCh*)element->getAttribute(XMLString::transcode("name"));
					   XMLCh *speech = NULL;
					   DOMNode *speechNode = element->getFirstChild();
					   


					   if ( speechNode == NULL ) 
					   {
						   speechNode = element->getNextSibling();		
					   }
					   if ( (speechNode !=NULL) && ( speechNode->getNodeType() == DOMNode::TEXT_NODE ) )
					   {
						   speech = (XMLCh*)speechNode->getNodeValue();
					   }					   					   



					   /// Get the timing tag as a string
						
					   char * t1, *t2;
					   char* spaceT = " ";        
					   std::string markString(t1 = XMLString::transcode(mark));
					   std::string speechString;

					   if(speechNode !=NULL)
					   {
						   const char* transcodeT2 = (speech)? XMLString::transcode(speech): " ";
						   t2 = const_cast<char*>(transcodeT2) ;
						   speechString = t2;
					   }
					   else
						   speechString = "";
						   
					   XMLString::release(&t1);
					   XMLString::release(&t2);

					   if( !strcmp(markString.c_str(),"") || !strcmp(speechString.c_str(),"") )
					   {
						   /// Null strings tend to cause problems with later code sections
						   if ( !strcmp(speechString.c_str(),"") ) speechString = " ";
					   }
					   else
					   {
						   std::string temporaryText = speechString;
						   cleanString(temporaryText);
						   /// Words can be used to match up which tags we need to see
							/// Push tag and word into xmlMetaData
							xmlMetaData.tags.push_back(markString);
							xmlMetaData.words.push_back(temporaryText);
							actualText += temporaryText;
							cleanString(actualText);
					   }
				   }
			   }
		   }
		   else 
		   {
			   fprintf(stderr, "Error: Got no nodes with specified tag.\n");
		   }
	   }
	   else
	   {
		   fprintf(stderr, "Error: Could not extract root element from XML DOM document\n");
	   }
   }
   else
   {
	   fprintf(stderr, "Error: XML DOM document is null!\n");
   }
   doc->release();

   if ( actualText != "" ) {
	   return actualText;
   }
   else {
	   //fprintf(stderr, "Error: Unable to instantiate DOM Xml parser, exiting \n");
	   return "";
   }
}
void FestivalSpeechRelayLocal::cleanString(std::string &message)
{
	/// Remove newlines and carriage-returns
	message.erase( std::remove(message.begin(), message.end(), '\r'), message.end() );
	message.erase( std::remove(message.begin(), message.end(), '\n'), message.end() );
	/// If it's a space string, we want to leave the last whitespace
	while ( message.find_last_of(" ") == message.length() - 1  && message.length() > 1)
	{
		//fprintf(stderr,"Debug: Reducing length by 1 to remove whitespace at end\n");
		message.resize( message.length() - 1 );
	}

	unsigned int pos;
	while ( (pos = message.find("  ",message.size())) != std::string::npos && pos < message.size())
	{
		//fprintf(stderr,"Debug: replacing 2 whitespaces at %d(%s) with 1 whitespace\n",pos, message.substr(pos,2).c_str());
		message.replace( pos, 2, " " );
	}

	while (( message.find_first_of(" ") == 0 ) && (message.length() > 1))
	{		
		//fprintf(stderr,"Debug: Cleaning initial whitespace %s\n",message[0]);
		message = message.substr(1);
	}
}

void FestivalSpeechRelayLocal::processSpeechMessage( const char * message )
{
	std::string message_c = message;

	// parse the string
	std::vector< std::string > tokens;
	const std::string delimiters = " ";
	vhcl::Tokenize( message_c, tokens, delimiters );

	/// Get non-XML components of the message
	std::string command = tokens.at( 0 );

	std::string agent_name = tokens.at( 1 );
	std::string message_id = tokens.at( 2 );
	std::string voice_id = tokens.at( 3 );
	std::string file_name = tokens.at( 4 );
	size_t prefix_length = message_c.find( file_name, 0 ) + file_name.length() + 1;
	std::string utterance = message_c.substr( prefix_length );  // strip off the prefix, only using the xml

	// remove anything after </speech> tag
	size_t postfix_pos = utterance.rfind( "</speech>" );
	if ( postfix_pos != std::string::npos )
	  utterance = utterance.substr( 0, postfix_pos + 9 );

	// parse out just the sound file name and give it a .wav file type
	int pos = file_name.find( ".aiff" );
	int pos2 = file_name.find( "utt" );

	file_name = file_name.substr( pos2, pos - pos2 ) + ".wav";
	std::string festival_file_name = cacheDirectory + file_name;
	//Generate the audio
	LOG("before reply");
	string replyXML = generateReply(utterance.c_str(),festival_file_name.c_str());
	LOG("after reply, replyXML = %s",replyXML.c_str());
	string remoteSpeechReply = agent_name+" "+message_id+" OK: <?xml version=\"1.0\" encoding=\"UTF-8\"?><speak><soundFile name=\"";
	LOG("remoteSpeechReply = %s",remoteSpeechReply.c_str());

	char full[ _MAX_PATH ];
	if ( getFullPath( full, const_cast<char*>(festival_file_name.c_str())) == NULL )
	{
		LOG("\nError converting path sent from SBM to absolute path\n");
	}
#if defined(__ANDROID__)
	string soundPathName = cacheDirectory + file_name;
#else
       string soundPathName = full;
#endif

	remoteSpeechReply += soundPathName+"\"/>";
	remoteSpeechReply += replyXML + "</speak>";
	LOG("replyXML = %s\n",replyXML.c_str());
	LOG("Sound path name = %s\n",soundPathName.c_str());


	mcuCBHandle& mcu = mcuCBHandle::singleton();
	char* cmdConst = const_cast<char*>(remoteSpeechReply.c_str());
	string replyCmd = "RemoteSpeechReply ";
	replyCmd = replyCmd + remoteSpeechReply; //cmdConst;
	//mcu.execute_later("RemoteSpeechReply", cmdConst ); //sends the remote speech command using singleton* MCU_p	
	LOG("replyCmd = %s",replyCmd.c_str());
	mcu.execute_later(replyCmd.c_str());
}

void FestivalSpeechRelayLocal::evalFestivalCommand( const char * cmd )
{
	int ret = festival_eval_command(cmd);
	LOG("%s : ret = %d\n", cmd,ret);	
}

void FestivalSpeechRelayLocal::setVoice(std::string voice)
{
	std::stringstream strstr;
	strstr << "(voice_" << voice << ")";
	int ret = festival_eval_command(strstr.str().c_str());
	LOG("Voice = %s : ret = %d\n", voice.c_str(),ret);	

	festival_eval_command("(Parameter.set `Duration_Method Duration_Default)");		
	festival_eval_command("(set! after_synth_hooks (list Duration_VirtualHuman))");	
	festival_eval_command("(Parameter.set 'Duration_Stretch 0.8)");
}

void FestivalSpeechRelayLocal::set_phonemes_to_visemes()
{

	if(mapping == "sbmold")
	{
		phonemeToViseme[ "pau" ] = "_";  // SIL
		phonemeToViseme[ "aa" ]  = "Ao"; // AA
		phonemeToViseme[ "ae" ]  = "Ih"; // AE
		phonemeToViseme[ "ah" ]  = "Ih"; // AH
		phonemeToViseme[ "ao" ]  = "Ao"; // AO
		phonemeToViseme[ "ax" ]  = "Ih"; // AX
		phonemeToViseme[ "@" ]   = "Ih"; // 
		phonemeToViseme[ "aw" ]  = "Ih"; // AW
		phonemeToViseme[ "ay" ]  = "Ih"; // AY
		phonemeToViseme[ "b" ]   = "BMP";//  B
		phonemeToViseme[ "ch" ]  = "j";  // CH
		phonemeToViseme[ "d" ]   = "D";  //  D
		phonemeToViseme[ "dh" ]  = "Th"; // DH
		phonemeToViseme[ "dx" ]  = "D";  // ??
		phonemeToViseme[ "eh" ]  = "Ih"; // EH
		phonemeToViseme[ "er" ]  = "Er"; // ER
		phonemeToViseme[ "ey" ]  = "Ih"; // 
		phonemeToViseme[ "f" ]   = "F";  //  F
		phonemeToViseme[ "g" ]   = "KG"; //  G
		phonemeToViseme[ "hh" ]  = "Ih"; // HH
		phonemeToViseme[ "ih" ]  = "Ih"; // IH
		phonemeToViseme[ "iy" ]  = "EE"; // IY
		phonemeToViseme[ "jh" ]  = "j";  // JH
		phonemeToViseme[ "k" ]   = "KG"; //  K
		phonemeToViseme[ "l" ]   = "D";  //  L
		phonemeToViseme[ "m" ]   = "BMP";//  M
		phonemeToViseme[ "n" ]   = "NG"; //  N
		phonemeToViseme[ "ng" ]  = "NG"; // NG
		phonemeToViseme[ "ow" ]  = "Oh"; // OW
		phonemeToViseme[ "oy" ]  = "Oh"; // OY
		phonemeToViseme[ "p" ]   = "BMP";//  P
		phonemeToViseme[ "r" ]   = "R";  //  R
		phonemeToViseme[ "s" ]   = "Z";  //  S
		phonemeToViseme[ "sh" ]  = "j";  // SH
		phonemeToViseme[ "T" ]   = "D";  // T?
		phonemeToViseme[ "t" ]   = "D";  // T?
		phonemeToViseme[ "th" ]  = "Th"; // TH
		phonemeToViseme[ "uh" ]  = "Oh"; // UH
		phonemeToViseme[ "uw" ]  = "Oh"; // UW
		phonemeToViseme[ "v" ]   = "F";  //  V
		phonemeToViseme[ "w" ]   = "OO"; //  W
		phonemeToViseme[ "y" ]   = "OO"; //  Y
		phonemeToViseme[ "z" ]   = "Z";  //  Z
		phonemeToViseme[ "zh" ]  = "J";  // ZH
	}
	else if (mapping == "sbm")
	{
		phonemeToViseme[ "pau" ] = "_";  // SIL
		phonemeToViseme[ "aa" ]  = "Aa"; // AA
		phonemeToViseme[ "ae" ]  = "Ah"; // AE
		phonemeToViseme[ "ah" ]  = "Ah"; // AH
		phonemeToViseme[ "ao" ]  = "Ao"; // AO
		phonemeToViseme[ "ax" ]  = "Ah"; // AX
		phonemeToViseme[ "@" ]   = "Ih"; // ??
		phonemeToViseme[ "aw" ]  = "Ah"; // AW
		phonemeToViseme[ "ay" ]  = "Ay"; // AY
		phonemeToViseme[ "b" ]   = "BMP";//  B
		phonemeToViseme[ "ch" ]  = "Sh";  // CH
		phonemeToViseme[ "d" ]   = "D";  //  D
		phonemeToViseme[ "dh" ]  = "Th"; // DH
		phonemeToViseme[ "dx" ]  = "D";  // ??
		phonemeToViseme[ "eh" ]  = "Eh"; // EH
		phonemeToViseme[ "er" ]  = "Er"; // ER
		phonemeToViseme[ "ey" ]  = "Eh"; // 
		phonemeToViseme[ "f" ]   = "F";  //  F
		phonemeToViseme[ "g" ]   = "Kg"; //  G
		phonemeToViseme[ "hh" ]  = "Ih"; // HH
		phonemeToViseme[ "ih" ]  = "Ih"; // IH
		phonemeToViseme[ "iy" ]  = "Ih"; // IY
		phonemeToViseme[ "jh" ]  = "Sh";  // JH
		phonemeToViseme[ "k" ]   = "Kg"; //  K
		phonemeToViseme[ "l" ]   = "L";  //  L
		phonemeToViseme[ "m" ]   = "BMP";//  M
		phonemeToViseme[ "n" ]   = "Kg"; //  N
		phonemeToViseme[ "ng" ]  = "Kg"; // NG
		phonemeToViseme[ "ow" ]  = "Ow"; // OW
		phonemeToViseme[ "oy" ]  = "Oy"; // OY
		phonemeToViseme[ "p" ]   = "BMP";//  P
		phonemeToViseme[ "r" ]   = "R";  //  R
		phonemeToViseme[ "s" ]   = "Z";  //  S
		phonemeToViseme[ "sh" ]  = "Sh";  // SH
		phonemeToViseme[ "T" ]   = "D";  // T?
		phonemeToViseme[ "t" ]   = "D";  // T?
		phonemeToViseme[ "th" ]  = "Th"; // TH
		phonemeToViseme[ "uh" ]  = "Eh"; // UH
		phonemeToViseme[ "uw" ]  = "Oh"; // UW
		phonemeToViseme[ "v" ]   = "F";  //  V
		phonemeToViseme[ "w" ]   = "W"; //  W
		phonemeToViseme[ "y" ]   = "Ih"; //  Y
		phonemeToViseme[ "z" ]   = "Z";  //  Z
		phonemeToViseme[ "zh" ]  = "Sh";  // ZH
	}

}

void FestivalSpeechRelayLocal::initSpeechRelay(std::string libPath, std::string cacheDir)
{
	
        //freopen ("/sdcard/sbm/festivalLog.txt","w",stderr);

	mapping = "sbm";
	set_phonemes_to_visemes();

	std::string scriptFile = "";
	std::string voice = "voice_kal_diphone";
	//std::string voice = "voice_roger_hts2010";
	festivalLibDirectory = libPath;
	cacheDirectory = cacheDir;
	std::string festivalLibDir = libPath;
	std::string cache_directory = cacheDir;
	
	festival_libdir = festivalLibDir.c_str();
    int heap_size = FESTIVAL_HEAP_SIZE;  
	int load_init_files = 1; // we want the festival init files loaded
    festival_initialize(load_init_files,heap_size);

	LOG( "Festival Text To Speech Engine:\n\n" );
	LOG( "Initializing....\n");
	LOG( "Hooking up VH Module\n");

	std::vector<std::string> festivalCommands;
	std::ifstream scriptStream(scriptFile.c_str());
	
	LOG("Festival lib directory (use -festivalLibDir): %s\n", festivalLibDir.c_str());
	LOG("Cache directory (use -festivalLibDir)       : %s\n", cache_directory.c_str());
	LOG("Voice (use -voice)                          : %s\n", voice.c_str());

	//if (!scriptFileRead)
	{
		LOG("Running default Festival commands\n\n", voice.c_str());
		//festivalCommands.push_back("(voice_roger_hts2010)");
		// setting the duration method to be used by festival
		festivalCommands.push_back("(Parameter.set `Duration_Method Duration_Default)");
		// this command hooks our virtual human method such that every time an utterance is synthesized, our method is called on it
		// in order to generate the virtual human message (RemoteSpeechReply)
		festivalCommands.push_back("(set! after_synth_hooks (list Duration_VirtualHuman))");
		// setting duration stretch parameter
		festivalCommands.push_back("(Parameter.set 'Duration_Stretch 0.8)");
		std::stringstream strstr;
		strstr << "(set! voice_default '" << voice << ")";
		festivalCommands.push_back(strstr.str());
	}

	LOG("\n");
	for (size_t x = 0; x < festivalCommands.size(); x++)
	{		
		int ret = festival_eval_command(festivalCommands[x].c_str());
		LOG("%s : ret = %d\n", festivalCommands[x].c_str(),ret);		
	}
	LOG("\n");	
	


	printf( "Checking for Cache Directory\n");
	// check to see if cache directory exists and if not create it
	if( !(checkPermission( cache_directory.c_str(), 0 ) == 0 ) )
    {
		std::string temp = "";
		std::vector< std::string > tokens;
		const std::string delimiters = "\\/";
		vhcl::Tokenize( cache_directory.c_str(), tokens, delimiters );
		printf( "Warning, audio cache directory, %s, does not exist. Creating directory...\n", cache_directory.c_str() );
		for (unsigned int i = 0; i < tokens.size(); i++)
		{
		 temp += tokens.at( i ) + "/";
		 makeDirectory( temp.c_str() );
		}
	}
	LOG( "Done Initializing local speech relay\n");	    
}
#else

FestivalSpeechRelayLocal::FestivalSpeechRelayLocal()
{
	
}

FestivalSpeechRelayLocal::~FestivalSpeechRelayLocal()
{
}
void FestivalSpeechRelayLocal::setVoice(std::string voice)
{
}


std::string FestivalSpeechRelayLocal::generateReply(const char * utterance,const char * soundFileName)
{
   std::string actualText;
   return actualText;	
}
std::string FestivalSpeechRelayLocal::TransformTextWithTimes(std::string dtxt)
{
   std::string actualText;
   return actualText;
}

std::string FestivalSpeechRelayLocal::CreateMarkTimeStamps(std::string text)
{
   std::string actualText;
   return actualText;
}
void FestivalSpeechRelayLocal::removeTabsFromString(string &spoken_text)
{
	
}
std::string FestivalSpeechRelayLocal::storeXMLMetaData( const std::string & txt)
{
	std::string actualText;
    return actualText;	
}
void FestivalSpeechRelayLocal::cleanString(std::string &message)
{
	
}

void FestivalSpeechRelayLocal::evalFestivalCommand( const char * cmd )
{
}

void FestivalSpeechRelayLocal::processSpeechMessage( const char * message )
{
	
}

void FestivalSpeechRelayLocal::initSpeechRelay(std::string libPath, std::string cacheDir)
{
	 
}
#endif
/* Local Speech Class */
#define FLOAT_EQ(x,v) (((v - DBL_EPSILON) < x) && (x <( v + DBL_EPSILON)))
#define LOG_RHETORIC_SPEECH (0)
#define USE_CURVES_FOR_VISEMES 0

local_speech::local_speech( float timeOutInSeconds )
:	remote_speech(timeOutInSeconds) // default is 10 seconds
{}

local_speech::~local_speech()
{}

void local_speech::sendSpeechCommand(const char* cmd)
{
	//LOG("speech cmd = %s",cmd);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	char* cmdConst = const_cast<char*>(cmd);
	mcu.execute("RemoteSpeechCmd", cmdConst ); //sends the remote speech command using singleton* MCU_p
}

