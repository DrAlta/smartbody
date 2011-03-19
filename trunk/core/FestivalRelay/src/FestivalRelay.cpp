/*************************************************************************/
/*                                                                       */
/*                Centre for Speech Technology Research                  */
/*                     University of Edinburgh, UK                       */
/*                       Copyright (c) 1996,1997                         */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  THE UNIVERSITY OF EDINBURGH AND THE CONTRIBUTORS TO THIS WORK        */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL THE UNIVERSITY OF EDINBURGH NOR THE CONTRIBUTORS BE LIABLE     */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*                      Author :  Shridhar Ravikumar                     */
/*                      Date   :  September 2010                         */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*						Festival Text To Speech Relay					 */
/* 																		 */
/*=======================================================================*/

#include "vhcl.h"

#include <xercesc/util/XMLUTF8Transcoder.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <xercesc/util/OutOfMemoryException.hpp>
//New Modifications to better use XML structure of input message
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <algorithm>
#include <direct.h>
#include "vhmsg-tt.h"
#include <festival.h>
#include <VHDuration.h>




// class is taken from somewhere, unsure of it's origins.  Here is one of many examples of use across the net:
//    http://mail-archives.apache.org/mod_mbox/xerces-c-dev/200210.mbox/%3C7C78B8615661D311B15500A0C9AB28C1915E40@earth.telestream.net%3E
class XStr
{
   public:
      // -----------------------------------------------------------------------
      //  Constructors and Destructor
      // -----------------------------------------------------------------------
      XStr( const char * const toTranscode )
      {
         // Call the private transcoding method
         fUnicodeForm = xercesc_3_0::XMLString::transcode( toTranscode );
      }

      ~XStr()
      {
         xercesc_3_0::XMLString::release( &fUnicodeForm );
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
//#define FESTIVAL_HEAP_SIZE 999999

bool done = false;
extern SpeechRequestData xmlMetaData;


const char * FESTIVAL_RELAY_LIB_DIR = "..\\..\\lib\\festival\\festival\\lib";

string cache_directory = "..\\..\\data\\cache\\festival\\";


/// Cleans up spurious whitespaces in string, and removes weird \n's
void cleanString(std::string &message)
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
	while ( (pos = message.find("  ")) != std::string::npos )
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


// read the input RemoteSpeechCommand message and store it's data so we can use it for generating the output RemoteSpeechReply message
std::string storeXMLMetaData( const std::string & txt)
{

   /// Put in some defaults, and do basic cleanup just to be safe
   xmlMetaData.speechIdentifier = "sp1";
   xmlMetaData.tags.clear();
   xmlMetaData.words.clear();
   /// Start an XML parser to parse the message we have received
   xercesc_3_0::XMLPlatformUtils::Initialize();
   xercesc_3_0::XercesDOMParser *parser = new XercesDOMParser();

   std::string truncatedTxt = txt.substr(txt.find_first_of(">")+1);
   char * message = (char*)truncatedTxt.c_str();

   std::string actualText = "";

   /// Set up a parser for XML message in memory - code sourced from unknown online reference for Xerces XML library
   xercesc_3_0::MemBufInputSource memIS((const XMLByte*)message, strlen(message), "XMLBuffer");
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
					   std::string markString(t1 = XMLString::transcode(mark));
					   std::string speechString;

					   if(speechNode !=NULL)
					   {
						   t2 = (speech)?XMLString::transcode(speech): " ";
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
	   fprintf(stderr, "Error: Unable to instantiate DOM Xml parser, exiting \n");
	   return "";
   }
}


// remove extra tabs from the string and replce with spaces
void removeTabsFromString(string &spoken_text)
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

// generate the sound file for the specified text
std::string generateReply(const char * utterance,const char * soundFileName)
{

	EST_Wave wave;

	string spoken_text = storeXMLMetaData( utterance );

	removeTabsFromString(spoken_text);

	//festival_say_text(spoken_text.c_str());
	festival_text_to_wave(spoken_text.c_str(),wave);
    wave.save(soundFileName,"riff");

	return xmlMetaData.replyString;
}


// process the input RemoteSpeechCommand message
void process_message( const char * message )
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

	std::string festival_file_name = cache_directory + file_name;

	//Generate the audio
	string replyXML = generateReply(utterance.c_str(),festival_file_name.c_str());

	string remoteSpeechReply = agent_name+" "+message_id+" OK: <?xml version=\"1.0\" encoding=\"UTF-8\"?><speak><soundFile name=\"";

	char full[ _MAX_PATH ];
	if ( _fullpath( full, festival_file_name.c_str(), _MAX_PATH ) == NULL )
	{
		printf("\nError converting path sent from SBM to absolute path\n");
	}
	string soundPathName = full;

	remoteSpeechReply += soundPathName+"\"/>";
	remoteSpeechReply += replyXML + "</speak>";


	//Only send out a reply when result is not empty, ignore otherwise as a nother voice relay might pick up the request
	if ( replyXML.compare("") != 0 )
	{
	  vhmsg::ttu_notify2( "RemoteSpeechReply", remoteSpeechReply.c_str() );
	}
}


// handles messages on receiving them
void tt_client_callback( const char * op, const char * args, void * user_data )
{
   string sOp = op;
   string sArgs = args;
   vector< string > splitArgs;
   vhcl::Tokenize( sArgs, splitArgs );

   if ( sOp.compare( "vrAllCall" ) == 0 )
   {
      vhmsg::ttu_notify2( "vrComponent", "tts" );
   }
   if ( sOp.compare( "RemoteSpeechCmd" ) == 0 )
   {
      process_message( args );
   }
   if ( sOp.compare( "vrKillComponent" ) == 0 )
   {
      if ( splitArgs.size() > 0 )
      {
         if ( splitArgs[ 0 ].compare( "tts" ) == 0 ||
              splitArgs[ 0 ].compare( "all" ) == 0 )
         {
			vhmsg::ttu_notify2( "vrProcEnd", "tts festival" );
			done =true;
         }
      }
   }
}



// initialize activeMQ
bool open_activeMQ()
{
   vhmsg::ttu_set_client_callback( tt_client_callback );

   int err = vhmsg::ttu_open();
   if ( err != vhmsg::TTU_SUCCESS )
   {
      cout<<"ttu_open failed!";
      return false;
   }

   // register for messages
   vhmsg::ttu_register( "vrKillComponent" );
   vhmsg::ttu_register( "vrAllCall" );
   vhmsg::ttu_register( "RemoteSpeechCmd" );
   vhmsg::ttu_notify2( "vrComponent", "tts" );

   return true;
}


// close activeMQ
void close_activeMQ()
{
   vhmsg::ttu_close();
}


BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    //char mesg[128];

