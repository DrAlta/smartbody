/*
 *  sk_motion_io.cpp - part of Motion Engine and SmartBody-lib
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
 *      Marcelo Kallmann, USC (currently at UC Merced)
 *      Ashok Basawapatna, USC (no longer)
 *      Ed Fast, USC
 *      Andrew n marshall, USC
 *      Marcus Thiebaux, USC
 */

#include "vhcl.h"
# include <stdlib.h>
# include <sk/sk_motion.h>
#include <iostream>
#include <sstream>

using namespace std;


#define DEPRECATE_BAD_METADATA_NAMES (false)

//============================= load ============================

bool SkMotion::_load_bvh ( SrInput& in ) {
  
   SrInput::TokenType type;

   SrString name;
   float freq = 0.0f;
   int n, frames = 0;

   // 1. read channels
   while (true)
    { type = in.get_token();
      SrString& s = in.last_token();

      if ( type==SrInput::EndOfFile ) return true;

      if ( type==SrInput::Name )
       { if ( s=="ROOT" || s=="JOINT" )
          { if ( in.get_token()!=SrInput::Name ) return false;
             name = in.last_token();
          }
         else if ( s=="CHANNELS" )
          { in >> n;
            if ( in.last_error()!=SrInput::NoError ) return false;
            while ( n-- )
             { in.get_token();
               _channels.add ( (const char*) name, SkChannel::get_type(in.last_token()) );
             }
          }
         else if ( s=="MOTION" ) break;
       }
    }

   // 2. read motion description
   while ( in.get_token()==SrInput::Name )
    { if ( in.last_token()=="Frames" )
       { in.getd(); in.getn();
         frames = in.last_token().atoi();
       }

      if ( in.last_token()=="Time" )
       { in.getd(); in.getn();
         freq = in.last_token().atof();
       }

      if ( in.last_error()==SrInput::UnexpectedToken ) return false;
    }
   in.unget_token();
  
   if ( _channels.size()==0 ) return false;

   _postsize = _channels.size();
   _frames.resize( frames );
   _frames.clear ();

   // 3. Read the motion data, note that Quaternions are not used in bvh format
   int f, i;
   float val, kt = 0;
   for ( f=0; f<frames; f++ )
    {
		_frames.push_back(Frame());
		int topFrame = _frames.size() - 1;
      _frames[topFrame].keytime = kt;
      _frames[topFrame].posture = (float*) malloc ( sizeof(float)*_postsize );
      kt += freq;
      for ( i=0; i<_postsize; i++ )
       { in.getn();
         if ( in.last_error()==SrInput::UnexpectedToken ) return false; // eof or other
         val = in.last_token().atof();
         if ( _channels[i].type>=SkChannel::XRot ) val = SR_TORAD(val);
         _frames[topFrame].posture[i] = val;
       }
    }

   compress ();

   return true;
}

//======================= parse_timing_metadata ========================

bool parse_timing_metadata( SrInput& in, SrString name, float& time ) {
	int      line_number = in.curline();  // store in case of error

	SrInput::TokenType token_type = in.get_token();
	SrString&          token      = in.last_token();

	if( token_type!=SrInput::Name ||
		token!="time" )
	{
		std::stringstream strstr;
		strstr << "ERROR: SkMotion::load(): File \""<<in.filename()<<"\", line "<<line_number<<": Invalid \""<<name<<"\" line. Expected \"time:\" before value, but recieved \"" << token << "\" (token_type "<<token_type<<").";
		LOG(strstr.str().c_str());
		return false;
	}
	token_type = in.get_token();
	if( token_type==SrInput::Delimiter ) {
		// Ignore and get next token
		token_type = in.get_token();
	}
	token      = in.last_token();
	if( token_type!=SrInput::Real && token_type!=SrInput::Integer ) {
		std::stringstream strstr;
		strstr << "ERROR: SkMotion::load(): File \""<<in.filename()<<"\", line "<<line_number<<": Invalid \""<<name<<"\" time value. Expected number, but recieved \"" << token << "\" (token_type "<<token_type<<").";
		LOG(strstr.str().c_str());
		return false;
	}
	time = token.atof();
	return true;
}

//============================= load ============================

