/*
   Part of SBM: SmartBody Module
   Copyright (C) 2008  University of Southern California

   SBM is free software: you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public License
   as published by the Free Software Foundation, version 3 of the
   license.

   SBM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   Lesser GNU General Public License for more details.

   You should have received a copy of the Lesser GNU General Public
   License along with SBM.  If not, see:
       http://www.gnu.org/licenses/lgpl-3.0.txt

   CONTRIBUTORS:
      Edward Fast, USC
      Thomas Amundsen, USC
*/

#include "vhcl.h"

#include "cerevoice_tts.h"

#include <map>
#include <sstream>
#include <fstream>
#include <conio.h>
#include <io.h>

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

extern "C"
{
   #include "cerenorm_wrap.h"
   #include "cerevoice.h"
   #include "cerevoice_io.h"
}

#include "cerevoice_pmod.h"


//#define MAXLINESZ 1024
#define MAXLINESZ 4095
#define MAXSPURTSZ 32000


XERCES_CPP_NAMESPACE_USE


// license strings
char * lic_text;
char * signature;
char * licfile;
CPRC_voice * voice;
std::map<std::string, std::string> phonemeToViseme;
static char * EMPTY_STRING = "";

//normalization file names
char * abbfile;
char * pbreakfile;
char * homographfile;
char * reductionfile;
char * rulesfile;


//  removes XML tags and new lines from a string - used to take out
//  time markings before processing text, otherwise
//  silence is put in place of them
// the opening speech tag will remain, and a new line must be put at the end
std::string removeXMLTagsAndNewLines( const std::string & txt )
{
   std::stringstream txtstream;

   //for the entire input string
   for ( unsigned int i = 0; i < txt.length(); i++ )
   {
      //if the character is an opening bracket
      if ( txt.at( i ) == '<' )
      {
         //loop until hitting the ending bracket
         //so that this text is not input into
         //the stringstream
         while ( i < txt.length() && txt.at( i ) != '>' )
         {
            i++;
         }

         //to skip over the ending bracket
         i++;
      }

      //add the character to the stringstream
      if ( i < txt.length() && txt.at( i ) != '\n' )
         txtstream << txt.at( i );
   }

   txtstream << "\n";

   return txtstream.str();
}


