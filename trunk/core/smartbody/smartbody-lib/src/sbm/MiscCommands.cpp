#include "MiscCommands.h"

#include <sbm/mcontrol_util.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBBoneBusManager.h>


int set_attribute( SbmPawn* pawn, string& attribute, srArgBuffer& args)
{
	mcuCBHandle* mcu_p = &mcuCBHandle::singleton();

	if( attribute=="world_offset" || attribute=="world-offset" ) {
		//  Command: set pawn <character id> world_offset ...
		//  Sets the parameters of the world_offset joint
		return set_world_offset_cmd( pawn, args );
	} 
	else if (attribute == "mass")
	{
		if (args.calc_num_tokens() == 0)
		{
			SkSkeleton* skeleton = pawn->_skeleton;
			std::vector<SkJoint*>& joints = skeleton->get_joint_array();
			for (size_t j = 0; j < joints.size(); j++)
			{
				LOG("%s : %f", joints[j]->name().c_str(), joints[j]->mass());

			}
			return CMD_SUCCESS;
		}
		std::string jointName = args.read_token();
		if (jointName.length() == 0)
		{
			LOG("ERROR: SbmCharacter::set_cmd_func(..): Need joint name. Use: set char mass <joint> <amount>");
			return CMD_FAILURE;
		}
		const SkJoint* joint = pawn->get_joint(jointName.c_str());
		if (!joint)
		{
			LOG("ERROR: SbmCharacter::set_cmd_func(..): No joint found with name '%s'.", jointName.c_str());
			return CMD_FAILURE;
		}
		float mass = args.read_float();
		if (mass < 0)
		{
			LOG("ERROR: SbmCharacter::set_cmd_func(..): Mass must be > 0.");
			return CMD_FAILURE;
		}
		// is there a function that returns an SkJoint* and not a const SkJoint*?
		// That would make this next line of code unnecessary.
		SkJoint* editableJoint = const_cast<SkJoint*>(joint);
		editableJoint->mass(mass);
		//LOG("Set joint '%s' on character '%s' to mass '%f'.", jointName.c_str(), pawn->name, mass);
		return CMD_SUCCESS;
	} 	
	else 
	{
		LOG("ERROR: SbmPawn::set_cmd_func(..): Unknown attribute \"%s\".", attribute.c_str() );
		return CMD_FAILURE;
	}
}


int print_attribute( SbmPawn* pawn, string& attribute, srArgBuffer& args)
{
	mcuCBHandle* mcu_p = &mcuCBHandle::singleton();
	std::stringstream strstr;
	if( attribute=="world_offset" || attribute=="world-offset" ) {
		//  Command: print pawn <character id> world_offset
		//  Print out the current state of the world_offset joint
		strstr << "pawn " << pawn->getName() << " world_offset:\t";
		const SkJoint* joint = pawn->get_world_offset_joint();
		LOG(strstr.str().c_str());
		if( joint==NULL ) {
			LOG("No world_offset joint.");
		} else {
			print_joint( joint );
		}
		return CMD_SUCCESS;
	} else if( attribute=="joint" || attribute=="joints" ) {
		//  Command: print character <character id> [joint|joints] <joint name>*
		//  Print out the current state of the named joints
		string joint_name = args.read_token();
		if( joint_name.length()==0 ) {
			LOG("ERROR: SbmPawn::print_attribute(..): Missing joint name of joint to print.");
			return CMD_FAILURE;
		}

		do {
			strstr.clear();
			strstr << "pawn " << pawn->getName() << " joint "<<joint_name<<":\t";
			const SkJoint* joint = pawn->get_joint( joint_name.c_str() );
			LOG(strstr.str().c_str());
			if( joint==NULL ) {
				LOG("No joint \"%s\".", joint_name.c_str() );
			} else {
				print_joint( joint );
			}

			joint_name = args.read_token();
		} while( joint_name.length()>0 );

		return CMD_SUCCESS;
	} else {
		LOG("ERROR: SbmPawn::print_attribute(..): Unknown attribute \"%s\".", attribute.c_str() );
		return CMD_FAILURE;
	}
}


