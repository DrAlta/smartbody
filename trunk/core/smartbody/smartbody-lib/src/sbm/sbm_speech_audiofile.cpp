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

#include "sbm_speech_audiofile.hpp"
#include "xercesc_utils.hpp"
#include "sr_hash_map.h"
#include "sr_arg_buff.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sstream>
#include <float.h>
#include "time.h" 
#include <fstream>

#include "xercesc_utils.hpp"


const char* VISEME_MAP = "../audiofile_viseme.map";  // TODO make this a configurable variable


using namespace std;
using namespace SmartBody;

//static variables
//  TODO: No need to be static.  Include these in AudioFileSpeech class
srHashMap<std::string> AudioFileSpeech::audioSoundLookUp;
srHashMap<std::string> AudioFileSpeech::audioVisemeLookUp;
srHashMap<std::string> AudioFileSpeech::textOfUtterance;
int                    AudioFileSpeech::audioMsgNumber;

// Constructor / Destructor
AudioFileSpeech::AudioFileSpeech() {
	// TODO when fields are added
}

AudioFileSpeech::~AudioFileSpeech() {
	// TODO when fields are added
}


//  Override SpeechInterface methods (see sbm_speech.hpp)

/**
 *  Requests audio for a speech by agentName as specified in XML node.
 *  If node is a DOMElement, use the child nodes.
 *  If node is a text node, is the value a string.
 *  If node is an attribute, use the attribute value as a string.
 *  Returns a unique request identifier.
 */
RequestId AudioFileSpeech::requestSpeechAudio( const char* agentName, const DOMNode* node, const char* callbackCmd ) {
	// TODO
	//TODO: Test this function with a variety of XML documents
	string encoding= XMLString::transcode(node->getOwnerDocument()->getEncoding()); //XMLStringconverts XML to cString; encoding
	string version= XMLString::transcode(node->getOwnerDocument()->getVersion ()); //the xml version number
	string xmlConverted="<?xml version=\"" + version.substr(0,6)+ "\" "; 
	xmlConverted= xmlConverted+ "encoding=\"" + encoding.substr(0,7) + "\"?>";
	xml_utils::xmlToString(node, xmlConverted); //Xml to string recursively searches DOM tree and returns a string of the xml document
	
/*	if(xmlConverted=="BADSTRING"){ 
		printf( "remote_speech::RvoiceXmlCmd ERR: Invalid DOMNode type encountered"); 
		return(CMD_FAILURE);
	}*/
	//converts string to char*
	char* text= new char[xmlConverted.length()+1];
	strcpy(text, xmlConverted.c_str()); 
	RequestId ret= requestSpeechAudio(agentName, text, callbackCmd );
	//delete [] text; text=0;
	return (ret); //string is converted to char* and sent to other request audio fcn
}


/**
 *  Requests audio for a speech char[] text by agentName.
 *  Returns a unique request identifier.
 */
