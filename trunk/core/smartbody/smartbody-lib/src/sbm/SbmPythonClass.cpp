#include "SbmPythonClass.h"
#include "sbm/me_ct_reach.hpp"

namespace SmartBody 
{

#ifdef USE_PYTHON


std::string PyLogger::strBuffer = "";

Script::Script()
{
}

Script::Script(std::string seqName)
{
	seq = seqName;
}

Script::~Script()
{
}

void Script::print()
{
	if (type == "seq")
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		srCmdSeq* seq_p = mcu.lookup_seq(seq.c_str());
		if( seq_p == NULL )	
		{
			LOG( "%s not exist", seq.c_str()); 
			return;
		}
		// Note: may need to use LOG instead of printing to stdout
		seq_p->print(stdout);
	}
	else
		LOG ("py file doesn't support abort function");
}

void Script::run()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	int err = 0;

	if (type == "seq")
	{
		srCmdSeq *seq_p = mcu.lookup_seq(seq.c_str());
		if (seq_p)
		{	
			srCmdSeq *cp_seq_p = new srCmdSeq;
			preProcessingScript(cp_seq_p, seq_p);
			cp_seq_p->offset((float)(mcu.time));

			if (!mcu.activeSequences.getSequence(seq))
			{
				mcu.activeSequences.addSequence(seq, cp_seq_p);
			}
			else
			{
				LOG("Unable to run a script '%s': this file cannot be inserted to the active sequences.", seq.c_str()); 
			}
			
		}
		else
		{
			LOG("Unable to run a script '%s': this file cannot be found.", seq.c_str()); 
		}
	}
	else if (type == "py")
	{
		std::string cmd = "execfile(\"" + seq + "\")";
		mcu.executePython(cmd.c_str());
	}

}

void Script::abort()
{
	if (type == "seq")
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		int result = mcu.abortSequence(seq.c_str());
		if (result == 0)
			LOG("'%s' not found, cannot abort.", seq.c_str()); 
	}
	else
		LOG ("py file doesn't support abort function");
}

void Script::preProcessingScript(srCmdSeq *to_seq_p, srCmdSeq *fr_seq_p)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	float t;
	char *cmd;	
	fr_seq_p->reset();
	while (cmd = fr_seq_p->pull(&t))	
	{
		srArgBuffer args(cmd);
		srCmdSeq *inline_seq_p = NULL;
		char *tok = args.read_token();
		if (strcmp(tok, "path") == 0)	
		{
			char *path_tok = args.read_token();
			char* path = args.read_token();
			addAssetPath(std::string(path_tok), std::string(path));
			delete [] cmd;
			cmd = NULL;
		}
		else if (strcmp(tok, "seq") == 0)
		{
			char *name = args.read_token();
			tok = args.read_token();
			if (strcmp(tok, "inline") == 0)
			{
				inline_seq_p = mcu.lookup_seq(name);
				delete [] cmd;
				cmd = NULL;
				if (inline_seq_p == NULL)	
				{
					LOG("Error when pre-process script '%s': inline seq '%s' not found.", seq.c_str(), name);
					return;
				}
			}
		}
		
		float absolute_offset = fr_seq_p->offset() + t;
		if( inline_seq_p )	{
			// iterate hierarchy
			inline_seq_p->offset(absolute_offset);
			preProcessingScript(to_seq_p, inline_seq_p);
		}
		else if (cmd)	
			to_seq_p->insert_ref(absolute_offset, cmd);
	}
	delete fr_seq_p;
}


Profiler::Profiler()
{
}

Profiler::~Profiler()
{
}

void Profiler::printLegend()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.profiler_p)	
		mcu.profiler_p->print_legend();
	else
		LOG("Profiler does not exist!");
}

void Profiler::printStats()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.profiler_p)	
		mcu.profiler_p->print();
	else
		LOG("Profiler does not exist!");
}

SimulationManager::SimulationManager()
{
}

SimulationManager::~SimulationManager()
{
}

void SimulationManager::printInfo()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->print();
	else	
	{
		LOG( "TIME:%.3f ~ DT:%.3f %.2f:FPS\n",
			mcu.time,
			mcu.time_dt,
			1.0 / mcu.time_dt
		);
	}
}

void SimulationManager::printPerf(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
	{
		if (v > 0.0) 
			mcu.timer_p->set_perf(v);
		else	
			mcu.timer_p->set_perf(10.0);	
	}
	else
		LOG("Time regulator not exist!");
}

double SimulationManager::getTime()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu.time;
}

bool SimulationManager::isStarted()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
		return mcu.timer_p->isStarted();
	else
		return false;
}

bool SimulationManager::isRunning()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
		return mcu.timer_p->isRunning();
	else
		return false;
}

void SimulationManager::reset()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->reset();
	else
		LOG("Time regulator not exist!");
}

void SimulationManager::start()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->start();
	else
		LOG("Time regulator not exist!");
}

void SimulationManager::pause()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->pause();
	else
		LOG("Time regulator not exist!");
}

void SimulationManager::resume()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->resume();
	else
		LOG("Time regulator not exist!");
}

void SimulationManager::step(int n)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)
	{
		if (n)
			mcu.timer_p->step(n);
		else
			mcu.timer_p->step(1);
	}
	else
		LOG("Time regulator not exist!");
}