#if USE_WSP
WSP::WSP_ERROR remote_pawn_position_update( std::string id, std::string attribute_name, wsp_vector & vector_3d, void * data, const std::string & data_provider )
{
	mcuCBHandle * mcu_p = static_cast< mcuCBHandle * >( data );

	SbmPawn * pawn_p = mcu_p->getPawn( id );
	if ( pawn_p != NULL )
	{
		float x, y, z, h, p, r;
		pawn_p->get_world_offset( x, y, z, h, p, r );

		pawn_p->set_world_offset( (float)vector_3d.x, (float)vector_3d.y, (float)vector_3d.z, h, p, r );
	}
	else
	{
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::remote_pawn_position_update: SbmPawn '" << id << "' is NULL, cannot set_world_offset";
		LOG(strstr.str().c_str());
		return WSP::not_found_error( "SbmPawn is NULL" );
	}

	return WSP::no_error();
}

WSP::WSP_ERROR remote_pawn_rotation_update( std::string id, std::string attribute_name, wsp_vector & vector_4d, void * data, const std::string & data_provider )
{
	mcuCBHandle * mcu_p = static_cast< mcuCBHandle * >( data );

	SbmPawn * pawn_p = mcu_p->getPawn( id );

	if ( pawn_p != NULL )
	{
		float x, y, z, h, p, r;
		pawn_p->get_world_offset( x, y, z, h, p, r );

		gwiz::euler_t e = gwiz::quat_t( vector_4d.q, vector_4d.x, vector_4d.y, vector_4d.z );
		pawn_p->set_world_offset( x, y, z, (float)e.h(), (float)e.p(), (float)e.r() );
	}
	else
	{
		std::stringstream strstr;
		strstr << "ERROR: SbmPawn::remote_pawn_rotation_update: SbmPawn '" << id << "' is NULL, cannotsbm set_world_offset";
		LOG(strstr.str().c_str());
		return  WSP::not_found_error( "SbmPawn is NULL" );
	}

	return  WSP::no_error();
}

void handle_wsp_error( std::string id, std::string attribute_name, int error, std::string reason, void* data )
{

	LOG( "error getting id: %s attribute_name: %s. error_code: %d reason: %s\n", id.c_str(), attribute_name.c_str(), error, reason.c_str() );
}


#endif