RequestId AudioFileSpeech::requestSpeechAudio( const char* agentName, const char* text, const char* callbackCmd ) {
	
	
	//cout<<"the Text: "<<endl<<text<<endl;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	audioMsgNumber++; //to make the message number unique it must not belong to any single object instantiation and thus resides in "lookup" along with other items that must be globally accessable among all AudioFileSpeech objects
	ostringstream myStream; //creates an ostringstream object
	myStream << audioMsgNumber << flush; //outputs the number into the string stream and then flushes the buffer
	//THis code is meant to replace the <tm id's> of generic BML type "text/plain" with "marks" that can be understood by remote speech process
	string textOfUtt = text;
	int keyWordIndex=0; int keyWordIndexEnd=0;
	keyWordIndex= textOfUtt.find("ref="); //should make this the element number of first element ie the '<'
	keyWordIndex+=5;
	


	string result;
	
	if(keyWordIndex!= string::npos)
		{
			keyWordIndexEnd=textOfUtt.find("\"",keyWordIndex);
			//cout<<"Text of Utt "<<textOfUtt.substr(keyWordIndex,keyWordIndexEnd-keyWordIndex);
			
			//if agent is not spawned in the level the character will not exist
			SbmCharacter* agent = mcu.character_map.lookup(agentName);
			if( agent == NULL)
			{
				printf( "AudioFileSpeech::requestSpeechAudio ERR: insert AudioFile voice code lookup FAILED, msgId=%s\n", agentName ); 
				return (NULL);
			}

			//the audio path is the .wav; the viseme file is .ltf (created by impersonator
			char fullAudioPath[ _MAX_PATH ];
			string relativeAudioPath = (string)"../../../../" + agent->get_voice_code();
			if ( _fullpath( fullAudioPath, relativeAudioPath.c_str(), _MAX_PATH ) == NULL )
			{
				printf( "AudioFileSpeech::requestSpeechAudio ERR: _fullpath() returned NULL" );
				return NULL;
			}

			string audioPath;
			audioPath += (string)fullAudioPath+"/"+textOfUtt.substr(keyWordIndex,keyWordIndexEnd-keyWordIndex)+".wav";
cout << "audioPath = \""<<audioPath<<"\""<<endl;
			string theKey= textOfUtt.substr(keyWordIndex,keyWordIndexEnd-keyWordIndex);
			string visemePath;
			visemePath += "../../../../"+agent->get_voice_code()+"/"+textOfUtt.substr(keyWordIndex,keyWordIndexEnd-keyWordIndex)+".ltf";
cout << "visemePath = \""<<visemePath<<"\""<<endl;
			string* audioPathPtr= new string(audioPath);
			string* visemePathPtr= new string(visemePath);
			audioSoundLookUp.insert(myStream.str().c_str(), audioPathPtr);
			audioVisemeLookUp.insert(myStream.str().c_str(), visemePathPtr);
			ifstream checkVis;
			checkVis.open(visemePath.c_str());
			if(!checkVis.is_open())
			{
				result= "ERROR from AudioFileSpeech::requestSpeechAudio: File: " +visemePath+ " DOES NOT EXIST";
				cout<< "ERROR from AudioFileSpeech::requestSpeechAudio: File: "+visemePath+ " DOES NOT EXIST";
			}
			else
			{
				result = "SUCCESS";
			}
		}
		else
		{
			result = "ERROR from AudioFileSpeech::requestSpeechAudio: ref= attribute could not be found in speech behavior";
		}
	
	ostringstream msg;
	msg << callbackCmd << " " << agentName << " " << myStream.str() << " " << result;
	mcu.execute_later( msg.str().c_str() );
	return audioMsgNumber; // increment per request
}

string AudioFileSpeech::justTheText(string toConvert)
{
	//cout<<endl<<"In To convert"<<endl;
	int runOutOfText=0;
	int markNumber=0;
	int endIndex=toConvert.rfind("</speech>");
	string finalSentence="";
	string intermediate="";

	while(runOutOfText!=-1)
	{
		ostringstream num;
		ostringstream numPlusOne;
		//cout<<endl<<"the Mark Number "<<markNumber<<endl;
		num<<markNumber<<flush;
		numPlusOne<<(markNumber+1)<<flush;
		string toFind="";
		toFind+="T"+num.str()+"\"></mark>";
		string toFindPlusOne="";
		toFindPlusOne+="T"+numPlusOne.str()+"\"></mark>";
		//cout<<endl<<"the mark number and the next one: "<<toFind<<" "<<toFindPlusOne<<endl;
		int indexMarker= toConvert.find(toFind); //gives us the start of the T stuff
		runOutOfText= toConvert.find(toFindPlusOne,indexMarker);
		if(runOutOfText!=-1)
		{
			//cout<<endl<<"here is the if "<<toConvert.substr(indexMarker+toFind.length(), runOutOfText-(indexMarker+toFind.length()-1)-19)+" "<<endl;
			finalSentence+=toConvert.substr(indexMarker+toFind.length(), runOutOfText-(indexMarker+toFind.length()-1)-19)+" ";
		}
		markNumber+=2;
	}
	//cout<<endl<<"THIS IS TO CONVERT: "<<finalSentence<<endl;
	
return finalSentence;
}


/**
 *  If the request has been processed, returns the time ordered vector 
 *  of VisemeData for the requestId.  Otherwise return NULL.
 *
 *  Visemes in this list are actually morph targets, and multiple
 *  visemes with different weights can be added together.  Because of
 *  this, the returned viseme list should include zero weighted
 *  VisemeData instances of when to cancel previous visemes (change
 *  of viseme, and end of words).
 */