bool SkMotion::load ( SrInput& in, double scale ) {
	//  unset/invalid ready stroke relax times are given a value of -1
	float _time_ready=-1;
	float _time_stroke_start=-1;
	float _time_stroke_emphasis=-1;
	float _time_stroke_end=-1;
	float _time_relax=-1;

	in.lowercase_tokens ( false ); // string comparison remains case insensitive
	in.comment_style ( '#' );

	init ();

	// 1. verify signature
	in.get_token();
	if ( in.last_token()=="HIERARCHY" ) { // bvh format
		return _load_bvh ( in );
	} else if ( in.last_token()!="SkMotion" && in.last_token()!="HoMotion" ) {
		return false;
	}

	// 2. read name if any:
	in.get_token();
	if ( in.last_token()=="name" ) {
		in.get_token();
		name ( in.last_token() );
	} else {
		in.unget_token();
	}

	// 3. read channels
	in >> _channels;
	_postsize = _channels.floats();

	// 4. check if a start_kt is set
	float start_kt = -1.0f;
	in.get_token();
	if ( in.last_token()=="start_kt" ) {
		in.get_token(); 
		start_kt = in.last_token().atof();
	} else {
		in.unget_token();
	}

	// 5. read number of frames, if specified
	int num_frames = -1;
	in.get_token();
	if ( in.last_token()=="frames" ) {
		in.get_token(); 
		num_frames = in.last_token().atoi();
		_frames.resize ( num_frames );
		_frames.clear ();
	}
	else
		in.unget_token();

	// 6. read data
	int i, f=0;
	int chsize = _channels.size();

	float *pt, kt=0;
	while ( f < num_frames ) { 
		if ( in.get_token()==SrInput::EndOfFile )
			break; // 'kt' for key time
		if ( in.get_token()==SrInput::EndOfFile )
			break;
		insert_frame ( f, in.last_token().atof() );
		in.get_token(); // fr
		pt = _frames[f].posture;
		for ( i=0; i<chsize; i++ ) {
			pt += _channels[i].load ( in, pt, scale );
		}
		++f;
	}
	if( start_kt>-1 )
		move_keytimes ( start_kt );
	compress();


	// 7. read timing metadata until end of file
	SrInput::TokenType token_type = in.get_token();
	if( token_type==SrInput::EndOfFile ) {
#ifdef DEBUG
		cout<<endl<<endl<<"WARNING: ATTEMPT TO LOAD SKM WITHOUT READY,STROKE,RELAX METADATA : "<<_name<<endl; 
#endif
	} else {
		enum { READY=1, STROKE_START=2, STROKE_EMPH=4, STROKE_END=8, RELAX=16, ALL=31 };
		int metadata_flags = 0;
		do {
			SrString& token = in.last_token();

			if( token=="ready" ) {
				if( parse_timing_metadata( in, token, _time_ready ) )
					metadata_flags |= READY;
				else
					return false;
			} else if( token=="stroke_start" ) {
				if( parse_timing_metadata( in, token, _time_stroke_start ) )
					metadata_flags |= STROKE_START;
				else
					return false;
			} else if( token=="stroke_emphasis" ) {
				if( parse_timing_metadata( in, token, _time_stroke_emphasis ) )
					metadata_flags |= STROKE_EMPH;
				else
					return false;
			} else if( token=="stroke_end" ) {
				if( parse_timing_metadata( in, token, _time_stroke_end ) )
					metadata_flags |= STROKE_END;
				else
					return false;
			} else if( token=="relax" ) {
				if( parse_timing_metadata( in, token, _time_relax ) )
					metadata_flags |= RELAX;
				else
					return false;
			} else if( token=="strokeStart" ) {
				if( DEPRECATE_BAD_METADATA_NAMES )
				{
					std::stringstream strstr;
					strstr << "WARNING: SkMotion::load(): File \""<<in.filename()<<"\", line "<<in.curline()<<": Metadata \"strokeStart\" has been deprecated in favor of \"stroke_start\" to match BML.";
					LOG(strstr.str().c_str());
				}
				if( parse_timing_metadata( in, token, _time_stroke_start ) )
					metadata_flags |= STROKE_START;
				else
					return false;
			} else if( token=="emphasis" ) {
				if( DEPRECATE_BAD_METADATA_NAMES )
				{
					std::stringstream strstr;
					strstr << "WARNING: SkMotion::load(): File \""<<in.filename()<<"\", line "<<in.curline()<<": Metadata \"emphasis\" has been deprecated in favor of \"stroke_emphasis\" to match BML." << endl;
					LOG(strstr.str().c_str());
				}
				if( parse_timing_metadata( in, token, _time_stroke_emphasis ) )
					metadata_flags |= STROKE_EMPH;
				else
					return false;
			} else if( token=="stroke" ) {
				if( DEPRECATE_BAD_METADATA_NAMES )
				{
					std::stringstream strstr;
					strstr << "WARNING: SkMotion::load(): File \""<<in.filename()<<"\", line "<<in.curline()<<": Metadata \"stroke\" has been deprecated in favor of \"stroke_end\" to match BML." << endl;
					LOG(strstr.str().c_str());
				}
				if( parse_timing_metadata( in, token, _time_stroke_end ) )
					metadata_flags |= STROKE_END;
				else
					return false;
			} else if( token=="strokeEnd" ) {
				if( DEPRECATE_BAD_METADATA_NAMES )
				{
					std::stringstream strstr;
					strstr << "WARNING: SkMotion::load(): File \""<<in.filename()<<"\", line "<<in.curline()<<": Metadata \"strokeEnd\" has been deprecated in favor of \"stroke_end\" to match BML." << endl;
					LOG(strstr.str().c_str());
				}
				if( parse_timing_metadata( in, token, _time_stroke_end ) )
					metadata_flags |= STROKE_END;
				else
					return false;
			} else {
				std::stringstream strstr;
				strstr << "ERROR: SkMotion::load(): ";
				if( in.filename() )
				{
					strstr << "File \""<<in.filename()<<"\", line "<<in.curline();
				}
				else
				{
					strstr << "Unknown file, line "<<in.curline();
				}
				strstr <<": Expected metadata name, but recieved \"" << token << "\".";
				LOG(strstr.str().c_str());
				return false;
			}
		} while( in.get_token() != SrInput::EndOfFile );

		if( metadata_flags != ALL )
		{
			std::stringstream strstr;
			strstr << "WARNING: SkMotion::load(): File \""<<in.filename()<<"\": Timing Metadata incomplete before end of file (metadata_flags: "<<metadata_flags<<").";
			LOG(strstr.str().c_str());
		}

		synch_points.set_time( 
			keytime( 0 ), _time_ready, 
			_time_stroke_start, _time_stroke_emphasis, _time_stroke_end,
			_time_relax, last_keytime()
		);
		if( synch_points.get_error() )	{
			LOG( "SkMotion::load ERR: reading synch points in '%s'", in.filename() );
			synch_points.print_error();
		}
	}

//	bool metadata_valid = true;
//
//	while( metadata_valid ) {
//		//if the end of the file is reached before "ready" string is reached-- ie: the ready data does not exist
//		if ( in.get_token()==SrInput::EndOfFile ) {
//#ifdef DEBUG
//			cout<<endl<<endl<<"WARNING: ATTEMPT TO LOAD SKM WITHOUT READY,STROKE,RELAX METADATA : "<<_name<<endl; 
//#endif
//			metadata_valid = false; //makes the data invalid so that stroke and relax aren't attempted to read
//		}
//
//		//if ready string exists read the associated ready data as a float
//		if( token=="ready" ) {
//			in.get_token();in.get_token(); in.get_token(); //first token gets "time" string, next one gets ":" string and finally we get our actual value
//			_time_ready = in.last_token().atof(); //read as float
//			break;
//		}
//	}
//
//	while( metadata_valid ){
//		//if the end of the file is reached before "stroke" string is reached-- ie: the ready data does not exist 
//		if ( in.get_token()==SrInput::EndOfFile ) {
//			metadata_valid = false;
//		}
//
//		if ( token=="strokeStart" ) {
//			in.get_token();in.get_token(); in.get_token(); //first token gets "time" string, next one gets ":" string and finally we get our actual value
//
//			_time_stroke_start = in.last_token().atof(); //read as float
//			break;
//		}
//	}
//
//	while( metadata_valid ) {
//		//if the end of the file is reached before "ready" string is reached-- ie: the ready data does not exist 
//		if ( in.get_token()==SrInput::EndOfFile ) {
//			metadata_valid = false;
//		}
//		if( token=="emphasis") {
//			in.get_token();in.get_token();in.get_token(); //first token gets "time" string, next one gets ":" string and finally we get our actual value
//			_time_stroke_emphasis = in.last_token().atof();  //read as float
//			break;
//		}
//	}
//
//	while( metadata_valid ) {
//		//if the end of the file is reached before "ready" string is reached-- ie: the ready data does not exist 
//		if ( in.get_token()==SrInput::EndOfFile ) {
//			metadata_valid = false;
//		}
//		if( token=="stroke" ) {
//			in.get_token();in.get_token();in.get_token(); //first token gets "time" string, next one gets ":" string and finally we get our actual value
//			_time_stroke_end = in.last_token().atof();  //read as float
//			break;
//		}
//	}
//
//	while( metadata_valid ) {
//		//if the end of the file is reached before "ready" string is reached-- ie: the ready data does not exist 
//		if ( in.get_token()==SrInput::EndOfFile ) {
//			metadata_valid = false;
//		}
//
//		if( token=="relax" ) {
//			in.get_token();in.get_token();in.get_token(); //first token gets "time" string, next one gets ":" string and finally we get our actual value
//			_time_relax = in.last_token().atof();  //read as float
//			break;
//		}
//	}
	
//	LOG( "SkMotion::load '%s' %d frames\n", name(), frames() );
	return true;
}

//============================= save ============================

bool SkMotion::save ( SrOutput& out )
 {
   out << "SkMotion\n\n";

   if ( _name )
    { SrString s;
      s.make_valid_string ( _name );
      out << "name " << s << srnl<<srnl;
    }

   out << _channels << srnl;

   out << "frames " << _frames.size() << srnl;

   float *pt;
   int chsize = _channels.size();
   
   for (size_t i=0; i<_frames.size(); i++ )
    { out << "kt " << _frames[i].keytime << " fr ";
      pt = _frames[i].posture;
      for (int j=0; j<chsize; j++ )
       { pt += _channels[j].save ( out, pt );
         if ( j+1<chsize ) out<<srspc;
       }
      out << srnl;
    }
   out << srnl;

   return true;
 }

//============================ End of File ===========================
