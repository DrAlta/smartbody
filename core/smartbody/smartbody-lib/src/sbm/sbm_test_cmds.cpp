/*
 *  sbm_test_cmds.cpp - part of SmartBody-lib
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

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "sbm_test_cmds.hpp"
#include "bml/bml.hpp"  // for #define USE_RECIPIENT
#include <controllers/me_ct_scheduler2.h>

using namespace std;


int sbm_set_test_func( srArgBuffer& args, mcuCBHandle *mcu  ) {
	string arg = args.read_token();
	if( arg=="char" || arg=="character" ) {
		mcu->test_character_default = args.read_token();

		if( mcu->test_character_default.length() > 0 ) {
			if (mcu->getNumCharacters() > 0)
			{
				LOG("WARNING: Unknown default test character: \"%s\"", mcu->test_character_default.c_str());
			} else {
				LOG("Default test character: \"%s\"", mcu->test_character_default.c_str());
			}
		}
		return CMD_SUCCESS;
	} else if( arg=="recip" || arg=="recipient" ) {
		mcu->test_recipient_default = args.read_token();

		if( mcu->test_recipient_default.length() > 0 ) {
			if (mcu->getNumCharacters() > 0)
			{
				LOG("WARNING: Unknown default test recipient: \"%s\"", mcu->test_recipient_default.c_str());
			} else {
				LOG("Default test recipient: \"%s\"", mcu->test_recipient_default.c_str());
			}
		}
		return CMD_SUCCESS;
	} else {
		LOG("ERROR: Unrecogized test variable \"%s\".", arg.c_str());
		return CMD_NOT_FOUND;
	}
}


int sbm_print_test_func( srArgBuffer& args, mcuCBHandle *mcu_p  ) {
	string arg = args.read_token();
	if( arg=="char" || arg=="character" ) {
		LOG("Default test character: \"%s\"", mcu_p->test_character_default.c_str() );
		return CMD_SUCCESS;
	} else {
		LOG("ERROR: Unrecogized test variable \"%s\"", arg.c_str() );
		return CMD_NOT_FOUND;
	}
}




int test_args_func( srArgBuffer& args, mcuCBHandle *mcu_p  ) {
	int count = args.calc_num_tokens();
	std::stringstream strstr;
	strstr << "TEST ARGS: "<< count << " arguments";
	if( count ) {
		strstr << ": { \""<< args.read_token();

		string token( args.read_token() );
		while( token.length() ) {
			strstr << "\", \"" << token;
			token = args.read_token();
		}
		strstr << "\" }";
	}
	LOG("%s", strstr.str().c_str());


	////int count = args.calc_num_quotes();
	////char *A = args.read_quote();
	////char *B = args.read_quote();
	////char *C = args.read_quote();
	//int count = args.calc_num_tokens();

	//char *A = args.read_token();
	//char *B = args.read_token();
	//char *C = args.read_token();
	//LOG( "TEST: [%d]:{ '%s', '%s', '%s' }\n", count,A,B,C );
	return( CMD_SUCCESS );
}


/**
 *  Normalizes a char_id string.
 *  IValid ids:
 *  - a known SbmCharacter name
 *  - an unknown character name (while printing a warning)
 *  - "*" (meaning, perform test on all known characters)
 *
 *  Returns true if valid.
 */
bool normalize_character_id( const string& module, const string& role, const string& char_id ) {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	if( char_id.length()==0 ) {
		std::stringstream strstr;
		strstr << "ERROR: "<<module<<": No "<<role<<" specified.";
		LOG(strstr.str().c_str());

		return false;
	} else {
		// Lookup character
		if( char_id!="*" ) {
			SbmCharacter* character = mcu.getCharacter( char_id );
			if( character == NULL ) {
				std::stringstream strstr;
				strstr << "WARNING: "<<module<<": Unknown "<<role<<" id \""<<char_id<<"\".";
				LOG(strstr.str().c_str());
			}
		}
	}
	return true;
}


/**
 *  Reads the following options from args:
 *  - char[acter] <character id>
 *  - seq <sequence id>
 *  - echo
 *  - noecho
 *  in any order (but last option take priority in conflict)
 */