void SimulationManager::setSleepFps(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sleep_fps(v);
}

void SimulationManager::setEvalFps(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_eval_fps(v);
}

void SimulationManager::setSimFps(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sim_fps(v);
}

void SimulationManager::setSleepDt(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sleep_dt(v);
}

void SimulationManager::setEvalDt(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_eval_dt(v);
}

void SimulationManager::setSimDt(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!mcu.timer_p)	
	{
		LOG("Time regulator not exist!");
		return;
	}
	mcu.timer_p->set_sim_dt(v);
}

void SimulationManager::setSpeed(float v)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.timer_p)	
		mcu.timer_p->set_speed(v);
	else
		LOG("Time regulator not exist!");
}

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::printInfo()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::cout << "   Camera Info " << std::endl;
	std::cout << "-> eye position:	" << "(" << mcu.camera_p->eye.x << ", " << mcu.camera_p->eye.y << ", " << mcu.camera_p->eye.z << ")" << std::endl;
	std::cout << "-> center position:	" << "(" << mcu.camera_p->center.x << ", " << mcu.camera_p->center.y << ", " << mcu.camera_p->center.z << ")" << std::endl;
	std::cout << "-> up vector:		" << "(" << mcu.camera_p->up.x << ", " << mcu.camera_p->up.y << ", " << mcu.camera_p->up.z << ")" << std::endl;
	std::cout << "-> fovy:		" << mcu.camera_p->fovy << std::endl;
	std::cout << "-> near plane:		" << mcu.camera_p->znear << std::endl;
	std::cout << "-> far plane:		" << mcu.camera_p->zfar << std::endl;
	std::cout << "-> aspect:		" << mcu.camera_p->aspect << std::endl;
	std::cout << "-> scale:		" << mcu.camera_p->scale << std::endl;
}


void Camera::setEye(float x, float y, float z)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.camera_p->eye.x = x;
	mcu.camera_p->eye.y = y;
	mcu.camera_p->eye.z = z;
	mcu.viewer_p->set_camera(*mcu.camera_p);
}

void Camera::setCenter(float x, float y, float z)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.camera_p->center.x = x;
	mcu.camera_p->center.y = y;
	mcu.camera_p->center.z = z;
	mcu.viewer_p->set_camera(*mcu.camera_p);
}

void Camera::setScale(float s)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.camera_p->scale = s;
	mcu.viewer_p->set_camera(*mcu.camera_p);
}

void Camera::reset()
{
	setEye(0, 166, 185);
	setCenter(0, 92, 0);
}

void Camera::setDefault(int preset)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (preset == 1)
		mcu.viewer_p->view_all();
	else
		LOG("defaultCamera func option not valid.");
}

void Camera::setTrack(std::string cName, std::string jName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(cName);
	if (!pawn)
	{
		LOG("Object %s was not found, cannot track.", cName.c_str());
		return;
	}
	if (jName == "")
	{
		LOG("Need to specify a joint to track.");
		return;
	}

	SkSkeleton* skeleton = NULL;
	skeleton = pawn->getSkeleton();

	SkJoint* joint = pawn->getSkeleton()->search_joint(jName.c_str());
	if (!joint)
	{
		LOG("Could not find joint %s on object %s.", jName.c_str(), cName.c_str());
		return;
	}

	joint->skeleton()->update_global_matrices();
	joint->update_gmat();
	const SrMat& jointMat = joint->gmat();
	SrVec jointPos(jointMat[12], jointMat[13], jointMat[14]);
	CameraTrack* cameraTrack = new CameraTrack();
	cameraTrack->joint = joint;
	cameraTrack->jointToCamera = mcu.camera_p->eye - jointPos;
	LOG("Vector from joint to target is %f %f %f", cameraTrack->jointToCamera.x, cameraTrack->jointToCamera.y, cameraTrack->jointToCamera.z);
	cameraTrack->targetToCamera = mcu.camera_p->eye - mcu.camera_p->center;
	LOG("Vector from target to eye is %f %f %f", cameraTrack->targetToCamera.x, cameraTrack->targetToCamera.y, cameraTrack->targetToCamera.z);				
	mcu.cameraTracking.push_back(cameraTrack);
	LOG("Object %s will now be tracked at joint %s.", cName.c_str(), jName.c_str());
}

void Camera::removeTrack()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.cameraTracking.size() > 0)
	{
		for (std::vector<CameraTrack*>::iterator iter = mcu.cameraTracking.begin();
			 iter != mcu.cameraTracking.end();
			 iter++)
		{
			CameraTrack* cameraTrack = (*iter);
			delete cameraTrack;
		}
		mcu.cameraTracking.clear();
		LOG("Removing current tracked object.");
	}
}

GazeBML::GazeBML()
{
}

GazeBML::~GazeBML()
{
}

void GazeBML::setSpeed(boost::python::list &input)
{
	speed.clear();
	for (int i = 0; i < len(input); ++i)
		speed.push_back(boost::python::extract<std::string>(input[i]));
}

boost::python::list GazeBML::getSpeed()
{
	boost::python::list ret;
	for (size_t i = 0; i < speed.size(); i++)
		ret.append(speed[i]);
	return ret;
}