const std::vector<VisemeData *>* AudioFileSpeech::getVisemes( RequestId requestId ) {
	
	/**
	*Basically the viseme data needs 2 files to work-- The .ltf file and the Impersonator "doctor.map" file
	*The LTF file gives you the Phonemes; Then you use the doctor.map file for that Phoneme to figure out which Viseme
	*(see phon2Vis below) the phoneme  maps to
	*/
	
	//phoneme-viseme map Impersonator
	string phon2Vis[]= {"EE","Er","Ih","Ao","oh","OO","Z","j","F","Th","D","BMP","NG","R","KG"};
	ostringstream newStream; //creates an ostringstream object
	newStream << requestId << flush; //outputs the number into the string stream and then flushes the buffer
	string requestIdStr( newStream.str() );
	
	vector<VisemeData *> *visemeVector= new vector<VisemeData *>;

	ifstream visemeMap_inStream;
	ifstream utteranceVisemes_inStream;
	string* lookup_result = audioVisemeLookUp.lookup(requestIdStr.c_str());
	if( lookup_result == NULL ) {
		// unknown requestId in viseme table
		return NULL;
	}
	string utteranceVisemesPath=*lookup_result;
	utteranceVisemes_inStream.open(utteranceVisemesPath.c_str());
	
	//if the viseem file can't be opened
	if(!utteranceVisemes_inStream.is_open())
	{
		cout<<endl<<"AudioFileSpeech::getVisemes ERR: speech Request# "<<newStream.str().c_str()<<" "<<utteranceVisemesPath<<" could not be open"<<endl;
		utteranceVisemes_inStream.close();
		return NULL;
	}
	
	string readIn;
	//reads each viseme line; assumes it's less than 100 characters (this is the case for impersonator files)
	char* charReadIn= new char[100];
	bool visemeEncountered=false;
	for(int u=0; u<100; u++)
	{
		charReadIn[u]=0;
	}
	utteranceVisemes_inStream.getline(charReadIn,100,'\n');
	readIn= charReadIn;
	while(readIn.find("Curve Data")==-1)
	{

		if(visemeEncountered && readIn.length()!=0)
		{
			visemeMap_inStream.open(VISEME_MAP);
			if(!visemeMap_inStream.is_open())
			{
				cout<<endl<<"AudioFileSpeech::getVisemes ERR: speech Request# "<<newStream.str().c_str()<<VISEME_MAP<<" could not be open"<<endl;
				return NULL;
			}

			//it's a viseme!!
			int index1=0; int index2=0;
			index1=readIn.find(" ");
			index2=readIn.rfind(" ");
			VisemeData *singleViseme=NULL;

			//reads in start time, end time and the viseme number to map
			int visemeCount = atoi(readIn.substr(0,index1).c_str()); 
			float startTime = (float)atof(readIn.substr(index1+1,index2-1).c_str());
			float endTime   = (float)atof(readIn.substr(index2+1, readIn.length()-1).c_str());

			
			char* dummy= new char[100];

			//looks at the map file and tries to figure out which viseme is the 
			for(int k=0; k<visemeCount+8;k++)
			{
				visemeMap_inStream.getline(dummy,100);
				
			}
			delete[] dummy;
			char* visemeLineChar=new char[100];
			for(int t=0; t<100; t++)
			{
				visemeLineChar[t]=0;
			}

			visemeMap_inStream.getline(visemeLineChar,100);
		
			string visemeLine=""; 
			visemeLine+=visemeLineChar;
			
			int visemeIndex= visemeLine.find("=");
		
			string visemeString= visemeLine.substr(visemeIndex+1, visemeLine.length()-1);
			string tempString=visemeString;
			int visemeIndexCount=-1;
			int nextViseme=0;
			string compareString="";
			for (int w=0; w<15; w++)
			{
				compareString= tempString.substr(w*6, 4);
				if(nextViseme<atof(compareString.c_str()))
				{
					nextViseme = atoi(tempString.c_str());
					visemeIndexCount=w;
				}
			}

			string visemeId;

			//if all the visemes are zero it's the closed mouth viseme, otherwise it's one of the above visemes
			if(visemeIndexCount!=-1)
			{
			//	cout<<endl<<"PHON2VIS "<<phon2Vis[visemeIndexCount].c_str();
				visemeId= phon2Vis[visemeIndexCount];
			}
			else
			{
				visemeId="_";
			}
			//strcpy(visemeId, visemeString.c_str());
			//cout<<"VISEME ID: "<<visemeId<<" Start Time: "<<startTime<<endl;
			visemeLine="";
			singleViseme= new VisemeData(visemeId.c_str(), 1.0, startTime);
			visemeVector->push_back(singleViseme);
			VisemeData* resetViseme= new VisemeData(visemeId.c_str(),0.0,endTime);
			visemeVector->push_back(resetViseme);
			
			delete [] visemeLineChar;
			
			visemeMap_inStream.close();

		}
		if(readIn.find("// Phoneme Timing List")!=-1)
		{
			visemeEncountered=true;
		}
			for(int p=0; p<100; p++)
		{
			charReadIn[p]=0;
		}
		utteranceVisemes_inStream.getline(charReadIn,100,'\n');
		readIn= charReadIn;
		
	}
	
	utteranceVisemes_inStream.close();
	return visemeVector;
}
/**
 *  Returns the sbm command used to play the speech audio.
 */