bool read_options( const string& module, srArgBuffer& args, string& arg,
                   string& char_id, string& recip_id,
				   string& seq_id, bool& echo, bool& send ) {
	bool did_set_char = false;
	bool did_set_recip = false;
	while( arg.length() ) {
		if( arg == "char" || arg == "character" ) {
			did_set_char = true;
			char_id = args.read_token();
		} else if( arg == "recipient" || arg == "recip" ) {
			did_set_recip = true;
			recip_id = args.read_token();
		} else if( arg=="echo" ) {
			echo = true;
		} else if( arg=="noecho" ) {
			echo = false;
		} else if( arg=="send" ) {
			send = true;
		} else if( arg=="nosend" ) {
			send = false;
		} else if( arg=="seq" ) {
			seq_id = args.read_token();
		} else {
			break;
		}
		arg = args.read_token();
	}
	
	if( did_set_char ) {
		if( !normalize_character_id( module, "actor", char_id ) )
			return false;
	}
	if( did_set_recip ) {
		if( !normalize_character_id( module, "recipient", recip_id ) )
			return false;
	}

	return true;
}


/**
 *  Sets the contents of buffer to the cmd (usually vrSpeak or vrExpress) command for the given character & BML.
 */
void build_vrX( ostringstream& buffer, const string& cmd, const string& char_id, const string& recip_id, const string& content, bool for_seq ) {
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	buffer.str("");
	if( for_seq )
		buffer << "send " << cmd << " ";
	buffer << char_id << " "<< recip_id << " sbm";
	if( mcu.process_id != "" )  // Insert process_id if present.
		buffer << '_' << mcu.process_id; 
	buffer << "_test_bml_" << (++mcu.testBMLId) << endl << content;
}

/**
 *  Executes the BML as a vrSpeak
 *
 *  If seq_id has length>0, then a sequence is generated,
 *  including echoing output.
 */
int send_vrX( const char* cmd, const string& char_id, const string& recip_id,
			  const string& seq_id, bool echo, bool send, const string& bml ) {
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	ostringstream msg;

	bool all_characters = ( char_id=="*" );

	if( seq_id.length()==0 ) {
		if( echo ) {
			build_vrX( msg, cmd, char_id, recip_id, bml, false );
			// removed logging of vr messages
			//LOG("%s %s", cmd, msg.str().c_str());
		}

		if( send ) {
			// execute directly
			if( all_characters ) {
				for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
					iter != mcu.getCharacterMap().end();
					iter++)
				{
					build_vrX( msg, cmd, (*iter).second->getName().c_str(), recip_id, bml, false );
					mcu.vhmsg_send( cmd, msg.str().c_str() );
				}
			} else {
				build_vrX( msg, cmd, char_id, recip_id, bml, false );
				mcu.vhmsg_send( cmd, msg.str().c_str() );
			}
		}
		return CMD_SUCCESS;
	} else {
		// Command sequence to trigger vrSpeak
		srCmdSeq *seq = new srCmdSeq(); // sequence file that holds the bml command(s)
		seq->offset( (float)( mcu.time ) );

		if( echo ) {
			msg << "echo // Running sequence \"" << seq_id << "\"...";
			if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "WARNING: send_vrX(..): Failed to insert echo header command for character \"" << char_id << "\".";
				LOG(strstr.str().c_str());
			}
			build_vrX( msg, cmd, char_id, recip_id, bml, false );
			if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "WARNING: send_vrX(..): Failed to insert echoed command for character \"" << char_id << "\".";
				LOG(strstr.str().c_str());
			}
		}

		if( all_characters ) {
			for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
				iter != mcu.getCharacterMap().end();
				iter++)
			{
				build_vrX( msg, cmd, (*iter).second->getName().c_str(), recip_id, bml, true );

				//echo.str("");
				//echo << "echo " << msg.str();
				//if( seq->insert( 0, echo.str().c_str() )!=CMD_SUCCESS ) {
				//	strstr << "WARNING: send_vrSpeak(..): Failed to insert vrSpeak command echo for character \"" << char_id << "\"." <<endl;
				//}

				if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
					std::stringstream strstr;
					strstr << "WARNING: send_vrX(..): Failed to insert vrSpeak command for character \"" << char_id << "\".";
					LOG(strstr.str().c_str());
				}
			}
		} else {
			build_vrX( msg, cmd, char_id, recip_id, bml, true );

			//echo.str("");
			//echo << "echo " << msg.str();
			//if( seq->insert( 0, echo.str().c_str() )!=CMD_SUCCESS ) {
			//	strstr << "WARNING: send_vrSpeak(..): Failed to insert vrSpeak command echo for character \"" << char_id << "\"." <<endl;
			//}

			if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "WARNING: send_vrX(..): Failed to insert vrSpeak command for character \"" << char_id << "\".";
				LOG(strstr.str().c_str());
			}
		}

		if( send ) {
			mcu.activeSequences.removeSequence( seq_id, true );  // remove old sequence by this name
			if( !mcu.activeSequences.addSequence( seq_id, seq ))
			{
				std::stringstream strstr;
				strstr << "ERROR: send_vrX(..): Failed to insert seq into active sequences.";
				LOG(strstr.str().c_str());
				return CMD_FAILURE;
			}
		} else {
			mcu.pendingSequences.removeSequence( seq_id, true );   // remove old sequence by this name
			if( !mcu.pendingSequences.addSequence( seq_id, seq ))
			{
				std::stringstream strstr;
				strstr << "ERROR: send_vrX(..): Failed to insert seq into pending sequences.";
				LOG(strstr.str().c_str());
				return CMD_FAILURE;
			}
		}
		return CMD_SUCCESS;
	}
}