void GazeBML::setSmoothing(boost::python::list &input)
{
	smoothing.clear();
	for (int i = 0; i < len(input); ++i)
		smoothing.push_back(boost::python::extract<std::string>(input[i]));
}

boost::python::list GazeBML::getSmoothing()
{
	boost::python::list ret;
	for (size_t i = 0; i < smoothing.size(); i++)
		ret.append(smoothing[i]);
	return ret;
}

std::string GazeBML::buildGazeBML()
{
	std::string targetAttr;
	std::string directionAttr;
	std::string angleAttr;
	std::string speedAttr;
	std::string smoothAttr;

	if (target != "")
	{
		targetAttr = "target=\"";
		targetAttr += target;
		targetAttr += "\" ";
	}
	if (direction != "")
	{
		directionAttr = "direction=\"";
		directionAttr += direction;
		directionAttr += "\" ";		
	}
	if (angle != "")
	{
		angleAttr = "angle=\"";
		angleAttr += angle;
		angleAttr += "\" ";	
	}
	if (speed.size() == 3)
	{
		speedAttr = "sbm:joint-speed=\"";
		speedAttr += speed[0];
		speedAttr += " ";
		speedAttr += speed[1];
		speedAttr += " ";
		speedAttr += speed[2];
		speedAttr += "\" ";		
	}
	if (smoothing.size() == 3)
	{
		smoothAttr = "sbm:speed-smoothing=\"";
		smoothAttr += smoothing[0];
		smoothAttr += " ";
		smoothAttr += smoothing[1];
		smoothAttr += " ";
		smoothAttr += smoothing[2];
		smoothAttr += "\" ";		
	}

	std::ostringstream bml;
	// First half of BML
	bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<act>\n"
		<< "\t<bml>\n"
		<< "\t\t<gaze " << targetAttr << directionAttr << angleAttr << speedAttr << smoothAttr << "/>\n"
		<< "\t</bml>\n"
		<< "</act>";
	return bml.str();
}

BmlProcessor::BmlProcessor()
{
}

BmlProcessor::~BmlProcessor()
{
}

// This command is inside bml_processor.cpp, unlike most of other commands inside mcontrol_util. So unable to rewrite, instead, re-routine to bp.
void BmlProcessor::vrSpeak(std::string agent, std::string recip, std::string msgId, std::string msg)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::stringstream msgStr;
	msgStr << agent << " " << recip << " " << msgId << " " << msg;
	srArgBuffer vrMsg(msgStr.str().c_str());
	BML::Processor& bp = mcu.bml_processor;
	bp.vrSpeak_func(vrMsg, &mcu);
}

void BmlProcessor::vrAgentBML(std::string op, std::string agent, std::string msgId, std::string msg)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (op == "request" || op == "start" || op == "end")
	{
		std::stringstream msgStr;
		msgStr << agent << " " << msgId << " " << op << " " << msg;
		srArgBuffer vrMsg(msgStr.str().c_str());
		BML::Processor& bp = mcu.bml_processor;
		bp.vrAgentBML_cmd_func(vrMsg, &mcu);
	}
	else
	{
		LOG("vrAgentBML option %s not recognized!", op.c_str());
		return;	
	}
}

void BmlProcessor::build_vrX(std::ostringstream& buffer, const std::string& cmd, const std::string& char_id, const std::string& recip_id, const std::string& content, bool for_seq ) 
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	static int test_bml_id = 0;

	buffer.str("");
	if( for_seq )
		buffer << "send " << cmd << " ";
	buffer << char_id << " "<< recip_id << " sbm";
	if( mcu.process_id != "" )  // Insert process_id if present.
		buffer << '_' << mcu.process_id; 
	buffer << "_test_bml_" << (++test_bml_id) << std::endl << content;
}

void BmlProcessor::send_vrX( const char* cmd, const std::string& char_id, const std::string& recip_id,
			const std::string& seq_id, bool echo, bool send, const std::string& bml ) 
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::ostringstream msg;

	bool all_characters = ( char_id=="*" );

	if( seq_id.length()==0 ) {
		if( echo ) {
			build_vrX( msg, cmd, char_id, recip_id, bml, false );
			LOG("%s %s", cmd, msg.str().c_str());
		}

		if( send ) {
			// execute directly
			if( all_characters ) {
				for(std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
					iter != mcu.getCharacterMap().end();
					iter++)
				{
					SbmCharacter* character = (*iter).second;
					build_vrX( msg, cmd, character->getName().c_str(), recip_id, bml, false );
					mcu.vhmsg_send( cmd, msg.str().c_str() );
				}
			} else {
				build_vrX( msg, cmd, char_id, recip_id, bml, false );
				mcu.vhmsg_send( cmd, msg.str().c_str() );
			}
		}
		return;
	}else {
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
			for(std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
				iter != mcu.getCharacterMap().end();
				iter++)
			{
				SbmCharacter* character = (*iter).second;
				build_vrX( msg, cmd, character->getName().c_str(), recip_id, bml, true );
				if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
					std::stringstream strstr;
					strstr << "WARNING: send_vrX(..): Failed to insert vrSpeak command for character \"" << char_id << "\".";
					LOG(strstr.str().c_str());
				}
			}
		} else {
			build_vrX( msg, cmd, char_id, recip_id, bml, true );
			if( seq->insert( 0, msg.str().c_str() )!=CMD_SUCCESS ) {
				std::stringstream strstr;
				strstr << "WARNING: send_vrX(..): Failed to insert vrSpeak command for character \"" << char_id << "\".";
				LOG(strstr.str().c_str());
			}
		}

		if( send ) {
			mcu.activeSequences.removeSequence(seq_id, true); // remove old sequence by this name
			if( !mcu.activeSequences.addSequence(seq_id, seq ))
			{
				std::stringstream strstr;
				strstr << "ERROR: send_vrX(..): Failed to insert seq into active sequences.";
				LOG(strstr.str().c_str());
				return;
			}
		} else {
			mcu.pendingSequences.removeSequence(seq_id, true);  // remove old sequence by this name
			if (mcu.pendingSequences.addSequence(seq_id, seq))
			{
				std::stringstream strstr;
				strstr << "ERROR: send_vrX(..): Failed to insert seq into pending sequences.";
				LOG(strstr.str().c_str());
				return;
			}
		}
		return;
	}
}