char* AudioFileSpeech::getSpeechPlayCommand( RequestId requestId ) 
{
	ostringstream newStream; //creates an ostringstream object
	newStream << requestId << flush; //outputs the number into the string stream and then flushes the buffer


	if(!audioSoundLookUp.does_key_exist(newStream.str().c_str()))
	{
		return(NULL);
	}
	string soundFile= *(audioSoundLookUp.lookup(newStream.str().c_str()));
	soundFile= "send PlaySound "+ soundFile; //concatenates audio path with playsound command
	char* retSoundFile= new char[soundFile.length() + 1];
	strcpy(retSoundFile, soundFile.c_str());
	return (retSoundFile);
}

/**
 *  Returns the sbm command used to stop the speech audio.
 */
char* AudioFileSpeech::getSpeechStopCommand( RequestId requestId ) {
	ostringstream stopStream; //creates an ostringstream object
	stopStream << requestId << flush; //outputs the number into the string stream and then flushes the buffer
	
	if(!audioSoundLookUp.does_key_exist(stopStream.str().c_str()))
	{
		return(NULL);
	}
	
	string soundFile= *(audioSoundLookUp.lookup(stopStream.str().c_str()));
	soundFile= "send StopSound "+ soundFile + " 3"; //The 3 denotes the channel in the VR theatre that the sound corresponds to
	char* retSoundFile= new char[soundFile.length() + 1];
	strcpy(retSoundFile, soundFile.c_str());
	return (retSoundFile);
}

/**
 *  Returns the filename of the audio.
 */
char* AudioFileSpeech::getSpeechAudioFilename( RequestId requestId ) {
	
	ostringstream newStream; //creates an ostringstream object
	
	newStream << requestId << flush; //outputs the number into the string stream and then flushes the buffer
	
	if(!audioSoundLookUp.does_key_exist(newStream.str().c_str()))
	{
		return(NULL);
	}
	string soundFile    = *(audioSoundLookUp.lookup(newStream.str().c_str()));
	string::size_type nameIndex = soundFile.rfind("/");
	char* returnName= new char[soundFile.length()-nameIndex];
	for(string::size_type l=0; l<soundFile.length()-nameIndex;l++)
	{
		returnName[l]=0;
	}
	for (string::size_type i=0; i<soundFile.length()-nameIndex; i++)
	{
		returnName[i]=soundFile[i+nameIndex+1];
	}
	return (returnName);
}

/**
 *  Returns the timing for a synthesis bookmark,
 *  or -1 if the markId is not recognized.
 */
//Only need the request ID for this. It assumes a file called "Utterance Map" which hasthe following form

//<utterance="I hope your Arabic is just very poor, otherwise that insult is almost too much to bear"
//audio  src="arabic_poor_insult.wav" />