//returns true if file exists, else false
bool fileExists( const char * fileName )
{
   return _access( fileName, 0 ) == 0;
}


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
         fUnicodeForm = XMLString::transcode( toTranscode );
      }

      ~XStr()
      {
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


static void _make_xml( char * spurtxml, char * inputline, char breaktype )
{
   char xmlhead[] = "<?xml version='1.0'?>\n<!DOCTYPE spurt SYSTEM \"spurt.dtd\">\n<spurt spurt_id=\"test\" speaker_id=\"unknown\"><pause><break time=\"0.1\" type=\"#\"/></pause><speech>\n";
   char xmlfoot[] = "</speech><pause><break time=\"0.1\" type=\"#\"/></pause></spurt>\n";
   char * p;

   // replace break type
   p = xmlhead;
   while ( *p )
   {
      if ( *p == '#' )
      {
         *p = breaktype;
      }

      p++;
   }

   p = xmlfoot;

   while ( *p )
   {
      if ( *p == '#' )
      {
         *p = breaktype;
      }

      p++;
   }

   spurtxml[ 0 ] = '\0';
   strcat( spurtxml, xmlhead );
   strcat( spurtxml, inputline );
   strcat( spurtxml, xmlfoot );
}


static int _read_licence( char * licfile, char ** text, char ** signature )
{
   int text_sz = 0;
   int sig_sz = 0;
   int last_line_sz = 0;
   int line_sz = 0;

   FILE * licfp = fopen( licfile, "r" );
   int c = fgetc( licfp );

   while ( !feof( licfp ) )
   {
      line_sz++;

      if ( c == '\n' )
      {
         last_line_sz = line_sz;
         line_sz = 0;
      }

      text_sz++;
      c = fgetc( licfp );
   }

   rewind( licfp );

   if ( !line_sz )
      text_sz = text_sz - last_line_sz;
   else
      text_sz = text_sz - line_sz;

   if ( !line_sz )
      sig_sz = last_line_sz - 1;
   else
      sig_sz = line_sz;

   *text = (char *)malloc( text_sz + 1 );
   fread( *text, sizeof(char), text_sz, licfp );
   (*text)[ text_sz ] = '\0';

   *signature = (char *)malloc( sig_sz + 1 );
   fread( *signature, sizeof(char), sig_sz, licfp );
   (*signature)[sig_sz] = '\0';
   fclose( licfp );

   return 1;
}


void cerevoice_tts::init()
{
   lic_text = EMPTY_STRING;
   signature = EMPTY_STRING;

//   licfile = "..\\..\\data\\cereproc\\voices\\cerevoice_2.0.0_katherine_00009.lic";
//   licfile = "..\\..\\data\\cereproc\\voices\\cerevoice_2.0.0_heather_00009.lic";
   licfile = "..\\..\\data\\cereproc\\voices\\cerevoice_2.0.0_star_00009.lic";

   //make sure license file exists
   if ( !fileExists( licfile ) )
   {
      std::cout<<"Error: license file not found! See README files for setup information. Press any key to exit.\n";
      _getch();
      exit(1);
   }
   _read_licence( licfile, &lic_text, &signature );

//   char * voiceFile = "..\\..\\data\\cereproc\\voices\\cerevoice_2.0.0_katherine_22k.voice";
//   char * voiceFile = "..\\..\\data\\cereproc\\voices\\cerevoice_2.0.0_heather_22k.voice";
   char * voiceFile = "..\\..\\data\\cereproc\\voices\\cerevoice_2.0.0_star_16k.voice";

   //make sure voice file exists
   if(!fileExists(voiceFile))
   {
      std::cout<<"Error: voice file not found! See README files for setup information. Press any key to exit.\n";
      _getch();
      exit(1);
   }

   voice = CPRC_load_voice( voiceFile, lic_text, static_cast<int>( strlen( lic_text ) ), signature, static_cast<int>( strlen( signature ) ) );

   // free license strings (aalocated in read_license)
   free( lic_text );
   free( signature );


   phonemeToViseme[ "sil" ] = "_";  // SIL
   phonemeToViseme[ "aa" ]  = "Ao"; // AA
   phonemeToViseme[ "ae" ]  = "Ih"; // AE
   phonemeToViseme[ "ah" ]  = "Ih"; // AH
   phonemeToViseme[ "ao" ]  = "Ao"; // AO
   phonemeToViseme[ "ax" ]  = "Ih"; // AX
   phonemeToViseme[ "@" ]  = "Ih"; // Shouldn't happen!
   phonemeToViseme[ "aw" ]  = "Ih"; // AW
   phonemeToViseme[ "ay" ]  = "Ih"; // AY
   phonemeToViseme[ "b" ]   = "BMP";//  B
   phonemeToViseme[ "ch" ]  = "j";  // CH
   phonemeToViseme[ "d" ]   = "D";  //  D
   phonemeToViseme[ "dh" ]  = "Th"; // DH
   phonemeToViseme[ "dx" ]  = "D";  // ??
   phonemeToViseme[ "eh" ]  = "Ih"; // EH
   phonemeToViseme[ "er" ]  = "Er"; // ER
   phonemeToViseme[ "ey" ]  = "Ih"; // ?? probably EY
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
   phonemeToViseme[ "T" ]   = "D";  //  T ?
   phonemeToViseme[ "t" ]   = "D";  //  T ?
   phonemeToViseme[ "th" ]  = "Th"; // TH
   phonemeToViseme[ "uh" ]  = "Oh"; // UH
   phonemeToViseme[ "uw" ]  = "Oh"; // UW
   phonemeToViseme[ "v" ]   = "F";  //  V
   phonemeToViseme[ "w" ]   = "OO"; //  W
   phonemeToViseme[ "y" ]   = "OO"; //  Y
   phonemeToViseme[ "z" ]   = "Z";  //  Z
   phonemeToViseme[ "zh" ]  = "J";  // ZH
   //phonemeToViseme[ "i" ]  = "";  // 
   //phonemeToViseme[ "j" ]  = "";  // 
   //possibly need phonemeToViseme entries for "i" and "j"?


   abbfile       = "../../lib/cerevoice/veng_db/en/norm/abb.txt";
   pbreakfile    = "../../lib/cerevoice/veng_db/en/norm/pbreak.txt";
   homographfile = "../../lib/cerevoice/veng_db/en/homographs/rules.dat";
   reductionfile = "../../lib/cerevoice/veng_db/en/reduction/rules.dat";
   rulesfile     = "../../lib/cerevoice/veng_db/en/gb/norm/rules.py";

   if ( !fileExists( abbfile ) )
   {
      std::cout<<"Error: normalization file abb.txt not found! See README files for setup information. Press any key to exit.\n";
      _getch();
      exit(1);
   }

   if ( !fileExists( pbreakfile ) )
   {
      std::cout<<"Error: normalization file pbreak.txt not found! See README files for setup information. Press any key to exit.\n";
      _getch();
      exit(1);
   }

   if ( !fileExists( homographfile ) )
   {
      std::cout<<"Error: normalization file homographsrules.dat not found! See README files for setup information. Press any key to exit.\n";
      _getch();
      exit(1);
   }

   if ( !fileExists( reductionfile ) )
   {
      std::cout<<"Error: normalization file reductionrules.dat not found! See README files for setup information. Press any key to exit.\n";
      _getch();
      exit(1);
   }

   if ( !fileExists( rulesfile ) )
   {
      std::cout<<"Error: normalization file normrules.py not found! See README files for setup information. Press any key to exit.\n";
      _getch();
      exit(1);
   }

   // Initialize the XML4C2 system.
   try
   {
      XMLPlatformUtils::Initialize();
   }
   catch ( const XMLException & toCatch )
   {
      char * pMsg = XMLString::transcode( toCatch.getMessage() );
      XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n" << "  Exception message:" << pMsg;
      XMLString::release( &pMsg );
   }
}


std::string cerevoice_tts::tts( const char * text, const char * file_name )
{
   char * result = "";

   /*
      Create a text normalisation parser
      The parser is configured with a file defining abbreviations to be
      expanded, and a file that defines the break times for particular pauses.
      Both files are easily configurable by the user.
      A homograph rules file is also loaded.

      The parser is cleared between input documents to allow parsing of
      multiple files.
   */

   // Create a reference to the normaliser, which needs to be passed in to all normaliser functions
   int norm_id = Normaliser_create( elNEWID );
   Normaliser_set_abbreviations( norm_id, abbfile );
   Normaliser_set_pbreaks( norm_id, pbreakfile );
   Normaliser_set_rules( norm_id, rulesfile );
   Normaliser_set_homographs( norm_id, homographfile );
   Normaliser_set_reductions( norm_id, reductionfile );

   // file to record database coverage data
   //char * logfile = EMPTY_STRING;
   //char * covfile = EMPTY_STRING;
   //char * lexfile = EMPTY_STRING;

   /*
   voice = CPRC_load_voice( "..\\..\\..\\util\\cerevoice_2.0.0_heather_22k.voice", 
               lic_text, strlen(lic_text), 
               signature, strlen(signature) );
   */

   if ( !voice )
   {
      fprintf( stderr, "ERROR: voice load failed, check license integrity\n" );
      exit( 1 );
   }

   /* set up a spurt, an audio buffer for data, and a lexicon search structure */

   CPRC_spurt * spurt = CPRC_spurt_new( voice );
//   CPRC_abuf * abuf = CPRC_abuf_new( 22050 );
   CPRC_abuf * abuf = CPRC_abuf_new( 16000 );
   CPRC_lts_search * ltssrch = CPRC_lts_search_new( 0 );
   CPRC_lexicon_search * lxsrch = CPRC_lexicon_search_new();

   /* Feed in input text, further data is to come */
   Normaliser_parse( norm_id, const_cast<char*>( removeXMLTagsAndNewLines( text ).c_str() ), 0 );
   Normaliser_parse( norm_id, "", 1 );

   int numspts = Normaliser_get_num_spurts( norm_id );

   if ( numspts )
   {
      int sptcount = 0;
      for ( int n = 0; n < numspts; n++ )
      {
         CPRCPMOD_spurt_synth( Normaliser_get_spurt( norm_id, n ), spurt, lxsrch, ltssrch, abuf );
         sptcount++;
      }
   }

   /* Reset the parser.  This has to be done between input documents
   or the xml will be invalid and the parse will fail.
   */
   Normaliser_reset_parser( norm_id );

   /* synthesise */

   /* Reused: spurt, abuf, ltssrch
   New each synthesis: lxsrch
   Pitch modification is performed, use CPRC_spurt_synth
   to avoid using the pitch modification library
   */

   // make the output file name from the input file less the extension, and the output dir
   //std::string fpathout( "sample.wav" );
   CPRC_riff_save( abuf, file_name );

   // Watch for special case help request
   int errorCode = 0;


   DOMImplementation * impl =  DOMImplementationRegistry::getDOMImplementation( X( "Core" ) );

   if ( impl != NULL )
   {
      try
      {
         //XMLCh * end = XMLString::transcode( "end" );
         //XMLCh * start = XMLString::transcode( "start" );
         XMLCh * name = XMLString::transcode( "name" );
         XMLCh * file_path = XMLString::transcode( file_name );

         xercesc_2_7::DOMDocument* doc = impl->createDocument(
            0,                    // root element namespace URI.
            X( "speak" ),         // root element name
            0 );                  // document type object (DTD).

         DOMElement * rootElem = doc->getDocumentElement();

         DOMElement * soundFileElement = doc->createElement( X( "soundFile" ) );
         soundFileElement->setAttribute( name, file_path );
         rootElem->appendChild( soundFileElement );

         DOMElement * wordElement = doc->createElement( X( "word" ) );

         int num_words = 0;

         for ( int i = 0; i < abuf->trans_sz; i++ )
         {
            if ( abuf->trans[ i ].type == CPRC_ABUF_TRANS_PHONE )
            {
              std::map<std::string, std::string>::iterator iter = phonemeToViseme.find( abuf->trans[ i ].name );

              //check added to avoid crashing on entries which are not defined
              if ( iter != phonemeToViseme.end() )
              {

                 XMLCh * start = XMLString::transcode( "start" );
                 XMLCh * end = XMLString::transcode( "end" );
                 XMLCh * type = XMLString::transcode( "type" );
                 XMLCh * phone_type = XMLString::transcode( iter->second.c_str() );

                 std::string end_f = vhcl::Format( "%0.6f", abuf->trans[i].end );
                 XMLCh * end_time = XMLString::transcode( end_f.c_str() );

                 std::string start_f = vhcl::Format( "%0.6f", abuf->trans[i].start );
                 XMLCh * start_time = XMLString::transcode( start_f.c_str() );

                 DOMElement * visemeElement = doc->createElement( X( "viseme" ) );
                 visemeElement->setAttribute( start, start_time );
                 visemeElement->setAttribute( type, phone_type );

                 if ( strcmp( abuf->trans[ i ].name, "sil" ) == 0 )
                 {
                   rootElem->appendChild( visemeElement );
                 }
                 else
                 {
                   wordElement->appendChild( visemeElement );
                 }

                 if ( i < ( abuf->trans_sz - 1 ) )
                 {
                   if ( ( abuf->trans[ i + 1 ].type == CPRC_ABUF_TRANS_WORD ) || ( abuf->trans[ i + 1 ].type == CPRC_ABUF_TRANS_MARK ) //) {
                     || ( strcmp( abuf->trans[ i + 1 ].name, "sil" ) == 0 ) )
                   {
                     wordElement->setAttribute( end, end_time );

                     if ( wordElement->hasChildNodes() )
                        rootElem->appendChild( wordElement );

                     wordElement = doc->createElement( X( "word" ) );

                     std::string word_start_f = vhcl::Format( "%0.6f", abuf->trans[ i + 1 ].start );
                     XMLCh * word_start_time = XMLString::transcode( word_start_f.c_str() );
                     wordElement->setAttribute( start, word_start_time );

                     //float word_start = abuf->trans[ i + 1 ].start;
                     num_words++;
                   }
                 }

              }
              else
              {
                printf( "COULDN'T FIND MATCH FOR: %s\n", abuf->trans[ i ].name );
              }
            }
         }

         DOMWriter * theSerializer = ((DOMImplementationLS *)impl)->createDOMWriter();
         XMLCh * xml_result = theSerializer->writeToString( *rootElem );
         result = XMLString::transcode( xml_result );
         theSerializer->release();
      }
      catch ( const OutOfMemoryException & )
      {
         XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
         errorCode = 5; 
      }
      catch ( const DOMException & e )
      {
         XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
         errorCode = 2;
      }
      catch ( const XMLException & toCatch )
      {
         printf( "XMLException occurred: %s", toCatch.getCode() );
         errorCode = 4;
      }
      catch (...)
      {
         XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
         errorCode = 3;
      }
   }

   //int speech_sz = 0;
   //speech_sz += abuf->wav_sz;
   CPRC_lts_search_reset( ltssrch );
   CPRC_spurt_clear( spurt );
   //int fno = 0;
   //fno += 1;

   return std::string( result );
}