// Should I put echo and send flag as input parameters? How to support default parameters
void BmlProcessor::execAnimation(std::string character, std::string anim)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkMotion*>::iterator motionIter = mcu.motion_map.find(anim);
	if (motionIter == mcu.motion_map.end()) {
		LOG("WARNING: Unknown animation \"%s\".", anim.c_str());
	}

	std::ostringstream bml;
	bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<act>\n"
		<< "\t<bml>\n"
		<< "\t\t<sbm:animation name=\"" << anim << "\"/>\n"
		<< "\t</bml>\n"
		<< "</act>";
	send_vrX( "vrSpeak", character, "ALL", "", true, true, bml.str() );
}

void BmlProcessor::execPosture(std::string character, std::string posture)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkPosture*>::iterator postureIter = mcu.pose_map.find(posture);
	if (postureIter == mcu.pose_map.end()) {
		LOG("WARNING: Unknown posture \"%s\".", posture.c_str());
	}

	std::ostringstream bml;
	bml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<act>\n"
		<< "\t<bml>\n"
		<< "\t\t<body posture=\"" << posture << "\"/>\n"
		<< "\t</bml>\n"
		<< "</act>";
	send_vrX( "vrSpeak", character, "ALL", "", true, true, bml.str() );
}

void BmlProcessor::execGaze(std::string character, GazeBML& gazeBML)
{
	std::string bml = gazeBML.buildGazeBML();
	if (gazeBML.getTarget() == "" || character == "")
	{
		LOG("execGaze Failure: Gazing character and target have to be defined both.");
		return;
	}
	send_vrX( "vrSpeak", character, "ALL", "", true, true, bml );
}

void BmlProcessor::execBML(std::string character, std::string bml)
{
	std::ostringstream entireBml;
	entireBml	<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				<< "<act>\n"
				<< "\t<bml>\n"
				<< "\t\t" << bml
				<< "\t</bml>\n"
				<< "</act>";	
	send_vrX( "vrSpeak", character, "ALL", "", true, true, entireBml.str() );
}

Viseme::Viseme()
{
}

Viseme::~Viseme()
{
}

void Viseme::setWeight(float weight, float dur, float rampin, float rampout)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(charName);
	if (character)
	{
		if (dur < 0.0)		dur = 0.0f;
		if (rampin < 0.0)	rampin = 0.1f;
		if (rampout < 0.0)	rampout = 0.1f;
		character->schedule_viseme_trapezoid( visemeName.c_str(), mcu.time, weight, dur, rampin, rampout );
	}
}

void Viseme::setCurve(int numKeys, boost::python::list weights)
{
	if( numKeys <= 0 )	
	{
		LOG( "Viseme data is missing" );
		return;
	}

	int num_remaining = len(weights);
	int numKeyParams = num_remaining / numKeys;
	if (num_remaining != numKeys * numKeyParams)
	{
		LOG( "Viseme data is malformed" );
		return;
	}
	float* curveInfo = new float[num_remaining];
	for (int i = 0; i < num_remaining; i++)
		curveInfo[i] = boost::python::extract<float> (weights[i]);

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(charName);
	if (character)
	{
		character->schedule_viseme_blend_curve( visemeName.c_str(), mcu.time, 1.0f, curveInfo, numKeys, numKeyParams );
	}

	delete [] curveInfo;
}

Motion::Motion()
{
	motionFile = "";
}

Motion::Motion(std::string file)
{
	motionFile = file;
}

Motion::~Motion()
{
	motionFile = "";
}

const std::string& Motion::getMotionFileName()
{
	SkMotion* skMotion = getSkMotion();
	if (skMotion)
		return skMotion->filename();
	else
		return motionFile;
}

const std::string& Motion::getMotionName()
{
	SkMotion* skMotion = getSkMotion();
	if (skMotion)
		return skMotion->name();
	else
		return emptyString;
}

int Motion::getNumFrames()
{
	SkMotion* skMotion = getSkMotion();
	if (skMotion)
		return skMotion->frames();
	else
		return 0;
}

boost::python::list Motion::getFrameData(int frameId)
{
	SkMotion* skMotion = getSkMotion();
	if (skMotion)
	{
		boost::python::list ret;
		for (int i = 0; i < getFrameSize(); i++)
			ret.append(skMotion->posture(frameId)[i]);
		return ret;
	}
	else
		return boost::python::list();
}