void print_test_bml_help() {
	std::stringstream strstr;
	strstr << "Syntax:" << endl
		 << "\ttest bml [char[acter] <character id or * for all>] [echo|noecho] [send|nosend] [seq <seq id>] <test subcommand>" << endl
		 << "where the test subcommand can be any of . . ." << endl
	     << "\thelp                             // prints this text" << endl
	     << "\tanim[ation] <animation name>     // run an animation" << endl
	     << "\tposture <posture name>           // set body posture" << endl
		 << "\tgaze [target] <target>           // sets the gaze" << endl
		 << "\t     [dir[ection] <direction> [angle <angle>]]  // optional offset" << endl
		 << "\t     [speed <lumbar> <cervical> <eyeballs>]     // optional speed" << endl
		 << "\t     [smoothing <lumbar> <cervical> <eyeballs>] // optional smoothing" << endl
		 << "\thead [nod|shake]                 // tests nod behaviors" << endl
		 << "\thead orient <direction> <amount> // angles the head" << endl
		 << "\thead orient target <target>      // orients the head toward an object" << endl
//		 << "\thead orient target <target> [<direction> <amount>]     // orients the head toward an object" << endl
	     << "\tspeech [text] <sentence>         // performs plain text speech" << endl
	     << "\tspeech ssml <sentence>           // performs ssml speech" << endl
	     << "\tinterrupt <performance id>       // interrupt prior performane" << endl
	     << "\tfile <filename>                  // calls vrSpeak on a BML file" << endl
	     << "\t<?xml ...                        // sends inline XML as BML" << endl
	     << "\t<act>...</act>                   // sends inline <act> XML" << endl
	     << "\t<bml>...</bml>                   // sends inline <bml> XML" << endl
	     << "\t< ... />                         // sends inline behavior XML (can be multiple)" << endl;
	LOG("%s", strstr.str().c_str());
}