int pawn_set_cmd_funcx( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	string pawn_id = args.read_token();
	if( pawn_id.length()==0 ) {
		LOG("ERROR: SbmPawn::set_cmd_func(..): Missing pawn id.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn = mcu_p->getPawn( pawn_id );
	if( pawn==NULL ) {
		LOG("ERROR: SbmPawn::set_cmd_func(..): Unknown pawn id \"%s\".", pawn_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmPawn::set_cmd_func(..): Missing attribute \"%s\" to set.", attribute.c_str());
		return CMD_FAILURE;
	}

	return  set_attribute( pawn, attribute, args);
}

int set_voice_cmd_func( SbmCharacter* character, srArgBuffer& args)
{
	mcuCBHandle* mcu_p = &mcuCBHandle::singleton();
	//  Command: set character voice <speech_impl> <character id> voice <implementation-id> <voice code>
	//  Where <implementation-id> is "remote" or "audiofile"
	//  Sets character's voice code
	const char* impl_id = args.read_token();

	if( strlen( impl_id )==0 ) {
		character->set_speech_impl( NULL );
		string s( "" );
		character->set_voice_code( s );

		// Give feedback if unsetting
		LOG("Unset %s's voice.", character->getName().c_str());
	} else if( _stricmp( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected remote voice id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_rvoice() );
		string s( voice_id );
		character->set_voice_code( s );
	} else if( _stricmp( impl_id, "local" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected local voice id.");
			return CMD_FAILURE;
		}
		LOG("set local voice");
		character->set_speech_impl( mcu_p->speech_localvoice() );
		FestivalSpeechRelayLocal* relay = mcu_p->festivalRelay();
		relay->setVoice(voice_id);
		string s( voice_id );
		character->set_voice_code( s );
	} else if( _stricmp( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected audiofile voice path.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else if( _stricmp( impl_id, "text" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl( mcu_p->speech_text() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code( voice_path_str );
	} else {
		LOG("ERROR: Unknown speech implementation \"%s\".", impl_id);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

int set_voicebackup_cmd_func( SbmCharacter* character, srArgBuffer& args)
{
	mcuCBHandle* mcu_p = &mcuCBHandle::singleton();
	//  Command: set character voice <speech_impl> <character id> voice <implementation-id> <voice code>
	//  Where <implementation-id> is "remote" or "audiofile"
	//  Sets character's voice code
	const char* impl_id = args.read_token();

	if( strlen( impl_id )==0 ) {
		character->set_speech_impl_backup( NULL );
		string s("");
		character->set_voice_code_backup( s );

		// Give feedback if unsetting
		LOG("Unset %s's voice.", character->getName().c_str());
	} else if( _stricmp( impl_id, "remote" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected remote voice id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_rvoice() );
		string s( voice_id );
		character->set_voice_code_backup( s );
	} else if( _stricmp( impl_id, "local" )==0 ) {
		const char* voice_id = args.read_token();
		if( strlen( voice_id )==0 ) {
			LOG("ERROR: Expected local voice id.");
			return CMD_FAILURE;
		}
		LOG("set local voice");
		character->set_speech_impl_backup( mcu_p->speech_localvoice() );
		FestivalSpeechRelayLocal* relay = mcu_p->festivalRelay();
		relay->setVoice(voice_id);
		string s( voice_id );
		character->set_voice_code_backup( s );
	} else if( _stricmp( impl_id, "audiofile" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected audiofile voice path.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_audiofile() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code_backup( voice_path_str );
	} else if( _stricmp( impl_id, "text" )==0 ) {
		const char* voice_path = args.read_token();
		if( strlen( voice_path )==0 ) {
			LOG("ERROR: Expected id.");
			return CMD_FAILURE;
		}
		character->set_speech_impl_backup( mcu_p->speech_text() );
		string voice_path_str= "";
		voice_path_str+=voice_path;
		character->set_voice_code_backup( voice_path_str );
	} else {
		LOG("ERROR: Unknown speech implementation \"%s\".", impl_id);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}






int pawn_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	string pawn_name = args.read_token();
	if( pawn_name.length()==0 )
	{
		LOG("ERROR: Expected pawn name.");
		return CMD_FAILURE;
	}

	string pawn_cmd = args.read_token();
	if( pawn_cmd.length()==0 )
	{
		LOG("ERROR: Expected pawn command.");
		return CMD_FAILURE;
	}

	if (pawn_cmd == "init")
	{
		// pawn <name> init [loc <x> <y> <z>] [geom <shape name>] [color <color hex>] [size <size>]
		SbmPawn* pawn_p = mcu_p->getPawn(pawn_name);
		if( pawn_p != NULL ) {
			LOG("ERROR: Pawn \"%s\" already exists.", pawn_name.c_str());
			return CMD_FAILURE;
		}
		// Options
		float loc[3] = { 0, 0, 0 };

		bool has_geom = false;
		std::string geom_str = "box";
		std::string file_str = "";
		std::string size_str = "";
		std::string color_str = "red";
		std::string type_str = "";
		SrVec size = SrVec(1.f,1.f,1.f);
		bool setRec = false;
		SrVec rec;
		std::string defaultColor = "red";
		while( args.calc_num_tokens() > 0 ) {
			string option = args.read_token();
			// TODO: Make the following option case insensitive
			if( option == "loc" ) {
				args.read_float_vect( loc, 3 );
			} else if( option=="geom" ) {
				geom_str = args.read_token();
				has_geom = true;
			} else if (option == "file")
			{
				file_str = args.read_token();
				has_geom = true;	
			} else if( option=="type" ) {
				type_str = args.read_token();
				has_geom = true;
			} else if( option=="size" ) {
				size_str = args.read_token();
				has_geom = true;
			} else if( option=="color" ) {
				color_str = args.read_token();
				has_geom = true;
			} else if( option=="rec" ) {
				setRec = true;
				size.x = rec.x = args.read_float();
				size.y = rec.y = args.read_float();
				size.z = rec.z = args.read_float();
				has_geom = true;
			} else {
				std::stringstream strstr;
				strstr << "WARNING: Unrecognized pawn init option \"" << option << "\"." << endl;
				LOG(strstr.str().c_str());
			}
		}		

		pawn_p = new SmartBody::SBPawn( pawn_name.c_str() );
		pawn_p->setClassType("pawn");
		SkSkeleton* skeleton = new SmartBody::SBSkeleton();
		skeleton->ref();
		string skel_name = pawn_name+"-skel";
		skeleton->name( skel_name.c_str() );
		// Init channels
		skeleton->make_active_channels();
		if (mcu_p->sbm_character_listener)
		{
			mcu_p->sbm_character_listener->OnCharacterChanged(pawn_name);
		}

		int err = pawn_p->init( skeleton );
		mcu_p->registerPawn(pawn_p);

		if( err != CMD_SUCCESS ) {
			std::stringstream strstr;		
			strstr << "ERROR: Unable to initialize SbmPawn \"" << pawn_name << "\".";
			LOG(strstr.str().c_str());
			delete pawn_p;
			skeleton->unref();
			return err;
		}

		// setting up geometry and physics 
		if( has_geom && !geom_str.empty() ) {
			//LOG("WARNING: SbmPawn geometry not implemented.  Ignoring options.");			
			if (!size_str.empty())
			{
				float uniformSize = (float)atof(size_str.c_str());
				for (int i=0;i<3;i++)
					size[i] = uniformSize;
			}			
			//pawn_p->initGeomObj(geom_str.c_str(),size,color_str.c_str(),file_str.c_str());
			SBPhysicsManager* phyManager = mcu_p->_scene->getPhysicsManager();
			phyManager->createPhysicsPawn(pawn_p->getName(),geom_str,size);
		}
		if (pawn_p->getGeomObject())
		{
			if (geom_str == "box")
			{
				pawn_p->steeringSpaceObjSize = rec;
				if (!setRec)
				{
					float size = (float)atof(size_str.c_str());
					pawn_p->steeringSpaceObjSize = SrVec(size, size, size);
				}
				if (type_str == "steering")
					pawn_p->initSteeringSpaceObject();
			}
		}
		// 		else // default null geom object
		// 		{
		// 			SbmGeomObject* colObj = new SbmGeomNullObject();
		// 			pawn_p->colObj_p = colObj;
		// 		}

		/*
		bool ok = mcu_p->addPawn( pawn_p );
		if( !ok )	{
			std::stringstream strstr;
			strstr << "ERROR: SbmPawn pawn_map.insert(..) \"" << pawn_name << "\" FAILED";
			LOG(strstr.str().c_str());
			delete pawn_p;
			skeleton->unref();
			return err;
		}
		*/

		if (pawn_p->getPhysicsObject())
		{
			pawn_p->setWorldOffset(pawn_p->getPhysicsObject()->getGlobalTransform().gmat());
		}
		// [BMLR] Send notification to the renderer that a pawn was created.
		// NOTE: This is sent both for characters AND pawns
		mcu_p->_scene->getBoneBusManager()->getBoneBus().SendCreatePawn( pawn_name.c_str(), loc[ 0 ], loc[ 1 ], loc[ 2 ] );
		float x,y,z,h,p,r;
		pawn_p->get_world_offset(x,y,z,h,p,r);
		//printf("h = %f, p = %f, r = %f\n",h,p,r);	
		pawn_p->set_world_offset(loc[0],loc[1],loc[2],h,p,r);	
		pawn_p->wo_cache_update();

		if (mcu_p->sendPawnUpdates)
			pawn_p->bonebusCharacter = mcu_p->_scene->getBoneBusManager()->getBoneBus().CreateCharacter( pawn_name.c_str(), pawn_p->getClassType().c_str(), false );

		if ( mcuCBHandle::singleton().sbm_character_listener )
		{
			mcuCBHandle::singleton().sbm_character_listener->OnCharacterCreate( pawn_name, pawn_p->getClassType().c_str() );
		}

		return CMD_SUCCESS;
	}

	bool all_pawns = false;
	SbmPawn* pawn_p = NULL;
	if( pawn_name== "*" )
	{
		std::vector<std::string> pawns;
		for (std::map<std::string, SbmPawn*>::iterator iter = mcu_p->getPawnMap().begin();
			iter != mcu_p->getPawnMap().end();
			iter++)
		{
			pawns.push_back((*iter).second->getName());
		}
		for (std::vector<std::string>::iterator citer = pawns.begin();
			citer != pawns.end();
			citer++)
		{
			srArgBuffer copy_args( args.peek_string() );
			pawn_p = mcu_p->getPawn( *citer );
			int err = pawn_parse_pawn_command( pawn_p, pawn_cmd, copy_args);
			if( err != CMD_SUCCESS )
				return( err );
		}
		return CMD_SUCCESS;
	} 
	else
	{
		pawn_p = mcu_p->getPawn( pawn_name.c_str() );
		if( pawn_p ) 
		{
			int ret = pawn_parse_pawn_command( pawn_p, pawn_cmd, args);
			return( ret );
		}
		else
		{
			LOG("No pawn named '%s' exists.", pawn_name.c_str());
			return CMD_FAILURE;
		}
	}
}

int character_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	string char_name = args.read_token();
	if( char_name.length()==0 ) {
		LOG( "HELP: char <> <command>" );
		LOG( "  param" );
		LOG( "  init" );
		LOG( "  smoothbindmesh" );
		LOG( "  smoothbindweight" );
		LOG( "  mesh");
		LOG( "  ctrl" );
		LOG( "  inspect" );
		LOG( "  channels" );
		LOG( "  controllers" );
		LOG( "  prune" );
		LOG( "  viseme curveon|curveoff" );
		LOG( "  viseme timedelay <timedelay>" );
		LOG( "  viseme magnitude <amount>" );
		LOG( "  viseme <viseme name> <weight> <ramp in>" );
		LOG( "  viseme <viseme name> trap <weight> <dur> [<ramp-in> [<ramp-out>]]" );
		LOG( "  viseme <viseme name> curve <number of keys> <curve information>" );
		LOG( "  viseme curve" );
		LOG( "  viseme plateau on|off" );
		LOG( "  clampvisemes on|off" );
		LOG( "  minvisemetime <amount>" );
		LOG( "  bone" );
		LOG( "  bonep" );
		LOG( "  remove" );
		LOG( "  viewer" );
		LOG( "  gazefade in|out [<interval>]" );
		LOG( "  gazefade print" );
		LOG( "  reholster" );
		LOG( "  blink" );
		LOG( "  eyelid pitch <enable>" );
		LOG( "  eyelid range <min-angle> <max-angle> [<lower-min> <lower-max>]" );
		LOG( "  eyelid close <closed-angle>" );
		LOG( "  eyelid tight <upper-norm> [<lower-norm>]" );
		LOG( "  softeyes" );
		LOG( "  sk <file> <scale>");
		LOG( "  minibrain <on|off>");
		return( CMD_SUCCESS );
	}

	string char_cmd = args.read_token();
	if( char_cmd.length()==0 ) {
		LOG( "SbmCharacter::character_cmd_func: ERR: Expected character command." );
		return CMD_FAILURE;
	}

	bool all_characters = false;
	SbmCharacter* character = NULL;
	if( char_name == "*" ) {

		all_characters = true;
		std::vector<std::string> characters;
		for (std::map<std::string, SbmCharacter*>::iterator iter = mcu_p->getCharacterMap().begin();
			iter != mcu_p->getCharacterMap().end();
			iter++)
		{
			characters.push_back((*iter).second->getName());
		}

		for (std::vector<std::string>::iterator citer = characters.begin();
			citer != characters.end();
			citer++)
		{
			srArgBuffer copy_args( args.peek_string() );
			character = mcu_p->getCharacter( *citer );
			int err = character->parse_character_command( char_cmd, copy_args, true );
			if( err != CMD_SUCCESS )
				return( err );
		}
		return( CMD_SUCCESS );
	} 

	character = mcu_p->getCharacter( char_name );
	if( character ) {

		int err = character->parse_character_command( char_cmd, args, false );
		if( err != CMD_NOT_FOUND )	{
			return( err );
		}
	}

	// Commands for uninitialized characters:
	if( char_cmd == "init" ) {

		char* skel_file = args.read_token();
		char* type = args.read_token();
		return(	
			mcu_character_init( char_name.c_str(), skel_file, type, mcu_p )
			);
	} 
	else
		if( char_cmd == "param" ) {

			char* param_name = args.read_token();
			GeneralParam * new_param = new GeneralParam;
			new_param->size = args.read_int();

			if( new_param->size == 0 )
			{
				LOG("SbmCharacter::parse_character_command: param_registeration ERR: parameter size not defined!\n");
				delete new_param;
				return( CMD_FAILURE );
			}
			for(int i = 0 ; i < (int)new_param->char_names.size(); i++)
			{
				if(char_name == new_param->char_names[i])
				{
					LOG("SbmCharacter::parse_character_command: param_registeration ERR: parameter redefinition!\n");
					delete new_param;
					return( CMD_FAILURE );	
				}
			}
			new_param->char_names.push_back( char_name );
			GeneralParamMap::iterator it; 
			if( (it = mcu_p->param_map.find(param_name)) != mcu_p->param_map.end())
			{
				it->second->char_names.push_back( char_name );
				delete new_param;
			}
			else
			{
				mcu_p->param_map.insert(make_pair(string(param_name),new_param));
			}
			return( CMD_SUCCESS );
		}

		LOG( "SbmCharacter::character_cmd_func ERR: char '%s' or cmd '%s' NOT FOUND", char_name.c_str(), char_cmd.c_str() );
		return( CMD_FAILURE );
}

int create_remote_pawn_func( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	std::string pawn_and_attribute = args.read_token();
	int interval = args.read_int();

	if( pawn_and_attribute.length()==0 ) {
		LOG("ERROR: Expected pawn name.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn_p = NULL;

	pawn_p = mcu_p->getPawn( pawn_and_attribute );

	if( pawn_p != NULL ) {
		LOG("ERROR: Pawn \"%s\" already exists.", pawn_and_attribute.c_str() );
		return CMD_FAILURE;
	}

	pawn_p = new SmartBody::SBPawn( pawn_and_attribute.c_str() );

	SkSkeleton* skeleton = new SmartBody::SBSkeleton();
	skeleton->ref();
	std::string skel_name = pawn_and_attribute+"-skel";
	skeleton->name( skel_name.c_str() );
	// Init channels
	skeleton->make_active_channels();	

	if (mcu_p->sbm_character_listener)
	{
		mcu_p->sbm_character_listener->OnCharacterChanged(pawn_and_attribute);
	}

	int err = pawn_p->init( skeleton );

	if( err != CMD_SUCCESS ) {
		LOG("ERROR: Unable to initialize SbmPawn \"%s\".", pawn_and_attribute.c_str() );
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	err = mcu_p->addPawn( pawn_p );

	if( err != CMD_SUCCESS )	{
		LOG("ERROR: SbmPawn pawn_map.insert(..) \"%s\" FAILED", pawn_and_attribute.c_str() );
		delete pawn_p;
		skeleton->unref();
		return err;
	}

	if( err != CMD_SUCCESS )	{
		LOG("ERROR: SbmPawn pawn_map.insert(..) \"%s\" FAILED", pawn_and_attribute.c_str() );
		delete pawn_p;
		skeleton->unref();
		return err;
	}


#if USE_WSP
	mcu_p->theWSP->subscribe_vector_3d_interval( pawn_and_attribute, "position", interval, handle_wsp_error, remote_pawn_position_update, mcu_p );
	mcu_p->theWSP->subscribe_vector_4d_interval( pawn_and_attribute, "rotation", interval, handle_wsp_error, remote_pawn_rotation_update, mcu_p );
#endif

	return( CMD_SUCCESS );
}


int character_set_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	string character_id = args.read_token();
	if( character_id.length()==0 ) {
		LOG("ERROR: SbmCharacter::set_cmd_func(..): Missing character id.");
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->getCharacter( character_id );
	if( character==NULL ) {
		LOG("ERROR: SbmCharacter::set_cmd_func(..): Unknown character \"%s\" to set.", character_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmCharacter::set_cmd_func(..): Missing attribute to set.");
		return CMD_FAILURE;
	}

	//  voice_code and voice-code are backward compatible patches
	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		return set_voice_cmd_func( character, args );
	} else if( attribute == "voicebackup") {
		return set_voicebackup_cmd_func( character, args );
	} else {
		return set_attribute( character, attribute, args );
	}
}

int pawn_print_cmd_funcx( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	string pawn_id = args.read_token();
	if( pawn_id.length()==0 ) {
		LOG("ERROR: SbmPawn::print_cmd_func(..): Missing pawn id.");
		return CMD_FAILURE;
	}

	SbmPawn* pawn = mcu_p->getPawn( pawn_id );
	if( pawn==NULL ) {
		LOG("ERROR: SbmPawn::print_cmd_func(..): Unknown pawn \"%s\".", pawn_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmPawn::print_cmd_func(..): Missing attribute to print.");
		return CMD_FAILURE;
	}

	return print_attribute( pawn, attribute, args);
}

int character_print_cmd_func( srArgBuffer& args, mcuCBHandle* mcu_p)
{
	string character_id = args.read_token();
	if( character_id.length()==0 ) {
		LOG("ERROR: SbmCharacter::print_cmd_func(..): Missing character id.");
		return CMD_FAILURE;
	}

	SbmCharacter* character = mcu_p->getCharacter( character_id );
	if( character==NULL ) {
		LOG("ERROR: SbmCharacter::print_cmd_func(..): Unknown character \"%s\".", character_id.c_str());
		return CMD_FAILURE;
	}

	string attribute = args.read_token();
	if( attribute.length()==0 ) {
		LOG("ERROR: SbmCharacter::print_cmd_func(..): Missing attribute to print.");
		return CMD_FAILURE;
	}

	if( attribute=="voice" || attribute=="voice_code" || attribute=="voice-code" ) {
		//  Command: print character <character id> voice_code
		//  Print out the character's voice_id
		std::stringstream strstr;
		strstr << "character " << character_id << "'s voice_code: " << character->get_voice_code();
		LOG(strstr.str().c_str());
		return CMD_SUCCESS;
	} else if( attribute=="schedule" ) {
		return character->print_controller_schedules();
	} else {
		return print_attribute( character, attribute, args);
	}
}


// Print error on error..
bool parse_float_or_error( float& var, const char* str, const string& var_name )
{
	if( istringstream( str ) >> var )
		return true; // no error
	// else
	LOG("ERROR: Invalid value for %s: %s", var_name.c_str(), str);
	return false;
}


int set_world_offset_cmd( SbmPawn* pawn, srArgBuffer& args )
{
	float x, y, z, h, p, r;
	pawn->get_world_offset( x, y, z, h, p, r );

	bool has_error = false;
	string arg = args.read_token();
	if( arg.length() == 0 ) {
		LOG("ERROR: SbmPawn::set_world_offset: Missing offset parameters.");
		return CMD_FAILURE;
	}

	while( arg.length() > 0 ) {
		// TODO: handle "+x", "-x", etc...
		if( arg=="x" ) {
			has_error |= !parse_float_or_error( x, args.read_token(), arg );
		} else if( arg=="y" ) {
			has_error |= !parse_float_or_error( y, args.read_token(), arg );
		} else if( arg=="z" ) {
			has_error |= !parse_float_or_error( z, args.read_token(), arg );
		} else if( arg=="z" ) {
			has_error |= !parse_float_or_error( z, args.read_token(), arg );
		} else if( arg=="p" || arg=="pitch" ) {
			has_error |= !parse_float_or_error( p, args.read_token(), "pitch" );
		} else if( arg=="r" || arg=="roll" ) {
			has_error |= !parse_float_or_error( r, args.read_token(), "roll" );
		} else if( arg=="h" || arg=="heading" || arg=="yaw" ) {
			has_error |= !parse_float_or_error( h, args.read_token(), "yaw" );
		} else if( arg=="xyz" || arg=="pos" || arg=="position" ) {
			has_error |= !parse_float_or_error( x, args.read_token(), "x" );
			has_error |= !parse_float_or_error( y, args.read_token(), "y" );
			has_error |= !parse_float_or_error( z, args.read_token(), "z" );
		} else if( arg=="hpr" ) {
			has_error |= !parse_float_or_error( h, args.read_token(), "heading" );
			has_error |= !parse_float_or_error( p, args.read_token(), arg );
			has_error |= !parse_float_or_error( r, args.read_token(), arg );
		} else {
			LOG("ERROR: Unknown world_offset attribute \"%s\".", arg.c_str());
			has_error = true;
		}
		arg = args.read_token();
	}

	if( has_error )
		return CMD_FAILURE;

	pawn->set_world_offset( x, y, z, h, p, r );
	return CMD_SUCCESS;
}

int pawn_parse_pawn_command( SbmPawn* pawn, std::string cmd, srArgBuffer& args)
{
	mcuCBHandle* mcu_p = &mcuCBHandle::singleton();

	if (cmd == "remove")
	{	
		mcu_p->_scene->removePawn(pawn->getName());
		return CMD_SUCCESS;
	}
	else if (cmd == "prune")
	{
		int result = pawn->prune_controller_tree();
		if( result != CMD_SUCCESS )
		{
			LOG("ERROR: Failed to prune pawn \"%s\"", pawn->getName().c_str());
			return CMD_FAILURE;
		}
		else 
		{
			return CMD_SUCCESS;
		}
	}
	else if (cmd == "setshape")
	{
		std::string geom_str = "box", color_str = "red", file_str = "", type_str = "";
		bool setRec = false;
		bool has_geom = false;
		SrVec size = SrVec(1.f,1.f,1.f);		
		while( args.calc_num_tokens() > 0 )
		{
			string option = args.read_token();
			// TODO: Make the following option case insensitive
			if( option=="geom" ) {
				geom_str = args.read_token();
				has_geom = true;
			} else if( option=="size" ) {
				//size_str = args.read_token();
				float uniformSize = args.read_float();
				for (int i=0;i<3;i++)
					size[i] = uniformSize;//args.read_float();
				has_geom = true;

			} else if (option=="file" ) {
				file_str = args.read_token();
				has_geom = true;			
			} else if( option=="color" ) {
				color_str = args.read_token();
				has_geom = true;
			} else if( option=="type" ) {
				type_str = args.read_token();
				has_geom = true;
			} else if( option=="rec" ) {
				setRec = true;
				size[0] = pawn->steeringSpaceObjSize.x = args.read_float();
				size[1] = pawn->steeringSpaceObjSize.y = args.read_float();
				size[2] = pawn->steeringSpaceObjSize.z = args.read_float();

				has_geom = true;
			} else {
				std::stringstream strstr;
				strstr << "WARNING: Unrecognized pawn setshape option \"" << option << "\"." << endl;
				LOG(strstr.str().c_str());
			}
		}	

		if (has_geom)
		{				
			//initGeomObj(geom_str.c_str(),size,color_str.c_str(),file_str.c_str());
			SBPhysicsManager* phyManager = mcu_p->_scene->getPhysicsManager();
			phyManager->createPhysicsPawn(pawn->getName(),geom_str,size);
			// init steering space
			if (!setRec)
				pawn->steeringSpaceObjSize = size;//SrVec(size, size, size);
			if (type_str == "steering")
				pawn->initSteeringSpaceObject();
			return CMD_SUCCESS;
		}
		else
		{
			LOG("Pawn %s, fail to setshape. Incorrect parameters.", pawn->getName().c_str());
			return CMD_FAILURE;
		} 
	}
	else if (cmd == "physics")
	{
		string option = args.read_token();

		bool turnOn = false;
		if (option == "on" || option == "ON")
			turnOn = true;			
		else if (option == "off" || option == "OFF")
			turnOn = false;			
		else
			return CMD_FAILURE;

		SbmPhysicsObj* phyObj = pawn->getPhysicsObject();
		if (phyObj) phyObj->enablePhysicsSim(turnOn);

		//setPhysicsSim(turnOn);
		return CMD_SUCCESS;
	}
	else if (cmd == "collision")
	{	
		string option = args.read_token();
		bool turnOn = false;
		if (option == "on" || option == "ON")
			turnOn = true;			
		else if (option == "off" || option == "OFF")
			turnOn = false;			
		else
			return CMD_FAILURE;

		SbmPhysicsObj* phyObj = pawn->getPhysicsObject();
		if (phyObj) phyObj->enableCollisionSim(turnOn);

		//setCollision(turnOn);			
		return CMD_SUCCESS;
	}
	else
	{
		return CMD_FAILURE;
	}
}