int Motion::getFrameSize()
{
	SkMotion* skMotion = getSkMotion();
	if (skMotion)
	{
		return skMotion->posture_size();
	}
	else
		return 0;
}

int Motion::getNumChannel()
{
	SkMotion* skMotion = getSkMotion();
	if (!skMotion)
		return 0;
	
	return skMotion->channels().size();
}

boost::python::list Motion::getChannels()
{
	SkMotion* skMotion = getSkMotion();
	if (!skMotion)
		return boost::python::list();

	if (skMotion->connected_skeleton() == NULL)
	{
		LOG("Motion not connected to a skeleton, cannot retrieve channels.");
		return boost::python::list();
	}
	boost::python::list ret;
	SkChannelArray channels = skMotion->channels();
	for (int i = 0; i < channels.size(); i++)
	{
		std::string chanName = channels[i].joint->name().c_str();
		int	chanType = channels[i].type;
		std::string chanTypeString;
		switch (chanType)
		{
			case 0:
				chanTypeString = "XPos";
				break;
			case 1:	
				chanTypeString = "YPos";
				break;
			case 2:
				chanTypeString = "ZPos";
				break;
			case 6:
				chanTypeString = "Quat";
				break;
			default:
				chanTypeString = "Others";
		}
		std::string name = chanName + " " + chanTypeString;
		ret.append(name);
	}
	return ret;
}

void Motion::checkSkeleton(std::string skel)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int chanSize;
	SkChannel chan;

	SkMotion* motion;
	std::map<std::string, SkMotion*>::iterator motionIter = mcu.motion_map.find(getMotionName().c_str());
	if (motionIter != mcu.motion_map.end())
		motion = motionIter->second;
	else
	{
		LOG("checkSkeleton ERR: Motion %s NOT EXIST!", getMotionName().c_str());
		return;
	}

	SkSkeleton* skSkel = load_skeleton(skel.c_str(), mcu.me_paths, mcu.resource_manager, mcu.skScale);
	if (skSkel)
	{
		int numValidChannels = motion->connect(skSkel);	// connect and check for the joints
		SkChannelArray& mChanArray = motion->channels();
		int mChanSize = mChanArray.size();
		SkChannelArray& skelChanArray = skSkel->channels();
		int skelChanSize = skelChanArray.size();
		chanSize = mChanSize;
		LOG("Channels in skeleton %s's channel matching motion %s's channel are preceeded with '+'", skel.c_str(), getMotionName().c_str());
		LOG("motion %s's Channel Info:", getMotionName().c_str());
		LOG("Channel Size: %d", chanSize);
		for (int i = 0; i < chanSize; i++)
		{				
			std::stringstream outputInfo;
			chan = mChanArray[i];
			std::string jointName = chan.joint->name().c_str();
			int	chanType = chan.type;
			std::string chanTypeString;
			switch (chanType)
			{
				case 0:
					chanTypeString = "XPos";
					break;
				case 1:	
					chanTypeString = "YPos";
					break;
				case 2:
					chanTypeString = "ZPos";
					break;
				case 6:
					chanTypeString = "Quat";
					break;
				default:
					chanTypeString = "Others";
			}
			int pos;
			pos = skelChanArray.linear_search(chan.joint->name(), chan.type);
			if (pos != -1)
				outputInfo << "+ ";
			if (pos == -1)	
				outputInfo << "  ";
			outputInfo << i << ": " << jointName.c_str() << " (" << chanTypeString << ")";
			LOG("%s", outputInfo.str().c_str());
		}
	}
	else
		LOG("Skeleton %s NOT EXIST!", skel.c_str());
}

SkMotion* Motion::getSkMotion()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkMotion*>::iterator motionIter = mcu.motion_map.find(motionFile);
	if (motionIter != mcu.motion_map.end())
	{
		SkMotion* motion = (*motionIter).second;
		return motion;
	}
	else
		return NULL;
}

void Motion::connect(SBSkeleton* skel)
{
//	SkSkeleton* skeleton = skel->getSkSkeleton(skel->getInstanceId());
//	SkMotion* skMotion = getSkMotion();
//	if (skeleton && skMotion)
//		skMotion->connect(skeleton);
}

void Motion::disconnect()
{
	SkMotion* skMotion = getSkMotion();
	if (skMotion)
		skMotion->disconnect();
}


int init_motion_controller( 
	const char *ctrl_name, 
	const char *mot_name, 
	mcuCBHandle *mcu_p
)	{
	int err = CMD_SUCCESS;

	std::map<std::string, SkMotion*>::iterator motionIter =  mcu_p->motion_map.find(mot_name);
	if( motionIter == mcu_p->motion_map.end() ) {
		LOG( "init_motion_controller ERR: SkMotion '%s' NOT FOUND in motion map\n", mot_name ); 
		return( CMD_FAILURE );
	}
	SkMotion *mot_p = (*motionIter).second;

	MeCtMotion* ctrl_p = new MeCtMotion;
	/*
	err = mcu_p->motion_ctrl_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_motion_controller ERR: MeCtMotion '%s' EXISTS\n", ctrl_name ); 
		delete ctrl_p;
		return( CMD_FAILURE );
	}
	*/
	ctrl_p->ref();

	err = mcu_p->controller_map.insert( ctrl_name, ctrl_p );
	if( err == CMD_FAILURE )	{
		LOG( "init_motion_controller ERR: MeController '%s' EXISTS\n", ctrl_name ); 
		return( CMD_FAILURE );
	}
	ctrl_p->ref();

	ctrl_p->setName( ctrl_name );
	ctrl_p->init( NULL, mot_p );
	return( CMD_SUCCESS );
}