// Handles all "test bml ..." sbm commands.
int test_bml_func( srArgBuffer& args, mcuCBHandle *mcu ) {
	string char_id = mcu->test_character_default;
	string recip_id = mcu->test_recipient_default;
	string seq_id;
	bool   echo = true;
	bool   send = true;


	string arg = args.read_token();
	if( !read_options( "test bml", args, arg,
		               char_id, recip_id,
					   seq_id, echo, send ) )
		return CMD_FAILURE;


	
	if (char_id != "*" )
	{
		SbmCharacter* character = mcu->getCharacter(char_id);
		if (!character)
		{
			LOG("No character named '%s'.", char_id.c_str());
			return CMD_FAILURE; // short circuit requests that do not map to a character that exists in this sbm instance
		}
	}

	if( arg=="" || arg=="help") {
		print_test_bml_help();
		return CMD_SUCCESS;
	} else if( arg[0]=='<' ) {
		ostringstream bml;
		if( arg=="<?xml" ) {  // does not work with quotes like vrSpeak
			bml << "<?xml " << args.read_remainder_raw();
			return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
		} else if( arg.compare(0,4,"<act",4)==0 ) {
			bml << "<?xml version=\"1.0\" ?>"
			    << arg << " " << args.read_remainder_raw();
			return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
		} else if( arg.compare(0,4,"<bml",4)==0 ) {
			bml << "<?xml version=\"1.0\" ?>"
			    << "<act>" << arg << " " << args.read_remainder_raw() << "</act>";
			return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
		} else {
			bml << "<?xml version=\"1.0\" ?>"
			    << "<act><bml>" << arg << " " << args.read_remainder_raw() << "</bml></act>";
			return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
		}
	} else if( arg=="file") {
		string filename = args.read_remainder_raw();
		// handle quotes
		if( filename[0]=='"' ) {
			filename.erase( 0, 1 );
			filename.erase( filename.length()-1 );
		}
		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, filename );
	} else if( arg=="anim" || arg=="animation") { //  anim[ation] <animation name>
		string anim = args.read_token();
		if( anim.length()==0 ) {
			LOG("ERROR: test bml %s: Missing animation name.", arg.c_str());
			return CMD_FAILURE;
		}

		std::map<std::string, SkMotion*>::iterator motionIter = mcu->motion_map.find(anim);
		if (motionIter == mcu->motion_map.end()) {
			LOG("WARNING: Unknown animation \"%s\".", anim.c_str());
		}

		ostringstream bml;
		bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<bml>\n"
			<< "\t\t<sbm:animation name=\"" << anim << "\"/>\n"
			<< "\t</bml>\n"
			<< "</act>";
		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
	} else if( arg=="posture") { // posture <posture name>
		string posture = args.read_token();
		if( posture.length()==0 ) {
			LOG("ERROR: test bml %s Missing posture name.", arg.c_str());
			return CMD_FAILURE;
		}

		std::map<std::string, SkPosture*>::iterator postureIter = mcu->pose_map.find(posture);
		if (postureIter == mcu->pose_map.end()) {
			// the pose_map is deprecated - don't show the message
			//LOG("WARNING: Unknown posture \"%s\".", posture.c_str());
		}

		ostringstream bml;
		bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<bml>\n"
			<< "\t\t<body posture=\"" << posture << "\"/>\n"
			<< "\t</bml>\n"
			<< "</act>";
		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
	} else if( arg=="gaze") {
		// gaze [direction <direction> [angle <angle>]]
		//      [speed <lumbar> <neck> <eye>]
		//      [smooth <lumbar> <neck> <eye>]
		//      [target] <target string>
		string targetAttr;
		string directionAttr;
		string angleAttr;
		string speedAttr;
		string smoothAttr;

		arg = args.read_token();
		while( arg!="" ) {
			if( arg=="direction" || arg=="dir" ) {
				directionAttr = "direction=\"";
				directionAttr += args.read_token();
				directionAttr += "\" ";
			} else if( arg=="angle" || arg=="dir" ) {
				angleAttr = "angle=\"";
				angleAttr += args.read_token();
				angleAttr += "\" ";
			} else if( arg=="speed" || arg=="dir" ) {
				speedAttr = "sbm:joint-speed=\"";
				speedAttr += args.read_token();
				speedAttr += " ";
				speedAttr += args.read_token();
				speedAttr += " ";
				speedAttr += args.read_token();
				speedAttr += "\" ";
			} else if( arg=="smoothing" || arg=="dir" ) {
				smoothAttr = "sbm:speed-smoothing=\"";
				smoothAttr += args.read_token();
				smoothAttr += " ";
				smoothAttr += args.read_token();
				smoothAttr += " ";
				smoothAttr += args.read_token();
				smoothAttr += "\" ";
			} else {
				// unrecognized arguments are assumed to be target specifiers
				if( arg=="target" ) {
					arg = args.read_token();
				} else if( targetAttr.length()>0 ) {
					LOG("ERROR: test bml gaze: Unexpected second target value.");
					return CMD_FAILURE;
				}

				targetAttr = "target=\"";
				targetAttr += arg;
				targetAttr += "\" ";
			}
			arg = args.read_token();
		}

		if( targetAttr.length()==0 ) {
			LOG("WARNING: test bml gaze: Expected a target value.");
		}
		if( angleAttr.length()>0 && directionAttr.length()==0 ) {
			LOG("WARNING: test bml gaze: Expected a direction when specifying angle.");
			return CMD_FAILURE;
		}

		ostringstream bml;
		// First half of BML
		bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<bml>\n"
			<< "\t\t<gaze " << targetAttr << directionAttr << angleAttr << speedAttr << smoothAttr << "/>\n"
			<< "\t</bml>\n"
			<< "</act>";
		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
	} else if( arg=="head" ) { // head [orient <direction> <amount>|target <target>]
		ostringstream head_attrs;

		string arg = args.read_token();
		if( arg=="" || arg=="help" ) {
			std::stringstream strstr;
			strstr << "Available \"test bml head\" commands:" << endl
			     << "\tnod                         // single vertical (x-axis) nod" << endl
			     << "\tshake                       // single horizontal (y-axis) nod" << endl
			     << "\ttoss                        // z-axis nod?" << endl
			     << "\torient <direction> <angle>  // angles the head" << endl
			     << "\torient target <target>      // orients head toward target";
			LOG("%s", strstr.str().c_str());
			   //<< "\torient target <target> [<direction> <angle>]" << endl;
			return CMD_SUCCESS;
		} else if( arg=="nod" ) {
			head_attrs << "type=\"NOD\"";
		} else if( arg=="shake" ) {
			head_attrs << "type=\"SHAKE\"";
		} else if( arg=="toss" ) {
			head_attrs << "type=\"TOSS\"";
		} else if( arg=="orient" ) {
			head_attrs << " type=\"ORIENT\"";

			arg = args.read_token();
			if( arg=="target" ) {
				LOG("ERROR: test bml head orient: Command \"target\" unimplemented.");
				return CMD_FAILURE;
			} else if( arg=="right" || arg=="left" || arg=="up" || arg=="down" ||
			           arg=="rollright" || arg=="rollleft" ) {
				head_attrs << " direction=\""<<arg<<"\"";  // Should really be capitalized

				arg = args.read_token();
				if( arg.length()!=0 ) {
					head_attrs << " angle=\"" << arg << "\"";
				}
			} else {
				std::stringstream strstr;
				strstr << "ERROR: test bml head orient: Unrecognized head orientation \""<<arg<<"\"."<< endl
					<< "\tRecognized directions: right, left, up, down, rollright, rollleft";
				LOG(strstr.str().c_str());

				return CMD_FAILURE;
			}
		} else {
			std::stringstream strstr;
			strstr << "ERROR: test bml head: Unrecognized command \""<<arg<<"\". Expected: orient, shake, or nod";
			LOG(strstr.str().c_str());
			return CMD_FAILURE;
		}
		ostringstream bml;
		// First half of BML
		bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<bml>\n"
			<< "\t\t<head " << head_attrs.str() << "/>\n"
			<< "\t</bml>\n"
			<< "</act>";
		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
	} else if( arg=="speech") { // speech [text|ssml] <sentence>
		enum { PLAIN_TEXT, SSML };
		int speech_type = PLAIN_TEXT;

		string speech = args.read_token();
		string refTag = "";
		if( speech=="text" ) {
			speech_type = PLAIN_TEXT;
			speech = args.read_remainder_raw();
		} else if( speech=="ssml" ) {
			speech_type = SSML;
			speech = args.read_remainder_raw();
		} else if ( speech =="ref")
		{
			string refName = args.read_token();								
			refTag = " ref=\"" + refName + "\" ";
			speech = "";

		} else {
			speech = speech+" "+args.read_remainder_raw();
		}

		string speech_tag;
		switch( speech_type ) {
			case PLAIN_TEXT:
				speech_tag = "<speech type=\"text/plain\"" + refTag + ">";
				break;
			case SSML:
				speech_tag = "<speech type=\"application/ssml+xml\"" + refTag + ">";
				break;
			default:
				LOG("INTERNAL ERROR: BML::Processor::test_bml_func(..): Invalid speech_type: %d", speech_type);
		}		

		ostringstream bml;
		bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<bml>\n"
			<< "\t\t"<< speech_tag << speech << "</speech>\n"
			<< "\t</bml>\n"
			<< "</act>";
		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
	} else if( arg=="interrupt") {
		string act = args.read_token();
		if( act.length()==0 ) {
			LOG("ERROR: test bml %s: Missing BML performance id.", arg.c_str());
			return CMD_FAILURE;
		}

		ostringstream bml;
		bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<bml>\n"
			<< "\t\t<sbm:interrupt act=\"" << act << "\"/>\n"
			<< "\t</bml>\n"
			<< "</act>";
		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
	} else if( arg=="qdraw" || arg=="quickdraw" ) {  // Shhh! Its a secret!
		string target = args.read_token();
		if( target.length()==0 ) {
			LOG("ERROR: test bml %s: Missing target id.", arg.c_str());
			return CMD_FAILURE;
		}


		string roll;
		string anim;
		string param = args.read_token();
		while( !param.empty() ) {
			// TODO: case insensitive comparison
			if( param == "roll" ) {
				roll = args.read_token();
			} else if( param == "anim" || param == "aniation" ) {
				anim = args.read_token();
			}
			param = args.read_token();
		}

		ostringstream bml;
		bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<bml>\n"
			<< "\t\t<sbm:quickdraw target=\"" << target << "\"";
		if( !roll.empty() )
			bml << " roll=\"" << roll << "\"";
		if( !anim.empty() )
			bml << " anim=\"" << anim << "\"";
		bml << "/>\n"
			<< "\t</bml>\n"
			<< "</act>";

		return send_vrX( "vrSpeak", char_id, recip_id, seq_id, echo, send, bml.str() );
	} else {
		LOG("ERROR: test bml: Unrecognized \"test bml\" subscommand \"%s\".", arg.c_str());
		print_test_bml_help();
		return CMD_FAILURE;
	}
}

