#include "SBPython.h"
#include "SBPythonClass.h"
#include "controllers/me_ct_scheduler2.h"
#include "SBFaceDefinition.h"
#include "sbm/nvbg.h"
#include "SBBehavior.h"
#include <sb/SBMotion.h>
#include <sb/SBParseNode.h>
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sb/SBScript.h>
#include <sb/SBService.h>
#include <sb/SBServiceManager.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAnimationState.h>
#include <sb/SBMotionBlendBase.h>
#include <sb/SBAnimationTransition.h>
#include <sb/SBAnimationStateManager.h>
#include <sb/SBSteerManager.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBReach.h>
#include <sb/SBReachManager.h>
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBParser.h>
#include <sb/SBBoneBusManager.h>
#include <sb/SBCollisionManager.h>
#include <sb/SBSteerAgent.h>
#include <sb/SBPhoneme.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBBehaviorSet.h>
#include <sb/SBBehaviorSetManager.h>
#include <sr/sr_box.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include "SBPythonInternal.h"


#ifdef USE_PYTHON
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#endif

typedef std::map<std::string,SrQuat> QuatMap;
typedef std::map<std::string,SrVec> VecMap;
typedef std::map<std::string, std::string> StringMap;


#ifdef USE_PYTHON


namespace SmartBody 
{

void pythonFuncs4()
{
	boost::python::class_<SBDiphoneManager>("SBDiphoneManager")
		.def("createDiphone", &SBDiphoneManager::createDiphone, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a diphone.")
		.def("getDiphones", &SBDiphoneManager::getDiphones, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Get diphones given diphone set name.")
		.def("getDiphone", &SBDiphoneManager::getDiphone, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Get diphone given from phoneme, to phoneme and diphone set name.")
		.def("getNumDiphoneMap", &SBDiphoneManager::getNumDiphoneMap, "Return number of diphone set.")
		.def("getNumDiphones", &SBDiphoneManager::getNumDiphones, "Return number of diphones given the diphone set name.")
		;

	boost::python::class_<SBDiphone>("SBDiphone")
		.def("addKey", &SBDiphone::addKey, "add key to the diphone.")
		.def("getKeys", &SBDiphone::getKeys, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return the keys given viseme name.")
		.def("getVisemeNames", &SBDiphone::getVisemeNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the viseme names.")
		.def("getNumVisemes", &SBDiphone::getNumVisemes, "Return the number of viseme.")
		.def("getFromPhonemeName", &SBDiphone::getFromPhonemeName, boost::python::return_value_policy<boost::python::return_by_value>(), "Return FROM phoneme name.")
		.def("getToPhonemeName", &SBDiphone::getToPhonemeName, boost::python::return_value_policy<boost::python::return_by_value>(), "Return TO phoneme name.")		
		;

		boost::python::class_<SBBehaviorSetManager>("SBBehaviorSetManager")
		.def("createBehaviorSet", &SBBehaviorSetManager::createBehaviorSet, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a behavior set.")
		.def("getBehaviorSets", &SBBehaviorSetManager::getBehaviorSets, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets all the behavior sets.")
		.def("getBehaviorSet", &SBBehaviorSetManager::getBehaviorSet, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets a behavior set with a given name.")
		.def("getNumBehaviorSets", &SBBehaviorSetManager::getNumBehaviorSets, "Returns the number of behavior sets.")
		.def("removeBehaviorSet", &SBBehaviorSetManager::removeBehaviorSet, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Removes a behavior set with a given name.")
		.def("removeAllBehaviorSets", &SBBehaviorSetManager::removeAllBehaviorSets, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Removes all the behavior sets.")
		;

	boost::python::class_<SBBehaviorSet>("SBBehaviorSet")
		.def("setName", &SBBehaviorSet::setName, "Sets the name of the behavior set.")
		.def("getName", &SBBehaviorSet::getName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the name of the behavior set.")
		.def("setScript", &SBBehaviorSet::setScript, "Sets the name of the script to be run for this behavior set.")
		.def("getScript", &SBBehaviorSet::getScript, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the name of the script to be run for this behavior set.")
		;


/*
	boost::python::class_<Viseme>("Viseme")		
		.def("getName", &Viseme::getVisemeName, boost::python::return_value_policy<boost::python::return_by_value>(), "Get the viseme name. \n Input: NULL \n Output: viseme name")
		.def("getCharacterName", &Viseme::getCharName, boost::python::return_value_policy<boost::python::return_by_value>(), "Get the character name. \n Input: NULL \n Output: character name")
		.def("setWeight", &Viseme::setWeight, "Set the weight for current viseme in non-curve mode. \n Input: a list including weight, duration, rampup time, rampdown time e.g. [1,3,1,1] \n Output: NULL")
		.def("setCurve", &Viseme::setCurve, "Set the curve for current viseme in curve mode. \n Input: number of keys, a list of keys e.g. (3, [1,0,3,1,5,0]) \n Output: NULL")
		;
*/

	boost::python::class_<SBFaceDefinition>("SBFaceDefinition")
		.def("getName", &SBFaceDefinition::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns name of the face definition.")
		.def("getNumVisemes", &SBFaceDefinition::getNumVisemes, "Returns the number of visemes.")
		.def("getVisemeNames", &SBFaceDefinition::getVisemeNames, "Returns the names of the visemes.")
		.def("setViseme", &SBFaceDefinition::setViseme, "Sets a viseme to a particular motion name.")
		.def("getVisemeMotion", &SBFaceDefinition::getVisemeMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a motion associated with a particular viseme.")
		.def("getNumAUs", &SBFaceDefinition::getNumAUs, "Returns the number of Action Units.")
		.def("getAUNumbers", &SBFaceDefinition::getAUNumbers, "Returns the numbers of the Action Units.")
		.def("getAUSide", &SBFaceDefinition::getAUSide, "Returns LEFT, RIGHT, or BOTH, depending on which side is used for the Action Unit.")
		.def("getAUMotion", &SBFaceDefinition::getAUMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion associated with a given Action Unit side: LEFT, RIGHT, or BOTH.")
		.def("setAU", &SBFaceDefinition::setAU, "Sets an Action Unit of a given number to a side and a motion.")
		.def("setFaceNeutral", &SBFaceDefinition::setFaceNeutral, "Sets the neutral face to a particular motion name.")
		.def("save", &SBFaceDefinition::save, "Save face definition to a file.")
		;


	


	boost::python::class_<SBMotion, boost::python::bases<SBObject> >("SBMotion")
		//.def(boost::python::init<std::string>())
		.def("getMotionFileName", &SBMotion::getMotionFileName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion file name. \n Input: NULL \n Output: motion file name")
		.def("getNumFrames", &SBMotion::getNumFrames, "Returns the number of frames inside this motion. \n Input: NULL \n Output: number of frames in the motion")
		.def("getFrameData", &SBMotion::getFrameData, "Returns the frame data given frame index. \n Input: frame index \n Output: a list of frame data")
		.def("getFrameSize", &SBMotion::getFrameSize, "Returns the frame size. \n Input: NULL \n Output: frame size (how many data does one frame include)")
		.def("getNumChannels", &SBMotion::getNumChannels, "Returns the number of channels for this motion. \n Input: NULL \n Output: number of channels for this motion")
		.def("getChannels", &SBMotion::getChannels, "Returns the channels + type inside the skeleton. \n Input: NULL \n Output: channel name and type")
		.def("checkSkeleton", &SBMotion::checkSkeleton, "Print out all the motion channels and compare it with the given skeleton channels. Mark '+' in front if the skeleton channel exists in the motion. \n Input: skeleton file name \n Output: NULL")
		.def("connect", &SBMotion::connect, "Connect current motion to a skeleton object so the channels inside the motion are mapped to the channels inside skeleton. \n Input: Skeleton Object \n Output: NULL")
		.def("disconnect", &SBMotion::disconnect, "Disconnect current motion with current skeleton object. ")
		.def("mirror", &SBMotion::mirror, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Mirrors the motion.")
		.def("retarget", &SBMotion::retarget, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Retarget the motion to a different skeleton.")
		.def("footSkateCleanUp", &SBMotion::footSkateCleanUp, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Retarget the motion to a different skeleton.")
		.def("constrain", &SBMotion::constrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Constrain the retargeted motion to based on the source skeleton and motion.")
		.def("smoothCycle", &SBMotion::smoothCycle, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Build the smooth cycle the motion.")
		.def("alignToEnd", &SBMotion::alignToEnd, "Cut the first x number of frames and stitch them to the end. x is the input number")
		.def("alignToBegin", &SBMotion::alignToBegin, "Cut the last x number of frames and stitch them to the begin. x is the input number")
		.def("duplicateCycle", &SBMotion::duplicateCycle, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Duplicate motion by x amount of cycles.")
		.def("getJointSpeed", &SBMotion::getJointSpeed, "Get the accumulative joint speed. \n Input: SBJoint, start time, end time \n Output: joint speed(unit: same with the skeleton)")
		.def("getJointSpeedAxis", &SBMotion::getJointSpeedAxis, "Get the accumulative joint speed of a given axis: X, Y or Z. \n Input: SBJoint, axis, start time, end time \n Output: joint speed(unit: same with the skeleton)")
		.def("getJointAngularSpeed", &SBMotion::getJointAngularSpeed, "Get the joint accumulative angular speed. \n Input: SBJoint, start time, end time \n Output: joint angular speed(unit: degree/sec)")		
		.def("getJointAngularSpeedAxis", &SBMotion::getJointAngularSpeedAxis, "Get the joint accumulative angular speed of a given axis: X, Y or Z. \n Input: SBJoint, axis, start time, end time \n Output: joint angular speed(unit: degree/sec)")		
		.def("getJointTransition", &SBMotion::getJointTransition, "Get the joint transition vector. \n Input: SBJoint, start time, end time \n Output: joint transition vector containing x, y, z value (unit: same with the skeleton)")		
		.def("getJointPosition", &SBMotion::getJointPosition, "Get the joint position. \n Input: SBJoint, time \n Output: joint position containing x, y, z value (unit: same with the skeleton)")		
		.def("translate", &SBMotion::translate, "Translates the base joint name by x,y,z values.")		
		.def("rotate", &SBMotion::rotate, "Rotates the base joint name by x,y,z axis.")			
		.def("scale", &SBMotion::scale, "Scales all translations in skeleton by scale factor.")		
		.def("trim", &SBMotion::trim, "Trims the starting and ending frames in the motion.")	
		.def("saveToSkm", &SBMotion::saveToSkm, "Saves the file in .skm format to a given file name.")	
		.def("getTimeStart", &SBMotion::getTimeStart, "Returns the start time of the motion.")
		.def("getTimeReady", &SBMotion::getTimeReady, "Returns the ready time of the motion.")
		.def("getTimeStrokeStart", &SBMotion::getTimeStrokeStart, "Returns the stroke start time of the motion.")
		.def("getTimeStroke", &SBMotion::getTimeStroke, "Returns the stroke time of the motion.")
		.def("getTimeStrokeEnd", &SBMotion::getTimeStrokeEnd, "Returns the stroke end time of the motion.")
		.def("getTimeRelax", &SBMotion::getTimeRelax, "Returns the relax time of the motion.")
		.def("getTimeStop", &SBMotion::getTimeStop, "Returns the stop time of the motion.")	
		.def("getDuration", &SBMotion::getDuration, "Return the duration of the motion")
		.def("addEvent", &SBMotion::addEvent, "Adds an event associated with this motion that will be triggered at the given time. The last paramter determines if the event will be triggered only once, or every time the motion is looped.")
		.def("addMetaData", &SBMotion::addMetaData, "Add a tagged metadata as string to the motion.")
		.def("removeMetaData", &SBMotion::removeMetaData, "Remove a tagged metadata from the motion.")
		.def("getMetaDataString", &SBMotion::getMetaDataString, "Get the first metadata based on tag name")
		.def("getMetaDataDouble", &SBMotion::getMetaDataDouble, "Get the first metadata based on tag name")
		.def("getMetaDataTags", &SBMotion::getMetaDataTags, "Get all tag names in the metadata map.")
		;


	boost::python::class_<SBController, boost::python::bases<SBObject> >("SBController")
		.def(boost::python::init<>())
		.def("setName", &SBController::setName, "Sets the name for the controller.")
		.def("getName", &SBController::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name for this controller.")
		.def("getType", &SBController::getType, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the type for this controller.")
		.def("setIgnore", &SBController::setIgnore, "Ignore this controller when evaluating controller tree.")
		.def("isIgnore", &SBController::isIgnore, "Will the controller be ignored during evaluation.")
		.def("setDebug", &SBController::setDebug, "Sets the debug state for this controller.")
		.def("isDebug", &SBController::isDebug, "Is the controller in a debug state?")
		.def("getDuration", &SBController::getDuration, "Gets the controller's duration.")
		;

	boost::python::class_<MeCtScheduler2, boost::python::bases<SBController> > ("SchedulerController")
		.def("getNumTracks", &MeCtScheduler2::count_children, "Returns the number of children/tracks.")
		;

	boost::python::class_<MeCtMotion, boost::python::bases<SBController> > ("MotionController")
		//	.def("getMotion", &MeCtMotion::motion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the motion associated with this motion controller.")
		;

	boost::python::class_<MeCtEyeLid, boost::python::bases<SBController> > ("EyelidController")
		;

	boost::python::class_<MeCtLocomotion, boost::python::bases<SBController> > ("LocomotionController")
		;

	boost::python::class_<MeCtGaze, boost::python::bases<SBController> > ("GazeController")
		;

	//boost::python::class_<MeCtReach, boost::python::bases<SBController> > ("ReachController")	;

	boost::python::class_<MeCtCurveWriter, boost::python::bases<SBController> > ("CurveWriterController")
		;

	boost::python::class_<SBSkeleton>("SBSkeleton")
	//	.def(boost::python::init<>())
		.def(boost::python::init<std::string>())
		.def("load", &SBSkeleton::load, "Loads the skeleton definition from the given skeleton name.")
		.def("save", &SBSkeleton::save, "Saves the skeleton definition with the given skeleton name.")
		.def("getName", &SBSkeleton::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the skeleton.")
		.def("getNumJoints", &SBSkeleton::getNumJoints, "Returns the number of joints for this skeleton.")
		.def("getJointNames", &SBSkeleton::getJointNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the joint names for this skeleton.")
		.def("getJointByName", &SBSkeleton::getJointByName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint of a given name.")
		.def("getJoint", &SBSkeleton::getJoint, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint with a given index.")
		.def("getNumChannels", &SBSkeleton::getNumChannels, "Returns the number of the channels inside the skeleton.")
		.def("getChannelType", &SBSkeleton::getChannelType, "Returns the type of the channel of a given index.")
		.def("getChannelSize", &SBSkeleton::getChannelSize, "Returns the size of the channel given index.")
		.def("createSkelWithoutPreRot", &SBSkeleton::createSkelWithoutPreRot, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a new standard T-pose skel from source but without pre-rotations")	
		.def("orientJointsLocalAxesToWorld", &SBSkeleton::orientJointsLocalAxesToWorld, "Orient skeleton joints local axes to match world coordinate axes (Y-up Z-front)")	
		;

	boost::python::class_<SBJoint, boost::python::bases<SBObject> >("SBJoint")
		.def(boost::python::init<>())
		.def("setName", &SBJoint::setName, "Set the name of the joint.")
		.def("getName", &SBJoint::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the joint.")
		.def("getParent", &SBJoint::getParent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the parent joint.")
		.def("setParent", &SBJoint::setParent, "Sets the parent joint.")
		.def("getNumChildren", &SBJoint::getNumChildren, "Returns the number of child joints.")
		.def("getChild", &SBJoint::getChild, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the child joint with a given index.")
		.def("getSkeleton", &SBJoint::getSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the skeleton.")
		.def("setSkeleton", &SBJoint::setSkeleton, "Sets the skeleton.")
		.def("getOffset", &SBJoint::getOffset, "Returns the offset of the joint from the parent joint.") 
		.def("setOffset", &SBJoint::setOffset, "Sets the offset of the joint from the parent joint.")
		.def("getIndex", &SBJoint::getIndex, "Returns the index of the joint in current skeleton.")
		.def("getPosition", &SBJoint::getPosition, "Returns the current position of the joint in global coordinates.")
		.def("getQuat", &SBJoint::getQuaternion, "Returns the current quaterion of the joint in global coordinates.")
		.def("getMatrixGlobal", &SBJoint::getMatrixGlobal, "Returns the matrix of the joint in global coordinates.")
		.def("getMatrixLocal", &SBJoint::getMatrixLocal, "Returns the matrix of the joint in local coordinates.")
		.def("addChild", &SBJoint::addChild, "Add a child joint to current joint.")
		.def("setUseRotation", &SBJoint::setUseRotation, "Allows the joint to use rotation channels.")	
		.def("getUseRotation", &SBJoint::isUseRotation, "Determines if the joint uses rotation channels.")	
		.def("setUsePosition", &SBJoint::setUsePosition, "Allows the joint to use position channels.")	
		.def("isUsePosition", &SBJoint::isUsePosition, "Determines if the joint uses position channels.")	
		.def("getMass", &SBJoint::getMass, "Gets the mass of the joint.")
		.def("setMass", &SBJoint::setMass, "Sets the mass of the joint.")
		.def("getPrerotation", &SBJoint::getPrerotation, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the prerotation values for the joint.")
		.def("setPrerotation", &SBJoint::setPrerotation, "Sets the prerotation values for the joint.")
		.def("getPostrotation", &SBJoint::getPostrotation, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the postrotation values for the joint.")
		.def("setPostrotation", &SBJoint::setPostrotation, "Sets the postrotation values for the joint.")		;

	boost::python::class_<SBBehavior, boost::python::bases<SBObject> >("SBBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getType", &SBBehavior::getType, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the type of the behavior.")
		;

	boost::python::class_<GazeBehavior, boost::python::bases<SBBehavior> >("GazeBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getGazeTarget", &GazeBehavior::getGazeTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the gaze target.")
		.def("isFadingIn", &GazeBehavior::isFadingIn, "Returns true if the gaze controller is currently fading in.")
		.def("isFadingOut", &GazeBehavior::isFadingOut, "Returns true if the gaze controller is currently fading out.")
		.def("isFadedOut", &GazeBehavior::isFadedOut, "Returns true if the gaze controller is currently faded out and thus not active.")
		.def("getHandle", &GazeBehavior::getHandle, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the handle name of the gaze, or an empty string if it has no handle.")
;

	boost::python::class_<LocomotionBehavior, boost::python::bases<SBBehavior> >("LocomotionBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getLocomotionTarget", &LocomotionBehavior::getLocomotionTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the locomotion target as a vector.")
	;

	boost::python::class_<PostureBehavior, boost::python::bases<SBBehavior> >("PostureBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getPosture", &PostureBehavior::getPosture, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the current posture.")
	;

	boost::python::class_<SpeechBehavior, boost::python::bases<SBBehavior> >("SpeechBehavior")
		//.def(boost::python::init<std::string, std::string>())
		.def("getUtterance", &SpeechBehavior::getUtterance, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the current utterance.")
	;

	boost::python::class_<SBPawn, boost::python::bases<SBObject> >("SBPawn")
		.def("getName", &SBPawn::getName, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the name of the pawn..")
		.def("getSkeleton", &SBPawn::getSkeleton, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the skeleton object of the pawn.")
		.def("setSkeleton", &SBPawn::setSkeleton, "Attaches the skeleton to the character.")
		.def("setName", &SBPawn::setName, "Sets or changes the name of the character.")
		.def("getPosition", &SBPawn::getPosition, "Returns the current position of the character's world offset.")
		.def("getOrientation", &SBPawn::getOrientation, "Returns the current orientation of the character's world offset.")
		.def("setPosition", &SBPawn::setPosition, "Sets the current position of the character's world offset.")
		.def("setOrientation", &SBPawn::setOrientation, "Set the current orientation of the character's world offset.")
		.def("setHPR", &SBPawn::setHPR, "Sets the heading, pitch and roll of the character's world offset.")
		.def("getHPR", &SBPawn::getHPR, "Gets the heading, pitch and roll of the character's world offset.")
	;

	boost::python::class_<SBCharacter, boost::python::bases<SBPawn, SBObject> >("SBCharacter")
		//.def(boost::python::init<std::string, std::string>())
		.def("setMeshMap", &SBCharacter::setMeshMap, "Set the OpenCollada file for the character which contains all the smoothbinding information.")
		.def("addMesh", &SBCharacter::addMesh, "Add obj mesh to current character for smoothbinding.")
		.def("isAutomaticPruning", &SBCharacter::isAutomaticPruning, "Returns true if the character's cotnroller are automatically pruned.")
		.def("setAutomaticPruning", &SBCharacter::setAutomaticPruning, "Toggles the automatic pruning mechanism on or off.")
		.def("pruneControllers", &SBCharacter::pruneControllers, "Prunes the controller tree.")
		.def("setSoftEyes", &SBCharacter::setSoftEyes, "Sets the soft eyes feature.")
		.def("isSoftEyes", &SBCharacter::isSoftEyes, "Returns the value of the soft eyes feature.")
		.def("setUseVisemeCurves", &SBCharacter::setUseVisemeCurves, "Use curves when interpreting visemes.")
		.def("isUseVisemeCurves", &SBCharacter::isUseVisemeCurves, "Are curves used when interpreting visemes.")
		.def("setVisemeTimeOffset", &SBCharacter::setVisemeTimeOffset, "Set the time delay for viseme curve mode.")
		.def("getVisemeTimeOffset", &SBCharacter::getVisemeTimeOffset, "Get the time delay for viseme curve mode.")
		.def("getNumControllers", &SBCharacter::getNumControllers, "Returns number of top level controllers inside this character.")
		.def("createStandardControllers", &SBCharacter::createStandardControllers, "Returns number of top level controllers inside this character.")		
		//.def("getNumVisemes", &SBCharacter::getNumVisemes, "Returns the number of visemes.")
		.def("getControllerByIndex", &SBCharacter::getControllerByIndex, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the ith controller.")
		.def("getControllerByName", &SBCharacter::getControllerByName, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the controller with the given name.")
		.def("getControllerNames", &SBCharacter::getControllerNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the controller name vector.")
		.def("setVoice", &SBCharacter::setVoice, "Sets the voice type: remote, audiofile, text or none (use \"\").")
		.def("setVoiceCode", &SBCharacter::setVoiceCode, "Sets the voice code. For audiofile type, this is a path.")
		.def("setVoiceBackup", &SBCharacter::setVoiceBackup, "Sets the voice backup type: remote, audiofile, text or none (use \"\").")
		.def("setVoiceBackupCode", &SBCharacter::setVoiceBackupCode, "Sets the voice backup code. For audiofile type, this is a path.")
		.def("getVoiceCode", &SBCharacter::getVoiceBackupCode, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the voice code. For audiofile type, this is a path.")
		.def("getVoiceBackup", &SBCharacter::getVoiceBackup, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the voice backup type: remote, audiofile, text or none (use \"\").")
		.def("setFaceDefinition", &SBCharacter::setFaceDefinition, "Sets face definition (visemes, action units) for a character.")
		.def("getFaceDefinition", &SBCharacter::getFaceDefinition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets face definition (visemes, action units) for a character.")
		.def("getHeight", &SBCharacter::getHeight, "Gets the height of the character.")
		.def("getBoundingBox", &SBCharacter::getBoundingBox, "Gets the boundary dimensions of the character.")
		.def("getNumBehaviors", &SBCharacter::getNumBehaviors, "Returns the number of behaviors of the character.")
		.def("getBehavior", &SBCharacter::getBehavior, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the ith behavior of the character.")
		.def("setSteerAgent", &SBCharacter::setSteerAgent, "Set the steer agent of the character")
		//.def("getFaceDefinition", &SBCharacter::getFaceDefinition, "Gets face definition (visemes, action units) for a character.")
#ifndef __ANDROID__
		.def("setNvbg", &SBCharacter::setNvbg, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Sets the NVBG handler for this character.")
		.def("getNvbg", &SBCharacter::getNvbg, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the NVBG handler for this character.")
		.def("setMiniBrain", &SBCharacter::setMiniBrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Sets the mini brain handler for this character.")
		.def("getMiniBrain", &SBCharacter::getMiniBrain, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the mini brain handler for this character.")
#endif
		;

boost::python::class_<SBReach>("SBReach")
		.def("getCharacter", &SBReach::getCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the character associated with this reach engine.")
		.def("copy", &SBReach::copy, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Copies the reach engine.")
		.def("addMotion", &SBReach::addMotion, "Adds a motion to the reach engine.")
		.def("removeMotion", &SBReach::removeMotion, "Removes a motion from the reach engine.")
		.def("getNumMotions", &SBReach::getNumMotions, "Returns the number of motions in the reach engine.")
		.def("getMotionNames", &SBReach::getMotionNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the names of the motions used in the reach engine.")
		.def("build", &SBReach::build, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Constructs the reach engine using the exisiting motions.")
		.def("setGrabHandMotion", &SBReach::setGrabHandMotion, "Sets the hand pose to be used during grasping.")
		.def("getGrabHandMotion", &SBReach::getGrabHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used during grasping.")
		.def("setReleaseHandMotion", &SBReach::setReleaseHandMotion,  "Sets the hand pose to be used when releasing an object.")
		.def("getReleaseHandMotion", &SBReach::getReleaseHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used when releasing an object.")
		.def("setReachHandMotion", &SBReach::setReachHandMotion, "Sets the hand pose to be used when reaching for an object.")
		.def("getReachHandMotion", &SBReach::getReachHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used when reaching for an object.")
		.def("setPointHandMotion", &SBReach::setPointHandMotion, "Sets the hand pose to be used when reaching for an object.")
		.def("getPointHandMotion", &SBReach::getPointHandMotion, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the hand pose to be used when reaching for an object.")
		.def("setInterpolatorType", &SBReach::setInterpolatorType, "Set the interpolation type when building the reach engine")
		.def("isPawnAttached", &SBReach::isPawnAttached, boost::python::return_value_policy<boost::python::return_by_value>(), "Return True is the pawn is currently attached to the character's hand.")
		;

		;

	boost::python::class_<SBReachManager>("SBReachManager")
		.def("createReach", &SBReachManager::createReach, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a reach engine for a character.")
		.def("removeReach", &SBReachManager::removeReach, "Removes a reach engine for a character")
		.def("getNumReaches", &SBReachManager::getNumReaches, "Returns the number of reach engines present.")
		.def("getReach", &SBReachManager::getReach, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a reach engine for a given character.")
		;

	boost::python::class_<SBGestureMap>("SBGestureMap")
		.def("addGestureMapping", &SBGestureMap::addGestureMapping, "Add a gesture mapping. Input: name of the animation/state, type, posture, hand. Output: null")
		.def("getGestureByInfo", &SBGestureMap::getGestureByInfo, "Return a gesture given the type and hand of the gesture. Input: type, hand, style. Output: corresponding gesture name")
		.def("getGestureByIndex", &SBGestureMap::getGestureByIndex, "Return a gesture given the index inside the map.")
		.def("getNumMappings", &SBGestureMap::getNumMappings, "Return a number of entries inside the map.")
		.def("getGesturePosture", &SBGestureMap::getGesturePosture, "Return the gesture posture given the name.")
		.def("getGestureHand", &SBGestureMap::getGestureHand, "Return the gesture hand given the name.")
		.def("getGestureType", &SBGestureMap::getGestureType, "Return the gesture type given the name.")
		.def("getGestureStyle", &SBGestureMap::getGestureStyle, "Return the gesture style given the name.")
		;

	boost::python::class_<SBGestureMapManager>("SBGestureMapManager")
		.def("createGestureMap", &SBGestureMapManager::createGestureMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a gesture map for a character.")
		.def("removeGestureMap", &SBGestureMapManager::removeGestureMap, "Remove a gesture map for a character given character name.")
		.def("getNumGestureMaps", &SBGestureMapManager::getNumGestureMaps, "Return number of gesture maps in the scene.")
		.def("getGestureMap", &SBGestureMapManager::getGestureMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return gesture map given character name.")
		;

	boost::python::class_<SBJointMap>("SBJointMap")
		.def("setMapping", &SBJointMap::setMapping, "Sets the mapping from one joint name to another.")
		.def("removeMapping", &SBJointMap::removeMapping, "Removes a mapping from a given joint to whichever joint is mapped.")
		.def("getMapTarget", &SBJointMap::getMapTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the target joint for a given mapping.")
		.def("getMapSource", &SBJointMap::getMapSource, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the source joint for a given mapping.")
		.def("getNumMappings", &SBJointMap::getNumMappings, "Returns the number of joint mappings.")
		.def("getSource", &SBJointMap::getSource, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the source joint of the nth mapping.")
		.def("getTarget", &SBJointMap::getTarget, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the target joint of the nth mapping.")
		.def("applyMotion", &SBJointMap::applyMotion, "Applies the current joint mapping to a motion.")
		.def("applySkeleton", &SBJointMap::applySkeleton, "Applies the current skeleton mapping to a motion.")
		.def("applyMotionInverse", &SBJointMap::applyMotionInverse, "Applies the inverse joint mapping to a motion.")
		.def("applySkeletonInverse", &SBJointMap::applySkeletonInverse, "Applies the inverse skeleton mapping to a motion.")
		.def("guessMapping", &SBJointMap::guessMapping, "Automatic joint name matching to standard SmartBody names.")
		;

	boost::python::class_<SBJointMapManager>("SBJointMapManager")
		.def("getJointMap", &SBJointMapManager::getJointMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the joint map associated with a given name.")
		.def("createJointMap", &SBJointMapManager::createJointMap, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a joint map with a given name. Returns null is the map already exists.")
		.def("getJointMapNames", &SBJointMapManager::getJointMapNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the names of all joint maps.")
		.def("removeJointMap", &SBJointMapManager::removeJointMap, "Removes a joint map of a given name.")
		.def("removeAllJointMaps", &SBJointMapManager::removeAllJointMaps, "Removes all the joint maps.")
		;

	boost::python::class_<Event>("Event")
		.def(boost::python::init<>())
		.def("getType", &Event::getType, "Returns the event type.")
		.def("setType", &Event::setType, "Sets the event type.")
		.def("getParameters", &Event::getParameters, "Returns the event parameters.")
		.def("setParameters", &Event::setParameters, "Sets the event parameters.")
		;

	boost::python::class_<EventManager>("EventManager")
		.def("handleEvent", &EventManager::handleEvent, "Processes an event by the appropriate event handler.")
		.def("addEventHandler", &EventManager::addEventHandler, "Returns the event type.")
		.def("removeEventHandler", &EventManager::removeEventHandler, "Returns the event type.")
		.def("getNumHandlers", &EventManager::getNumEventHandlers, "Gets the number of event handlers.")
		.def("getEventHandler", &EventManager::getEventHandler, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Gets the number of event handlers.")
		.def("createEvent", &EventManager::createEvent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates an event of a given type with given parameters.")
		;

	boost::python::class_<SBParseNode>("SBParseNode")
		.def("getWord", &SBParseNode::getWord, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the word, empty word if not a terminal node.")
		.def("getTerm", &SBParseNode::getTerm, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the terminal, empty word if not a terminal node.")
		.def("isTerminal", &SBParseNode::isTerminal, "Is this node a terminal node.")
		.def("getNumChildren", &SBParseNode::getNumChildren, "Deletes parse tree.")
		.def("getChild", &SBParseNode::getChild, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a child node.")
		;

	boost::python::class_<SBParser>("SBParser")
		.def("parse", &SBParser::parse, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Parses an utterance using the Charniak parser.")
		.def("initialize", &SBParser::initialize, "Initializes the Charniak parser with parameters.")
		.def("cleanUp", &SBParser::cleanUp, "Deletes parse tree.")
		.def("isInitialized", &SBParser::isInitialized, "Return boolean telling if Charniak parser is initialized.")
		;

	boost::python::class_<SrVec>("SrVec")
		.def(boost::python::init<>())
		.def(boost::python::init<float, float, float>())
		.def("getData", &SrVec::getData, "gets the x,y,z values")
		.def("setData", &SrVec::setData, "sets the x,y,z values")
		.def("len", &SrVec::norm, "gets the length of the vector")
		.def("normalize", &SrVec::normalize, "normalizes the vector")
		.def("isZero", &SrVec::iszero, "returns True if the vector is zero")
		.def("rotY", &SrVec::rotY, "rotate vector around Y axis (radian)")
		.def("vecAngle", &SrVec::vecAngle, "Returns the angle between v1 and v2 (radian)")
		.def("vecYaw", &SrVec::vecYaw, "Returns Yaw angle on X-Z plane of given vec (radian)")
		;

	boost::python::class_<SrMat>("SrMat")
		.def(boost::python::init<>())
		.def("getData", &SrMat::getData, "gets the data in the matrix at r,c")
		.def("setData", &SrMat::setData, "sets the data in the matrix at r,c")
		.def("identity", &SrMat::identity, "sets the data in the matrix to an identity matrix")
		.def("transpose", &SrMat::transpose, "transposes the data in the matrix")
		;

	boost::python::class_<SrQuat>("SrQuat")
		.def(boost::python::init<>())
		.def(boost::python::init<float, float, float, float>())
		.def("getData", &SrQuat::getData, "gets the data in the quaterion at location indicated by the index w,x,y,z")
		.def("setData", &SrQuat::setData, "sets the data in the quaterion at location indicated by the index w,x,y,z")
		;	
	
	boost::python::class_<SrBox>("SrBox")
		.def(boost::python::init<>())
		.def(boost::python::init<SrVec, SrVec>())
		.def(boost::python::init<SrBox>())
		.def("setMinimum", &SrBox::setMinimum, "sets the minimum values of the box")
		.def("setMaximum", &SrBox::setMaximum,  "sets the maximum values of the box")
		.def("getMinimum", &SrBox::getMinimum, boost::python::return_value_policy<boost::python::return_by_value>(), "gets the minimum values of the box")
		.def("getMaximum", &SrBox::getMaximum, boost::python::return_value_policy<boost::python::return_by_value>(), "gets the maximum values of the box")
		.def("getCenter", &SrBox::getCenter, boost::python::return_value_policy<boost::python::return_by_value>(), "gets center of the box")
		.def("getMinSize", &SrBox::min_size, "gets the minimum dimension of the box")
		.def("getMaxSize", &SrBox::max_size, "gets the maximum dimension of the box")
		.def("getSize", &SrBox::getSize, boost::python::return_value_policy<boost::python::return_by_value>(), "returns the size of each dimension")
		.def("doesContain", &SrBox::contains, "returns the center of the box")
		.def("doesIntersect", &SrBox::intersects, "returns the center of the box")
		.def("getVolume", &SrBox::volume, "returns the volume of the box")
		.def("isEmpty", &SrBox::empty, "returns true if the box is empty")
		;

#ifndef __ANDROID__
	boost::python::class_<NvbgWrap, boost::python::bases<SBObject>, boost::noncopyable>("Nvbg")
		.def("objectEvent", &Nvbg::objectEvent, &NvbgWrap::default_objectEvent, "An event indicating that an object of interest is present.")
		.def("execute", &Nvbg::execute, &NvbgWrap::default_execute, "Execute the xml vrX message.")
		.def("executeEvent", &Nvbg::executeEvent, &NvbgWrap::default_executeEvent, "Execute the vrAgent message.")
		.def("executeSpeech", &Nvbg::executeSpeech, &NvbgWrap::default_executeSpeech, "Execute the vrSpeech message.")
		.def("notifyAction", &Nvbg::notifyAction, &NvbgWrap::default_notifyAction, "Notifies NVBG processor of a bool attribute.")
		.def("notifyBool", &Nvbg::notifyBool, &NvbgWrap::default_notifyBool, "Notifies NVBG processor of a bool attribute")
		.def("notifyInt", &Nvbg::notifyInt, &NvbgWrap::default_notifyInt, "Notifies NVBG processor of an int attribute")
		.def("notifyDouble", &Nvbg::notifyDouble, &NvbgWrap::default_notifyDouble, "Notifies NVBG processor of a double attribute")
		.def("notifyString", &Nvbg::notifyString, &NvbgWrap::default_notifyString, "Notifies NVBG processor of a string attribute")
		.def("notifyVec3", &Nvbg::notifyVec3, &NvbgWrap::default_notifyVec3, "Notifies NVBG processor of a vec3 attribute.")
		.def("notifyMatrix", &Nvbg::notifyMatrix, &NvbgWrap::default_notifyMatrix, "Notifies NVBG processor of a matrix attribute.")
		;

	boost::python::class_<SBScriptWrap, boost::noncopyable>("SBScript")
		.def("start", &SBScript::start, &SBScriptWrap::default_start, "Script start.")
		.def("beforeUpdate", &SBScript::beforeUpdate, &SBScriptWrap::default_beforeUpdate, "Script before update step.")
		.def("update", &SBScript::update, &SBScriptWrap::default_update, "Script updates.")
		.def("afterUpdate", &SBScript::afterUpdate, &SBScriptWrap::default_afterUpdate, "Script after update step.")
		.def("stop", &SBScript::stop, &SBScriptWrap::default_stop, "Script stop.")
		;

	boost::python::class_<EventHandlerWrap, boost::noncopyable>("EventHandler")
		.def("executeAction", &EventHandler::executeAction, &EventHandlerWrap::default_executeAction, "Execute the event handler.")
		;
#endif

	boost::python::class_<PythonControllerWrap, boost::python::bases<SBController>, boost::noncopyable> ("PythonController")
		.def("start", &PythonController::start, &PythonControllerWrap::default_start, "start.")
		.def("stop", &PythonController::stop, &PythonControllerWrap::default_stop, "stop.")
		.def("init", &PythonController::init, &PythonControllerWrap::default_init, "init.")
		.def("evaluate", &PythonController::evaluate, &PythonControllerWrap::default_evaluate, "evaluate.")
		;


}
}


#endif