void pythonExit()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.use_python = false;
}

void quitSbm()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.loop = false;
}

void reset()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.reset();
}


void command(const std::string& command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.execute((char*) command.c_str());
}

void commandAt(float seconds, const std::string& command)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.execute_later((char*) command.c_str(), seconds);
}


void printLog(const std::string& message)
{
	//LOG("Haha I am printing python log");
	LOG(message.c_str());
}

void runScript(std::string script)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.executePythonFile(script.c_str());
}

void setDefaultCharacter(const std::string& character)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.test_character_default = character;
}

void setDefaultRecipient(const std::string& recipient)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.test_recipient_default = recipient;
}


void sendVHMsg(std::string message)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.vhmsg_send(message.c_str());
}

void sendVHMsg2(std::string message, std::string message2)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.vhmsg_send(message.c_str(), message2.c_str());
}

int getNumCharacters() 
{  
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	return mcu.getNumCharacters(); 
}

int getNumPawns() 
{  
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	LOG("Python calls getNumPawns");
	return mcu.getNumPawns() - mcu.getNumCharacters(); 
}

EventManager* getEventManager()
{
	return EventManager::getEventManager();
}

boost::python::list getPawnNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	boost::python::list ret;

	for(std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		SbmCharacter* character = dynamic_cast<SbmCharacter*>(pawn);
		if (!character)
			ret.append(std::string(pawn->getName()));
	}

	return ret;
}

boost::python::list getCharacterNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	boost::python::list ret;

	for(std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* sbmCharacter = (*iter).second;
		ret.append(std::string(sbmCharacter->getName()));
	}

	return ret;
}

SBPawn* createPawn(std::string pawnName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	SbmPawn* pawn = mcu.getPawn(pawnName);
	SbmCharacter* character = dynamic_cast<SbmCharacter*>(pawn);
	if (character)
	{
		LOG("Pawn '%s' is a character.", pawnName.c_str());
		return NULL;
	}
	if (pawn)
	{
		LOG("Pawn '%s' already exists!", pawnName.c_str());
		return NULL;
	}
	else
	{
		SBPawn* pawn = new SBPawn(pawnName.c_str());
		return pawn;
	}
}

SBCharacter* createCharacter(std::string char_name, std::string metaInfo)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	SbmCharacter* character = mcu.getCharacter(char_name);
	if (character)
	{
		LOG("Character '%s' already exists!", char_name.c_str());
		return NULL;
	}
	else
	{
		SBCharacter* character = new SBCharacter(char_name, metaInfo);
		return character;
	}
}

SBSkeleton* createSkeleton(std::string skeletonDefinition)
{
	SBSkeleton* skeleton = new SBSkeleton(skeletonDefinition);
	return skeleton;
}

SBController* createController(std::string controllerType, std::string controllerName)
{
	SBController* controller = NULL;

	if (controllerType == "schedule")
	{
		controller = new MeCtScheduler2();
		
	}
	else if (controllerType == "gaze")
	{
		controller = new MeCtGaze();
	}
	else if (controllerType == "eyelid")
	{
		controller = new MeCtEyeLidRegulator();
	}
	else if (controllerType == "face")
	{
		controller = new MeCtFace();
	}
	else if (controllerType == "locomotion")
	{
		controller = new MeCtLocomotion();
	}
	else if (controllerType == "paramanimation")
	{
		controller = new MeCtParamAnimation();
	}
	else if (controllerType == "curvewriter")
	{
		controller = new MeCtCurveWriter();
	}

	if (controller)
		controller->setName(controllerName);

	return controller;
}

void removeCharacter(std::string charName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(charName);
	if (character)
	{
		SbmCharacter::remove_from_scene(charName.c_str());
	}	
}

FaceDefinition* getFaceDefinition(std::string def)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, FaceDefinition*>::iterator iter = mcu.face_map.find(def);
	if (iter == mcu.face_map.end())
	{
		FaceDefinition* faceDefinition = new FaceDefinition();
		faceDefinition->setName(def);
		mcu.face_map.insert(std::pair<std::string, FaceDefinition*>(def, faceDefinition));
		return faceDefinition;
	}
	else
	{
		return ((*iter).second);
	}

}

Camera* getCamera()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (mcu.camera_p)
	{
		Camera* camera = new Camera();
		return camera;
	}
	else
	{
		LOG("Camera not exists, returning NULL instead.");
		return NULL;
	}
}

SrViewer* getViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.viewer_p)
	{
		mcu.viewer_p = mcu.viewer_factory->create(100, 100, 640, 480);
		mcu.viewer_p->label_viewer("Visual Debugger");
		mcu.camera_p = new SrCamera();
		mcu.viewer_p->set_camera(*mcu.camera_p);
		mcu.viewer_p->root(mcu.root_group_p);
	}
	return mcu.viewer_p;
}