void print_test_fml_help() {
	LOG("Syntax:");
	LOG("\ttest fml [char[acter] <character id or * for all>] [echo|noecho] [send|nosend] [seq <seq id>] <test subcommand>");
	LOG("where the test subcommand can be any of . . .");
	LOG("\thelp                        // prints this text");
	LOG("\tspeech [text] <sentence>    // performs plain text speech");
	LOG("\tspeech ssml <sentence>      // performs ssml speech");
	LOG("\tfile <filename>             // calls vrSpeak on a BML file");
	LOG("\t<?xml ...                   // sends inline XML");
	LOG("\t<act>...</act>              // sends inline <act> XML");
	LOG("\t<fml>...</fml>              // sends inline <fml> XML");
}

int test_fml_func( srArgBuffer& args, mcuCBHandle *mcu ) {
	string char_id = mcu->test_character_default;
	string recip_id = mcu->test_recipient_default;
	string seq_id;
	bool   echo = true;
	bool   send = true;


	string arg = args.read_token();
	if( !read_options( "test fml", args, arg,
		               char_id, recip_id,
					   seq_id, echo, send ) )
		return CMD_FAILURE;

	if( arg=="" || arg=="help") {
		print_test_fml_help();
		return CMD_SUCCESS;
	} else if( arg[0]=='<' ) {
		ostringstream fml;
		if( arg=="<?xml" ) {  // does not work with quotes like vrSpeak
			fml << "<?xml " << args.read_remainder_raw();
			return send_vrX( "vrExpress", char_id, recip_id, seq_id, echo, send, fml.str() );
		} else if( arg.compare(0,4,"<act",4)==0 ) {
			fml << "<?xml version=\"1.0\" ?>"
			    << arg << args.read_remainder_raw();
			return send_vrX( "vrExpress", char_id, recip_id, seq_id, echo, send, fml.str() );
		} else if( arg.compare(0,4,"<fml",4)==0 ) {
			fml << "<?xml version=\"1.0\" ?>"
			    << "<act>" << arg << args.read_remainder_raw() << "</act>";
			return send_vrX( "vrExpress", char_id, recip_id, seq_id, echo, send, fml.str() );
		} else {
			LOG("ERROR: test_fml_func: Unrecognized test FML command: \"%s\"", arg.c_str());
			return CMD_FAILURE;
		}
	} else if( arg=="file") {
		string filename = args.read_remainder_raw();
		// handle quotes
		if( filename[0]=='"' ) {
			filename.erase( 0, 1 );
			filename.erase( filename.length()-1 );
		}

		ostringstream fml;
		ifstream file( filename.c_str() );
		if( !file ) {
			LOG("ERROR: test_fml_func: Unable to open file: \"%s\"", filename.c_str());
			return CMD_FAILURE;
		}
		fml << file.rdbuf();

		return send_vrX( "vrExpress", char_id, recip_id, seq_id, echo, send, fml.str() );
	} else if( arg=="speech") {
		enum { PLAIN_TEXT, SSML };
		int speech_type = PLAIN_TEXT;

		string speech = args.read_token();
		if( speech=="text" ) {
			speech_type = PLAIN_TEXT;
			speech = args.read_remainder_raw();
		} else if( speech=="ssml" ) {
			speech_type = SSML;
			speech = args.read_remainder_raw();
		} else {
			speech = speech+" "+args.read_remainder_raw();
		}

		string speech_tag;
		switch( speech_type ) {
			case PLAIN_TEXT:
				speech_tag = "<speech id=\"s1\" type=\"text/plain\">";
				break;
			case SSML:
				speech_tag = "<speech id=\"s1\" type=\"application/ssml+xml\">";
				break;
			default:
				LOG("INTERNAL ERROR: BML::Processor::test_bml_func(..): Invalid speech_type: %d", speech_type);
		}

		ostringstream fml;
		fml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<act>\n"
			<< "\t<participant id=\""<<char_id<<"\" role=\"actor\" />\n"
			<< "\t<bml>\n"
			<< "\t\t"<< speech_tag << speech << "</speech>\n"
			<< "\t</bml>\n"
			<< "</act>";
		return send_vrX( "vrExpress", char_id, recip_id, seq_id, echo, send, fml.str() );
	} else {
		LOG("ERROR: test bml: Unrecognized command \"%s\".", arg.c_str());
		print_test_fml_help();
		return CMD_FAILURE;
	}
}


int test_bone_pos_func( srArgBuffer& args, mcuCBHandle* mcu_p ) {
	string& character_id = mcu_p->test_character_default;
	if( character_id.empty() ) {
		LOG("ERROR: No test character defined");
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->getCharacter( character_id );
	if( character == NULL ) {
		LOG("ERROR: Unknown test character \"%s\"", character_id.c_str());
		return CMD_FAILURE;
	}

	int err=0;
	SkChannelArray _channels;
	if ( err == 0 ){
		char* channel_id = args.read_token();
		_channels.add( channel_id, SkChannel::YPos );
		_channels.add( channel_id, SkChannel::XPos);
		_channels.add( channel_id, SkChannel::ZPos);
	}

	
	MeCtChannelWriter* boneWriter= new MeCtChannelWriter();
	boneWriter->init(character, _channels, true);
	//quat_t q = euler_t(50,50,50);
	float data[3] = { (float)args.read_double(), (float)args.read_double(), (float)args.read_double() };
	//cout<<endl<<"here's the data "<<endl<<data[0]<<" "<<data[1]<<" "<<data[2]<<endl;
	boneWriter->set_data( data );

	character->posture_sched_p->create_track( 0, 0, boneWriter );

	return (CMD_SUCCESS);
}