    switch(CEvent)
    {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		vhmsg::ttu_notify2("vrProcEnd", "tts festival");
        break;    
	default:
        break;

    }
    return TRUE;
}



int main(int argc, char **argv)
{
    festival_libdir = FESTIVAL_RELAY_LIB_DIR;

    int heap_size = FESTIVAL_HEAP_SIZE;  
    int load_init_files = 1; // we want the festival init files loaded
    festival_initialize(load_init_files,heap_size);

	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		// unable to install handler... 
		// display message to the user

		printf("Unable to install handler for console events!\n");
		return -1;
	}


	printf( "Festival Text To Speech Engine:\n\n" );
	printf( "Initializing....\n");
	printf( "Hooking up VH Module\n");
	// setting the duration method to be used by festival
	festival_eval_command("(Parameter.set `Duration_Method Duration_Default)");
	// this command hooks our virtual human method such that every time an utterance is synthesized, our method is called on it
	// in order to generate the virtual human message (RemoteSpeechReply)
	festival_eval_command("(set! after_synth_hooks (list Duration_VirtualHuman))");
	// setting duration stretch parameter
	festival_eval_command("(Parameter.set 'Duration_Stretch 0.8)");
	festival_eval_command("(set! voice_default 'voice_rab_diphone)");

	
	printf( "Starting ActiveMQ\n");
	// initialize activeMQ
	bool success = open_activeMQ();
	if(!success)
	{
		cout<<"Could not initialize ActiveMQ";
		return 1;
	}

	printf( "Checking for Cache Directory\n");
	// check to see if cache directory exists and if not create it
	if( !(_access( cache_directory.c_str(), 0 ) == 0 ) )
    {
		std::string temp = "";
		std::vector< std::string > tokens;
		const std::string delimiters = "\\/";

		vhcl::Tokenize( cache_directory.c_str(), tokens, delimiters );

		printf( "Warning, audio cache directory, %s, does not exist. Creating directory...\n", cache_directory.c_str() );
		for (unsigned int i = 0; i < tokens.size(); i++)
		{
		 temp += tokens.at( i ) + "\\";
		 _mkdir( temp.c_str() );
		}
	}

	printf( "Done Initializing\n");

	// poll for messages
	while(!done)
	{
		vhmsg::ttu_wait(1.0);
	}

	close_activeMQ();
    return 0;
}