GenericViewer* getBmlViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.bmlviewer_p)
	{
		mcu.bmlviewer_p = mcu.bmlviewer_factory->create(100, 100, 640, 480);
		mcu.bmlviewer_p->label_viewer( "BML Viewer");	
	}
	return mcu.bmlviewer_p;
}

GenericViewer* getDataViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.channelbufferviewer_p)
	{
		mcu.channelbufferviewer_p = mcu.channelbufferviewer_factory->create(100, 100, 640, 480);
		mcu.channelbufferviewer_p->label_viewer( "Data Viewer");	
	}

	return mcu.channelbufferviewer_p;
}

GenericViewer* getPanimationViewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (!mcu.panimationviewer_p)
	{
		mcu.panimationviewer_p = mcu.panimationviewer_factory->create(100, 100, 640, 480);
		mcu.panimationviewer_p->label_viewer( "Parameterized Animation Viewer");	
	}

	return mcu.panimationviewer_p;
}

void execScripts(boost::python::list& input)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	std::vector<std::string> seq_names;
	for (int i = 0; i < len(input); ++i)
		seq_names.push_back(boost::python::extract<std::string>(input[i]));

	if (seq_names.empty())
	{
		LOG("ERROR: seq-chain expected one or more .seq filenames.");
		return;
	}
	
	mcu.execute_seq_chain(seq_names, "ERROR: seq-chain: ");
}

Script* getScript(std::string fileName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (fileName.substr(fileName.find_last_of(".") + 1) == "seq")
	{
		srCmdSeq *seq_p = mcu.lookup_seq(fileName.c_str());
		if (seq_p)
		{
			Script* script = new Script(fileName);
			script->setType("seq");
			return script;
		}
		else
		{
			LOG("Script %s not found. Returns Null.", fileName.c_str());
			return NULL;
		}
	}
	else if (fileName.substr(fileName.find_last_of(".") + 1) == "py")
	{
		std::string pythonFile = getScriptFromFile(fileName);
		if (pythonFile == "")
		{
			LOG("Script %s not found. Returns Null.", fileName.c_str());
			return NULL;
		}
		else
		{
			Script* script = new Script(pythonFile);
			script->setType("py");
			return script;
		}
	}
	else
	{
		LOG("Script %s's extension not recognized, please verify.", fileName.c_str());
		return NULL;
	}
}

void showCommandResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		CmdResource * res = dynamic_cast<CmdResource *>(mcu.resource_manager->getResource(r));
		if (res)
			LOG("%s", res->dump().c_str());
	}	
}

void showMotionResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		MotionResource * res = dynamic_cast<MotionResource  *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());

	}	
}


void showSkeletonResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		SkeletonResource * res = dynamic_cast<SkeletonResource  *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());

	}	
}

void showPathResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		PathResource * res = dynamic_cast<PathResource *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());
	}
}

void showScriptResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		FileResource * res = dynamic_cast<FileResource *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());
	}
}

void showControllerResources()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int numResources = mcu.resource_manager->getNumResources();

	for (int r = 0; r < numResources; r++)
	{
		ControllerResource * res = dynamic_cast<ControllerResource  *>(mcu.resource_manager->getResource(r));
		if(res)
			LOG("%s", res->dump().c_str());
	}	
}

void getResourceLimit()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.resource_manager->getLimit();
}

void setResourceLimit(int limit)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	if (limit > 0)
		mcu.resource_manager->setLimit(limit);
}

void addAssetPath(std::string type, std::string path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	PathResource* pres = new PathResource();
	pres->setPath(path);
	if (type == "seq")
	{
		pres->setType("seq");
		mcu.seq_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME")
	{
		pres->setType("me");
		mcu.me_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		pres->setType("audio");
		mcu.audio_paths.insert(const_cast<char *>(path.c_str()));
	}
	else if (type == "mesh")
	{
		pres->setType("mesh");
		mcu.mesh_paths.insert(const_cast<char *>(path.c_str()));
	}
	else
	{
		delete pres;
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}
	mcu.resource_manager->addResource(pres);
}

void removeAssetPath(std::string type, std::string path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 

	bool ret = false;
	if (type == "seq")
	{
		ret = mcu.seq_paths.remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "me" || type == "ME")
	{
		ret = mcu.me_paths.remove(const_cast<char *>(path.c_str()));
	}
	else if (type == "audio")
	{
		ret = mcu.audio_paths.remove(const_cast<char *>(path.c_str()));
	}
	else
	{
		LOG("Input type %s not recognized!", type.c_str());
		return;
	}

	if (ret)
	{
		// remove the resource from the resource manager
	}
}

void loadAssets()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.me_paths.reset();

	std::string path = mcu.me_paths.next_path();
	while (path != "")
	{
		mcu.load_motions(path.c_str(), true);
		mcu.load_skeletons(path.c_str(), true);
		path = mcu.me_paths.next_path();
	}
}

void addPose(std::string path, bool recursive)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.load_poses(path.c_str(), recursive);
}

void addMotion(std::string path, bool recursive)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	mcu.load_motions(path.c_str(), recursive);
}

void setMediaPath(std::string path)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.setMediaPath(path);
}

SimulationManager* getSimulationManager()
{
	SimulationManager* manager = new SimulationManager();
	return manager;
}

Profiler* getProfiler()
{
	Profiler* profiler = new Profiler();
	return profiler;
}