//THE NEW LINE between utterance and Audio Is important/ Part of the yet to be written specification for the utterance.xml file
float AudioFileSpeech::getMarkTime( RequestId requestId, const XMLCh* markId ) 
{
	/**
	* This estimates the visemes for each word in an utterance using the utterance.map file and audioFileSpeech::wordscore function.
	* This estimation is used to create a proportion of the total visemes for each word (ie: word number 1 will get
	* 2% if the total visemes etc.) Then the synch points for wordbreaks are figured out by the time of the final viseme.
	*  The Wordscore is the estimation of the # visemes a given word has. Each word's viseme over the total wordscore across every word, multiplied by the total actual
	* number of Visemes in the sentence yeilds the number of Visemes for that word.
	* --->EXAMPLE:::If you need to find <T6> you need to know the time of the last viseme
	*	for word #3. so you add up the visemes for the first word, the second word, and the third word; and then find that numbered viseme,
	*	take it's time information, and return it
	*/
	ostringstream id; //creates an ostringstream object
	//cout<<"THE MARK ID "<<XMLString::transcode(markId)<<endl;
	
	id << requestId << flush; //outputs the number into the string stream and then flushes the buffer
	//ifstream utteranceMap;
	ifstream numberOfVisemes;
	string visemePath= audioVisemeLookUp.lookup(id.str().c_str())->c_str();
	//int split= visemePath.rfind("/");
	//string splitPath;
	//splitPath= visemePath.substr(0,split);
	//splitPath+="/utteranceMap.xml";
	//utteranceMap.open(splitPath.c_str());
	numberOfVisemes.open(visemePath.c_str());
	string findString="";
	int visemeCount=0;
	if(numberOfVisemes.is_open())
	{
		bool foundNumber=false;
		while(!foundNumber)
		{
			getline(numberOfVisemes,findString);
			if(findString.find("Num. Phonemes")!=-1)
			{
				foundNumber=true;
			}
		}
		
		getline(numberOfVisemes,findString);
		visemeCount= atoi(findString.c_str());
		
		bool foundLine=false;
		
		string soundName=audioSoundLookUp.lookup(id.str().c_str())->c_str();
		string tempString;
		int split= soundName.rfind("/");
		soundName=soundName.substr(split+1,soundName.length()-1);
		
		/*while(!foundLine)
		{
			tempString=findString;
			getline(utteranceMap,findString);
			if(findString.find(soundName) != -1)
			{
				foundLine=true;
			}
		
		}
		
		int startIndex=0;
		int endIndex=0;

		startIndex=tempString.find("\"");
		endIndex=tempString.rfind("\"");

		tempString=tempString.substr(startIndex+1,endIndex-12);*/
		string* lookup_result = textOfUtterance.lookup(id.str().c_str());
		if( lookup_result==NULL ) {
			// Unknown markId
			return -1;
		}
		tempString = *lookup_result;
		//cout<<endl<<"********tempString "<<tempString<<endl;
		string testString=tempString;
		bool noWords=false;
		int numWords=0;
		int indexWords=0;
		while(!noWords)
		{
			indexWords= testString.find(" ");
			if(indexWords != -1)
			{
				numWords++;
				testString=testString.substr(indexWords+1);
			}
			else
			{
				noWords=true;
			}

		}
		numWords++;
		
		indexWords=0;
		testString=tempString;
		int oldIndex=0;
		int* wordScores;
		wordScores=new int [numWords];
		string theWord;

		if(numWords==1)
		{
			wordScores[0]=1;
		}
		else
		{
			testString+=" ";
			for(int u=0; u<numWords; u++)
			{
				oldIndex=indexWords;
				wordScores[u]=0;
				if(testString.find(" ") !=-1)
				{
					indexWords+= testString.find(" ");
				
				//enumerates the word scores for each workd
				

					//theWord=testString.substr(oldIndex,indexWords-oldIndex);
					theWord=testString.substr(0,indexWords-oldIndex);
					
					wordScores[u]=wordScore(theWord);
					testString=tempString.substr(indexWords+u+1,tempString.length()+ 500);
					//cout<<endl<<"word scores "<<wordScores[u]<<endl;
				}
			}
		}
	
		string markName=XMLString::transcode(markId);
		//cout<<endl<<endl<<"markName "<<markName<<endl<<endl<<endl;
		markName=markName.substr(5,markName.length());
		int markWord= atoi(markName.c_str());
		markWord= ((markWord+1)/2);
		
		int total=0;
		//calculates the total word score over every word
		for(int q=0; q<numWords; q++)
		{
			//cout<<"WORD SCORES "<<q<<" "<<wordScores[q]<<endl;
			total+=wordScores[q];
		}

		float visemeAllotment=0;
		
		//figures out the proportion of visemes each word should get, multiplies this by the actual # of visemes to get the actual visemes each word should get
		//this added up to the current word's synch point we want so we can go to that viseme in the list and get it's time
		for(int e=0; e<=markWord;e++)
		{
			visemeAllotment+=float(float(visemeCount)*float((float(wordScores[e])/float(total))));
		}
		//cout<<"visemeAllotment "<<visemeAllotment<<" Mark Name "<<markWord<<endl;
		
		visemeAllotment= ceil(visemeAllotment);
		//cout<<"visemeAllotmentCeil "<<visemeAllotment<<" Mark Name "<<markWord<<endl<<endl<<endl;
		
		//of we divy out more visemes than we have we just make it equal to the total number of visemes
		if(visemeAllotment>visemeCount || -visemeAllotment>visemeCount)
		{
			visemeAllotment=(float)visemeCount;
		}
		//cout<<"visemeAllotmentCeil "<<visemeAllotment<<" Mark Name "<<markWord<<endl<<endl<<endl;
		
		//WARNING-- CHANGE THIS-- It assumes that the firest viseme starts on line 17 always (for the most part in these
		//Impersonator files it does and this is an impersonator implementation
		string endTime="";
		
		for(int a=0; a<visemeAllotment+3; a++)
		{
			getline(numberOfVisemes,endTime);
		}
		//cout<<endl<<"Original end string "<<endTime<<endl;
		int endingIndex=endTime.rfind(" ");
		//cout<<endl<<"endingIndex "<<endingIndex<<"length "<<endTime.length();
		endTime=endTime.substr(endingIndex+1,endTime.length()-endingIndex-1);
		//cout<<endl<<"Mark Time: "<<markName<<" Word Number: "<<markWord<<" END TIME: "<<endTime<<endl;
		if(atof(endTime.c_str())<0)
		{
			return 0.001f;
		}
		return (float)atof(endTime.c_str());
	}
	else
	{
		cout<<endl<<"AudioFileSpeech::getMarkTime ERR: could not Open .lst file-- speech Request# "<<requestId<<endl;
		return -1.0;
	}
	return -1.0;
}