BmlProcessor* getBmlProcessor()
{
	BmlProcessor* processor = new BmlProcessor();
	return processor;
}

std::string getScriptFromFile(std::string fileName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	FILE* file_p = NULL;
	char buffer[ MAX_FILENAME_LEN ];
	char label[ MAX_FILENAME_LEN ];
	sprintf( label, "%s", fileName.c_str() );
	mcu.seq_paths.reset();
	std::string filename = mcu.seq_paths.next_filename( buffer, label );
	
	while (filename.size() > 0 && filename != "")	{
		file_p = fopen( filename.c_str(), "r" );
		if( file_p != NULL ) {
	
			// add the file resource
			FileResource* fres = new FileResource();
			std::stringstream stream;
			stream << filename;
			fres->setFilePath(stream.str());
			mcu.resource_manager->addResource(fres);
			
			break;
		}
		filename = mcu.seq_paths.next_filename( buffer, label );
	}
	if( file_p == NULL ) {
		// Could not find the file as named.  Perhap it excludes the extension	
		sprintf( label, "%s.py", fileName.c_str() );
		mcu.seq_paths.reset();
		filename = mcu.seq_paths.next_filename( buffer, label );
		while (filename.size() > 0)	{
			if( ( file_p = fopen( filename.c_str(), "r" ) ) != NULL ) {
				
				// add the file resource
				FileResource* fres = new FileResource();
				std::stringstream stream;
				stream << filename;
				fres->setFilePath(stream.str());
				mcu.resource_manager->addResource(fres);
				break;
			}
			filename = mcu.seq_paths.next_filename( buffer, label );
		}
	}

	// return empty string if file not found
	if (file_p == NULL)	return "";
	else				return std::string(filename);	
}

////////////////////////////////////////////
SBPawn* getPawn(std::string name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(name);
	if (pawn)
	{
		SBPawn* sbpawn = dynamic_cast<SBPawn*>(pawn);
		return sbpawn;
	}
	else
	{
		LOG("pawn %s does not exist.", name.c_str());
		return NULL;
	}
}

SBCharacter* getCharacter(std::string name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmCharacter* character = mcu.getCharacter(name);
	if (character)
	{
		SBCharacter* sbcharacter = dynamic_cast<SBCharacter*>(character);
		return sbcharacter;
	}
	else
	{
		LOG("Character %s does not exist.", name.c_str());
		return NULL;
	}
}

SkMotion* getMotion(std::string name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SkMotion* motion = mcu.getMotion(name);
	return motion;
}

void PyLogger::pa()
{
	strBuffer += "a";		
}

void PyLogger::pb()
{
	strBuffer += "b";		
}
void PyLogger::pc()
{
	strBuffer += "c";		
}

void PyLogger::pd()
{
	strBuffer += "d";		
}
void PyLogger::pe()
{
	strBuffer += "e";		
}

void PyLogger::pf()
{
	strBuffer += "f";		
}
void PyLogger::pg()
{
	strBuffer += "g";		
}

void PyLogger::ph()
{
	strBuffer += "h";		
}
void PyLogger::pi()
{
	strBuffer += "i";		
}

void PyLogger::pj()
{
	strBuffer += "j";		
}
void PyLogger::pk()
{
	strBuffer += "k";		
}

void PyLogger::pl()
{
	strBuffer += "l";		
}

void PyLogger::pm()
{
	strBuffer += "m";		
}
void PyLogger::pn()
{
	strBuffer += "n";		
}

void PyLogger::po()
{
	strBuffer += "o";		
}
void PyLogger::pp()
{
	strBuffer += "p";		
}

void PyLogger::pq()
{
	strBuffer += "q";		
}
void PyLogger::pr()
{
	strBuffer += "r";		
}

void PyLogger::ps()
{
	strBuffer += "s";		
}
void PyLogger::pt()
{
	strBuffer += "t";		
}

void PyLogger::pu()
{
	strBuffer += "u";		
}
void PyLogger::pv()
{
	strBuffer += "v";		
}

void PyLogger::pw()
{
	strBuffer += "w";		
}
void PyLogger::px()
{
	strBuffer += "x";		
}

void PyLogger::py()
{
	strBuffer += "y";		
}
void PyLogger::pz()
{
	strBuffer += "z";		
}
void PyLogger::pspace()
{
	strBuffer +=" ";
}

void PyLogger::p1()
{
	strBuffer +="1";
}
void PyLogger::p2()
{
	strBuffer +="2";
}
void PyLogger::p3()
{
	strBuffer +="3";
}
void PyLogger::p4()
{
	strBuffer +="4";
}
void PyLogger::p5()
{
	strBuffer +="5";
}
void PyLogger::p6()
{
	strBuffer +="6";
}
void PyLogger::p7()
{
	strBuffer +="7";
}
void PyLogger::p8()
{
	strBuffer +="8";
}
void PyLogger::p9()
{
	strBuffer +="9";
}
void PyLogger::p0()
{
	strBuffer +="0";
}




void PyLogger::pnon()
{
	strBuffer +=".";
}

void PyLogger::outlog()
{
	if (strBuffer.size() > 1)
	{
		LOG("pyLog : %s",strBuffer.c_str());
		strBuffer = "";
	}	
}
#endif

}