int AudioFileSpeech::wordScore(string word)
{
	//calculates viseme score for word
	//1 pt for any combination of 2 letters EXCEPT- the same letter or an e at the end
	int scoreBoard=0;
	if(word.length()<=0)
	{
		return 0;
	}
	for(string::size_type x=0; x<word.length()-1;x++)
	{
		if(x+1<word.length())
		{
		if(word.at(x)!= word.at(x+1) && (x+1)<word.length())
		{
			scoreBoard++;
		}
		if(word.at(x)!= word.at(x+1) && (x+1)==word.length())
		{
			if(word.at(x+1)!='e')
			{
				scoreBoard++;
			}
		}
		}
	}
	if(scoreBoard==0){scoreBoard++;}
	return scoreBoard;
}

/**
 *  Signals that requestId processing is complete and its data can be 
 *  discarded.
 */
void AudioFileSpeech::requestComplete( RequestId requestId ) {
	// TODO
}



//MISSNAMED-- this is actually just a test function to see if everything is working
//not so much an init function-- but it is under the test commands so it's all good
int init_audio_func(srArgBuffer& args, mcuCBHandle* mcu_p)
{
	/*
		Initializes minor character sounds
	*/
	
	try{
		//if( LOG_RHETORIC_SPEECH ) printf("\n \n *************The AudioFileSpeech_test***************** \n \n");
		char* x= "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><act><participant id=\"doctor\" role=\"actor\"/><bml><speech id=\"sp1\" ref=\"off-top-ask-else\" type=\"application/ssml+xml\"> Something <mark name=\"hello\"/></speech></bml></act>";
		//cout<<endl<<x<<endl;
		//char* x= "<?xml version=\"1.0\" encoding=\"UTF-8\"?><speak> Something <mark name=\"hello\"/> to <mark name=\"mark\"/> say <mark name= \"wtf\"/>  </speak>";
		XercesDOMParser *xmlParser;
		xmlParser = new XercesDOMParser();
		xmlParser->setErrorHandler( new HandlerBase() );
		DOMDocument *xmlDoc = xml_utils::parseMessageXml( xmlParser, x);
		DOMNode *funNode= xmlDoc->getDocumentElement();
		AudioFileSpeech anchor;
		char *tre= "doctor";
		char *command= "AudioProcessFinished";
		int id= anchor.requestSpeechAudio(tre, funNode, command);
		//cout<<endl<<"The Play Command "<<anchor.getSpeechPlayCommand(id)<<endl;
		//cout<<endl<<"The FileName "<<anchor.getSpeechAudioFilename(id)<<endl;
		anchor.getVisemes(id);
		const XMLCh* rock= L"T2";
		anchor.getMarkTime(id, rock);

	
		return(CMD_SUCCESS);
	
	} catch( const exception& e ) {
		cerr << "AudioFileSpeech: std::exception: "<<e.what()<< endl;
		return CMD_FAILURE;
	}


	
}
