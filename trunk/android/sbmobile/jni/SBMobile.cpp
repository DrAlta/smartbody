#include "VHEngine.h"
#include "SBWrapper.h"
#include <sstream>
#include <sb/SBScene.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBCommandManager.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBAssetManager.h>
#include <sb/SBSpeechManager.h>
#include <sb/SBVHMsgManager.h>
#include <sb/SBParser.h>
#include <sbm/local_speech.h>
#include <sbm/GPU/SbmTexture.h>

#include <alib/npc/npc.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>

#include "NonverbalBehavior.h"

class Model
{
	public:
		Model() {}
		~Model() {}
		//alib::npc::Collection collection;
		//std::map<std::string, alib::npc::detail::Ranker> rankerMap;
		alib::npc::Platform p;
};

JavaVM* VHEngine::jvm = NULL;
JNIEnv* VHEngine::env = NULL;
bool    VHEngine::jvmIsAttached = false;
VHEngine* VHEngine::engine = NULL;
int VHEngine::curH = -1;
int VHEngine::curW = -1;

class MyParserListener : public SmartBody::SBParserListener
{
	public:
		MyParserListener() : SmartBody::SBParserListener() { }
		MyParserListener (VHEngine* engine) : SmartBody::SBParserListener() { _engine = engine; }
		~MyParserListener() {}
		virtual void onWord(std::string timing, std::string word, std::string emphasis) { this->addBML(_engine->eventWord(timing, word, emphasis)); }
		virtual void onPartOfSpeech(std::string timing, std::string word) { this->addBML(_engine->eventPartOfSpeech(timing, word)); }
	
		VHEngine* _engine;
};


VHEngine::VHEngine()
{
	prevTime = 0;
	engineInit = false;
	_dataFolder = "classifier";
	
	
	std::stringstream strstr;
	strstr << "/sdcard/vhdata/" << _dataFolder << "/kstem-lexicon-beta";
	alib::npc::Platform::setLexiconDirectory(strstr.str());
	alib::npc::Platform::initialize();
	//_model = new Model();
	_model = NULL;
	
	_parserListener = new MyParserListener(this);
}


VHEngine::~VHEngine()
{
	delete _model;
	delete _parserListener;
}

bool VHEngine::isWifiOn()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","isWifiOn","()Z", interfaceClass, methodID);

	if (jvmSuccess)
	{
		bool ret = env->CallStaticBooleanMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
		return ret;
	}
	else
	{
		return false;
	}
}

bool VHEngine::isMobileOn()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","isMobileOn","()Z", interfaceClass, methodID);

	if (jvmSuccess)
	{
		bool ret = env->CallStaticBooleanMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
		return ret;
	}
	else
	{
		return false;
	}	
}

bool VHEngine::isLocationOn()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","isLocationOn","()Z", interfaceClass, methodID);

	if (jvmSuccess)
	{
		bool ret = env->CallStaticBooleanMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
		return ret;
	}
	else
	{
		return false;
	}	
}

std::string VHEngine::getVersion()
{
	return "Android revision 446";
}
void VHEngine::setDataFolder(std::string folder)
{
	_dataFolder = folder;
}
std::string VHEngine::getDataFolder()
{
	return _dataFolder;
}

bool VHEngine::eventScreenTouch(int action, float x, float y)
{
	//LOG("touch screen event, action = %d, x = %f, y = %f", action, x, y);
	static bool firstTime = true;
	static float prevx = 0.f, prevy = 0.f;
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	if (action == 0) // action down
		firstTime = true;
	//SrCamera* cam = scene->getCamera("defaultCamera");
	//if (!cam) return false;
	float deltaX, deltaY;
	deltaX = x - prevx; 
	deltaY = y - prevy;
	if (firstTime)
	{
		deltaX = 0.f;
		deltaY = 0.f;
		firstTime = false;		
	}
	prevx = x; 
	prevy = y;
	
	SBCameraOperation(deltaX,deltaY);

	return true;

#if 0

	float dx = deltaX * cam->getAspectRatio() * 0.01;
	float dy = deltaY * cam->getAspectRatio() * 0.01;

	enum { Touch_Pressed = 0, Touch_Released, Touch_Moved, Touch_Cancelled, Touch_None };
	int touchAction = Touch_None;
	switch(action)
	{
	case 0:
		//state.touchType = OIS::MT_Pressed;
		touchAction = Touch_Pressed;
		break;
	case 1:
		//state.touchType = OIS::MT_Released;
		touchAction = Touch_Released;
		break;
	case 2:
		//state.touchType = OIS::MT_Moved;
		touchAction = Touch_Moved;
		break;
	case 3:
		//state.touchType = OIS::MT_Cancelled;
		touchAction = Touch_Cancelled;
		break;
	default:
		//state.touchType = OIS::MT_None;
		touchAction = Touch_None;
		break;
	}

#if 0
	if (touchAction == Touch_Moved)	// zoom
	{
		LOG("touchAction = Touch_Moved");
		float tmpFov = cam->getFov() + (-dx + dy);
		cam->setFov(SR_BOUND(tmpFov, 0.001f, srpi));
	}
#endif
	static SrVec origUp, origCenter, origCamera;
	static float origX, origY;
	if (touchAction == Touch_Pressed)
	{
		//LOG("touchAction = Pressed");
		origUp = cam->getUpVector();
		origCenter = cam->getCenter();
		origCamera = cam->getEye();		
		origX = x;
		origY = y;
	}
	else if (touchAction == Touch_Released)
	{
		origX = -1;
		origY = -1;		
	}
	else if (touchAction == Touch_Moved) // rotate
	{
		//LOG("touchAction = Moved");
		float camDx = (x-origX) * cam->getAspectRatio() * 0.01;
		float camDy = (y-origY) * cam->getAspectRatio() * 0.01;
		SrVec forward = origCenter - origCamera; 		   
		SrQuat q = SrQuat(origUp, vhcl::DEG_TO_RAD()*camDx*20.f);			   
		forward = forward*q;
		SrVec tmp = cam->getEye() + forward;
		cam->setCenter(tmp.x, tmp.y, tmp.z);

		SrVec cameraRight = cross(forward,origUp);
		cameraRight.normalize();		   
		q = SrQuat(cameraRight, vhcl::DEG_TO_RAD()*camDy*20.f);	
		cam->setUpVector(origUp*q);
		forward = forward*q;
		SrVec tmpCenter = cam->getEye() + forward;
		cam->setCenter(tmpCenter.x, tmpCenter.y, tmpCenter.z);
	}
#endif
}


bool VHEngine::eventButtonTouch( std::string buttonName, int action )
{
	return true;
}

bool VHEngine::eventButtonClick( std::string buttonName )
{
	return true;
}

bool VHEngine::eventVideoCompletion(std::string videoViewName)
{
	return true;
}

bool VHEngine::eventSoundCompletion(std::string videoViewName)
{
	return true;
}


jstring VHEngine::stringToJString( const std::string& str )
{
	int status;
	JNIEnv *env;
	bool isAttached = false;
	jstring resultJString;
	status = jvm->GetEnv((void **) &env, JNI_VERSION_1_4);
	if(status < 0) {
		LOG("callback_handler: failed to get JNI environment, "
			"assuming native thread");
		status = jvm->AttachCurrentThread(&env, NULL);
		if(status < 0) {
			LOG("callback_handler: failed to attach "
				"current thread");
			return resultJString;
		}
		isAttached = true;
	}
	resultJString = env->NewStringUTF(str.c_str());
	if(isAttached) jvm->DetachCurrentThread();

	return resultJString;
}



bool VHEngine::beforeCallJavaMethod( const std::string& className, const std::string& methodName, const std::string& methodDef, jclass& interfaceClass, jmethodID& method)
{
	//va_list args;
	//va_start(args, methodDef);

	
	int status;
	jvmIsAttached = false;

	status = jvm->GetEnv((void **) &env, JNI_VERSION_1_4);
	if(status < 0) {
		LOG("callback_handler: failed to get JNI environment, "
			"assuming native thread");
		status = jvm->AttachCurrentThread(&env, NULL);
		if(status < 0) {
			LOG("callback_handler: failed to attach "
				"current thread");
			return false;
		}
		jvmIsAttached = true;
	}

	interfaceClass = env->FindClass(className.c_str());
	if(!interfaceClass) {
		LOG("callback_handler: failed to get class reference");
		if(jvmIsAttached) jvm->DetachCurrentThread();
		return false;
	}
	/* Find the callBack method ID */
	method = env->GetStaticMethodID(
		interfaceClass, methodName.c_str(), methodDef.c_str());
	if(!method) {
		LOG("callback_handler: failed to get method ID");
		if(jvmIsAttached) jvm->DetachCurrentThread();
		return false;
	}

	//LOG("interfaceClass = %d, method = %d", interfaceClass, method);
	//jstring jname = env->NewStringUTF(widgetName.c_str());
	//env->CallStaticVoidMethod(interfaceClass, method, args);
	//if(jvmIsAttached) jvm->DetachCurrentThread();
	return true;
}

void VHEngine::callJNIMethod(std::string className, std::string methodName, std::string methodDef, std::vector<std::string> parameters)
{
	std::vector<jstring> parametersJStr;
	for (unsigned int i=0;i<parameters.size();i++)
	{
		jstring temp = stringToJString(parameters[i]);
		parametersJStr.push_back(temp);
	}
	int sizeOfParameters = parameters.size();

	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod(className,methodName,methodDef, interfaceClass, methodID);

	if (jvmSuccess)
	{
		if (sizeOfParameters == 0)
			env->CallStaticVoidMethod(interfaceClass, methodID);
		else if (sizeOfParameters == 1)
			env->CallStaticVoidMethod(interfaceClass, methodID, parametersJStr[0]);
		else if (sizeOfParameters == 2)
			env->CallStaticVoidMethod(interfaceClass, methodID, parametersJStr[0], parametersJStr[1]);
		else if (sizeOfParameters == 3)
			env->CallStaticVoidMethod(interfaceClass, methodID, parametersJStr[0], parametersJStr[1], parametersJStr[2]);
		else if (sizeOfParameters == 4)
			env->CallStaticVoidMethod(interfaceClass, methodID, parametersJStr[0], parametersJStr[1], parametersJStr[2], parametersJStr[3]);
		else if (sizeOfParameters >= 5) // only support up to 5 string parameters
			env->CallStaticVoidMethod(interfaceClass, methodID, parametersJStr[0], parametersJStr[1], parametersJStr[2], parametersJStr[3], parametersJStr[4]);
		
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

void VHEngine::playVideo(std::string videoViewName, std::string videoFilePath, bool looping)
{
	jstring videoViewNameJStr = stringToJString(videoViewName);
	jstring videoFilePathJStr = stringToJString(videoFilePath);
	
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","playVideo","(Ljava/lang/String;Ljava/lang/String;Z)V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID, videoViewNameJStr, videoFilePathJStr, looping);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

void VHEngine::stopVideo(std::string videoViewName)
{
	jstring videoViewNameJStr = stringToJString(videoViewName);
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","stopVideo","(Ljava/lang/String;)V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID, videoViewNameJStr);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

void VHEngine::playSound(std::string soundFilePath, bool looping)
{
	jstring soundFilePathJStr = stringToJString(soundFilePath);
	
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","playSound","(Ljava/lang/String;Z)V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID, soundFilePathJStr, looping);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

void VHEngine::stopSound()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","stopSound","()V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}


void VHEngine::createDialogBox( std::string dialogName, std::string dialogTitle, std::string dialogMessage, std::string positiveButton, std::string negativeButton, bool hasTextInput )
{
	jstring dialogNameJStr = stringToJString(dialogName);
	jstring dialogTitleJStr = stringToJString(dialogTitle);
	jstring dialogMessageJStr = stringToJString(dialogMessage);
	jstring positiveButtonStr = stringToJString(positiveButton);
	jstring negativeButtonStr = stringToJString(negativeButton);
	//callJavaVoidMethod("edu/usc/ict/talktome/SBJNIAppActivity","openDialogBox","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", dialogNameJStr, dialogTitleJStr, dialogMessageJStr);

	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","openDialogBox","(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID, dialogNameJStr, dialogTitleJStr, dialogMessageJStr, positiveButtonStr, negativeButtonStr, hasTextInput);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
	else
	{
		LOG("Could not call method: edu/usc/ict/vhmobile/VHMobileMain openDialogBox (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z;Ljava/lang/String;Ljava/lang/String)V");
	}
}


void VHEngine::exitApp()
{
	//callJavaVoidMethod("edu/usc/ict/talktome/SBJNIAppActivity","exitProgram","()V");

	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","exitProgram","()V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

void VHEngine::setWidgetProperty( std::string widgetName, int visible, std::string widgetText )
{
	jstring widgetNameJStr = stringToJString(widgetName);
	jstring widgetTextJStr = stringToJString(widgetText);
	//callJavaVoidMethod("edu/usc/ict/talktome/SBJNIAppActivity","setWidgetProperty","(Ljava/lang/String;ILjava/lang/String;)V", widgetNameJStr, visible, widgetTextJStr);
	//callJavaVoidMethod("edu/usc/ict/talktome/SBJNIAppActivity","setWidgetProperty","(Ljava/lang/String;I)V", widgetNameJStr, visible);
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","setWidgetProperty","(Ljava/lang/String;ILjava/lang/String;)V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID, widgetNameJStr, visible, widgetTextJStr);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}


void VHEngine::enableSensor( std::string sensorName, bool enable )
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","enableSensor","(Ljava/lang/String;Z)V", interfaceClass, methodID);
	jstring sensorNameJStr = stringToJString(sensorName);
	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID, sensorNameJStr, enable);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}


void VHEngine::startVoiceRecognition()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","startVoiceRecognition","()V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

void VHEngine::stopVoiceRecognition()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","stopVoiceRecognition","()V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

bool VHEngine::isVoiceRecognitionListening()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","isVoiceRecognitionListening","()Z", interfaceClass, methodID);

	if (jvmSuccess)
	{
		bool ret = env->CallStaticBooleanMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
		return ret;
	}
	else
	{
		return false;
	}
}

void VHEngine::startVoiceRecording(std::string file)
{
	jstring fileJStr = stringToJString(file);
		
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","startVoiceRecording","(Ljava/lang/String;)V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID, fileJStr);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

void VHEngine::stopVoiceRecording()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","stopVoiceRecording","()V", interfaceClass, methodID);

	if (jvmSuccess)
	{
		env->CallStaticVoidMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
	}
}

bool VHEngine::isVoiceRecording()
{
	jclass interfaceClass;
	jmethodID methodID;
	bool jvmSuccess = beforeCallJavaMethod("edu/usc/ict/vhmobile/VHMobileMain","isVoiceRecording","()Z", interfaceClass, methodID);

	if (jvmSuccess)
	{
		bool ret = env->CallStaticBooleanMethod(interfaceClass, methodID);
		if(jvmIsAttached) jvm->DetachCurrentThread();
		return ret;
	}
	else
	{
		return false;
	}
}


void VHEngine::initTextToSpeechEngine( std::string ttsType )
{
	LOG("initTextToSpeechEngine");
	if (ttsType == "cerevoice")
	{
		std::string cacheDir = "/sdcard/vhdata/cerevoice/cache/";
		std::string cereprocLibDir = "/sdcard/vhdata/cerevoice/voices/";	
	
		// load the TTS voice licenses into the engine
		// star voice
		std::stringstream strstr;
		strstr << "UID=00009\n";
		strstr << "TYP=SDK\n";
		strstr << "VID=SGT\n";
		strstr << "AID=GA\n";
		strstr << "LID=EN\n";
		strstr << "EXP=31-12-2008\n";
		strstr << "SES=5\n";
		std::string licenseText = strstr.str();
		std::string signature = "c9baf1fbe10634c76fd2289ff8b5860f16c36d922921b47f9c768e49e148e895df73adb04fabe0abd482cf6903c008b23c0740ea6898cced5082227381a92e905a67767b4d287d5b07f1e71b4760dfdb08ea671931b308596bdfe7464e4d5f45de7deaecd39ce8429bb8a7318f8bd583b2f35d08db582254a2d37c03728897b1";
		std::string voiceFile = cereprocLibDir + "/cerevoice_star_3.0.0_22k.voice";
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->setLicenseInfo("star", "licenseText", licenseText);
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->setLicenseInfo("star", "signature", signature);
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->setLicenseInfo("star", "voiceFile", voiceFile);

			
		// katherine voice
		std::stringstream strstr2;
		strstr2 << "UID=00009\n";
		strstr2 << "TYP=SDK\n";
		strstr2 << "VID=SMO\n";
		strstr2 << "AID=GA\n";
		strstr2 << "LID=EN\n";
		strstr2 << "EXP=31-12-2008\n";
		strstr2 << "SES=5\n";
		std::string licenseText2 = strstr2.str();
		std::string signature2 ="8e3679800958377c8f3396796dbf7d2289aec8ab43b9456a1d8c86b25af34f1cc2910891143e74048922c81b47ad8d21f01b68735af11db36b426587f4b9baf0015124e95e3f31434a8d795cef590cfa197460a28631bf8f385424276b0de6b17d0e2f50dd1b67213069e89709f3df27ed306f8b75053ac19b0725cbab12f119";
		std::string voiceFile2 = cereprocLibDir + "/cerevoice_katherine_3.0.8_22k.voice";
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->setLicenseInfo("katherine", "licenseText", licenseText2);
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->setLicenseInfo("katherine", "signature", signature2);
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->setLicenseInfo("katherine", "voiceFile", voiceFile2);
		
		// look for any .lic files in the cereprocLibDir and load those files as well
		std::vector<boost::filesystem::path> voiceLicenseFiles;
		boost::filesystem::path voiceFolder(cereprocLibDir);
		if (boost::filesystem::exists(voiceFolder) &&  
		    boost::filesystem::is_directory(voiceFolder))
		{
			boost::filesystem::recursive_directory_iterator it(voiceFolder);
			boost::filesystem::recursive_directory_iterator endit;

			while(it != endit)
			{
				if(boost::filesystem::is_regular_file(*it) && 
				   it->path().extension() == ".lic")
			    {
					LOG("Found license for voice %s", it->path().filename().string().c_str());
					voiceLicenseFiles.push_back(it->path().filename());
			    }
			    ++it;
			}
		}
		
		std::vector<std::string> voices;
		std::vector<std::string> licenses;
		for (std::vector<boost::filesystem::path>::iterator iter = voiceLicenseFiles.begin();
		     iter != voiceLicenseFiles.end();
			 iter++)
		{
			 boost::filesystem::path curFile = (*iter);
			 voices.push_back(curFile.stem().string());
			 licenses.push_back(curFile.filename().string());
		}
		
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->setVoiceAndLicenses(voices, licenses);
		SmartBody::SBScene::getScene()->getSpeechManager()->cereprocRelay()->initSpeechRelay(cereprocLibDir, cacheDir);
			  
		SmartBody::SBScene::getScene()->setStringAttribute("speechRelaySoundCacheDir", cacheDir);
	}
	else
	{
		LOG("Error, currently only support cerevoice TTS engine on mobile platform.");
	}
	LOG("Finish initTextToSpeechEngine");
}

std::string VHEngine::getBehavior(std::string behaviorName, double start, double end)
{
	return defaultGetBehavior(behaviorName, start, end);
}

std::vector<std::string> VHEngine::getBehaviorNames()
{
	return defaultGetBehaviorNames();
}

std::string VHEngine::getNonverbalBehavior(std::string utterance)
{
	return SmartBody::SBScene::getScene()->getParser()->parseUtterance(_parserListener, utterance);
}

std::string VHEngine::eventWord(std::string timing, std::string word, std::string emphasis)
{
	return defaultOnWord(timing, word, emphasis);
}

std::string VHEngine::eventPartOfSpeech(std::string timing, std::string partOfSpeech)
{
	return defaultOnPartOfSpeech(timing, partOfSpeech);
}

SrVec VHEngine::getAccelerometerValues()
{
	return curAccel;
}

SrVec VHEngine::getGyroscopValues()
{
	return curGyroRate;
}


void VHEngine::setBackgroundImage( std::string imageName )
{
	SbmTextureManager& texManager = SbmTextureManager::singleton();
	LOG("setBackgroundImage, imageName = %s", imageName.c_str());
	texManager.loadTexture(SbmTextureManager::TEXTURE_DIFFUSE, "background_img", imageName.c_str());
}



void VHEngine::eventInit()
{

}

void VHEngine::eventInitUI()
{

}


void VHEngine::eventStep()
{

}

void VHEngine::eventVoiceRecognition( bool success, std::string recogText )
{

}

void VHEngine::eventDialogButton( std::string dialogName, int action , std::string textMsg )
{

}

void VHEngine::initClassifier( std::string filename )
{
#if 0
	LOG("Initializing Classifier");
	std::stringstream strstr;
	strstr << "/sdcard/vhdata/" << _dataFolder << "/kstem-lexicon-beta";
	alib::ir::filter::KStem::setLexiconDirectory(strstr.str().c_str());
	alib::npc::cpp::ObjectFactory of;
	LOG("Loading '%s'", filename.c_str());
	std::stringstream strstr2;
	strstr2 << "/sdcard/vhdata/" << filename;
	_model->collection = loadCollection(of, strstr2.str().c_str());
	LOG("After load collection");
	LOG("collection state size = %d", _model->collection.states().size());
	int count = 0;
	BOOST_FOREACH(alib::npc::State& s, _model->collection.states())
	{
		//LOG("State %d",++count);
		LOG("state name = %s",s.name().c_str());
		_model->rankerMap[s.name()] = unigramRanker(_model->collection, s.name().c_str());
	}
#else
	std::stringstream strstr2;
	strstr2 << "/sdcard/vhdata/" << filename;
	std::ifstream file(strstr2.str());
	if (file.good())
		LOG("File stream '%s' is good.", strstr2.str().c_str());
	else
		LOG("File stream '%s' is not good.", strstr2.str().c_str());
		
	if (_model)
		delete _model;
	_model = new Model();
	auto error = _model->p.loadCollection(file);

	if (error)
		LOG("error is %s", (*error).what());
	//LOG("Start Training Classifier");
	//auto trainError = _model->p.train("Anyone - The Twins");

	LOG("Classifier Init Done");

#endif	
}

std::string VHEngine::classify( std::string characterName, std::string question )
{
#if 0
	alib::npc::cpp::ObjectFactory of;
	alib::npc::Utterance query = of.newUtterance();
	alib::npc::Ranker::result_type rl;
	query.put(alib::npc::Utterance::kFieldText, question.c_str());

	// find the appropriate ranker
	std::map<std::string, alib::npc::Ranker>::iterator iter = _model->rankerMap.find(characterName);
	if (iter == _model->rankerMap.end())
	{
		LOG("Could not find character named %s", characterName.c_str());
		return "";
	}

	rl = ((*iter).second)(query);

	int i = 0;
	double highScore = -1e30;
	std::string answer = "";
	BOOST_FOREACH(const alib::npc::Ranker::ranked_list_type::value_type& s, rl)
	{
		//std::cout << ++i << " " << s.score() << " " << (*s.object()).get(Utterance::kFieldID) << std::endl;
		double score = s.score();
		const alib::npc::Utterance& testUtterance = s.object();
		std::string text = testUtterance.get("text");
		std::string externalID = testUtterance.get("id");
		if (score > highScore)
		{
			highScore = score;
			answer = text;
		}

		//LOG("%d %f text size = %u\n",++i, score, text.size());
		//LOG("extID = %s", externalID.c_str());
	}
#else
	std::string answer = "";
	boost::optional<alib::npc::Utterance> response = _model->p.respond(characterName, question);
	if (response)
	{
		answer = (*response).get("text");
	}
#endif
	return answer;
}

std::vector<std::string> VHEngine::classifyWithExternalID(std::string characterName, std::string question)
{
	std::vector<std::string> ret;
	std::string answer = "None";
	std::string id = "None";
	std::string type = "None";
	boost::optional<alib::npc::Utterance> response = _model->p.respond(characterName, question);
	if (response)
	{
		answer = (*response).get("text");
		//LOG("classifyWithExternalID, answer = %s", answer.c_str());
		id = (*response).get("id");
		//LOG("classifyWithExternalID, externalID = %s", id.c_str());
		
		if ( (*response).has("type"))
			type = (*response).get("type");
			
	}
	ret.push_back(answer);
	ret.push_back(id);
	return ret;
}

std::vector<std::string> VHEngine::classifyMultipleWithExternalID(std::string characterName, std::string question)
{
	std::vector<std::string> ret;
	ret.push_back("not_supported_now");
	#if 0
	std::string answer = "None";
	std::string id = "None";
	std::string type = "None";
	std::vector<alib::npc::Utterance> response = _model->p.respondMultiple(characterName, question);
	for (size_t u = 0; u < response.size(); u++)
	{
		answer = response[u].get("text");
		id = response[u].get("id");	
		ret.push_back(answer);
		ret.push_back(id);
	}
	#endif
	
	return ret;
}

void VHEngine::addQuestionAnswer(std::string stateName, std::string question, std::string answer)
{
	if (!_model)
		_model = new Model();
	
	alib::npc::Collection& collection = _model->p.getCollection();
	alib::npc::cpp::ObjectFactory of;
	
	auto optState = collection.states().optionalObjectWithName(stateName);
	if (!optState)
	{
		// state can't be found
		LOG("State '%s' can not be found!", stateName.c_str());
		return;
	}
	alib::npc::Domain questionDomain = (*optState).questions();
	alib::npc::Domain answerDomain = (*optState).answers();
	alib::npc::Lattice linkLattice = (*optState).links();
	
	// create and insert question/answer utterances to collection
	alib::npc::Utterance questionUtterance = of.newUtterance();
	alib::npc::Utterance answerUtterance = of.newUtterance();
	questionUtterance.put(alib::npc::Utterance::kFieldText, question);
	answerUtterance.put(alib::npc::Utterance::kFieldText, answer);
	collection.utterances().insert(questionUtterance);
	collection.utterances().insert(answerUtterance);
	
	// create the link that connect question and answer`
	alib::npc::Link link = of.newLink();
	link.setValue(6); // need to set to 6 by default
	link.utterances().insert(questionUtterance);
	link.utterances().insert(answerUtterance);
	collection.links().insert(link);
	// insert question and answer to their corresponding Domains
	questionDomain.utterances().insert(questionUtterance);
	answerDomain.utterances().insert(answerUtterance);
	
	// also insert the new link to corresponding lattice
	linkLattice.links().insert(link);
}

void VHEngine::updateClassifier(std::string tempFilename)
{
	if (!_model)
	{
		LOG("Classifier does not exist. Do 'initClassifier' first.");
		return;
	}
	
	std::stringstream strstr2;
	strstr2 << "/sdcard/vhdata/" << tempFilename;
	std::ofstream file(strstr2.str());
	if (file.good())
		LOG("File stream '%s' is good.", strstr2.str().c_str());
	else
		LOG("File stream '%s' is not good.", strstr2.str().c_str());
	// do a save then load for a temporary file to rebuild classifier
	//_model->p.train("Anyone - The Twins");
	_model->p.saveCollection(file);
	initClassifier(tempFilename);
}

bool VHEngine::isConnected()
{
	SmartBody::SBVHMsgManager* vhmsgManager = SmartBody::SBScene::getScene()->getVHMsgManager();
	return vhmsgManager->isConnected();
}

bool VHEngine::connect(std::string host)
{
	SmartBody::SBVHMsgManager* vhmsgManager = SmartBody::SBScene::getScene()->getVHMsgManager();
	if (vhmsgManager->isConnected())
	{
		vhmsgManager->disconnect();
	}
	vhmsgManager->setServer(host);
	bool isConnected = vhmsgManager->connect();
	if (isConnected)
	{
		vhmsgManager->setEnable(true);
	}
	return isConnected;
}

void VHEngine::disconnect()
{
	SmartBody::SBVHMsgManager* vhmsgManager = SmartBody::SBScene::getScene()->getVHMsgManager();
	if (vhmsgManager->isConnected())
	{
		vhmsgManager->disconnect();
	}
}

void VHEngine::send(std::string message)
{
	SmartBody::SBVHMsgManager* vhmsgManager = SmartBody::SBScene::getScene()->getVHMsgManager();
	if (vhmsgManager->isConnected())
	{
		int ret = vhmsgManager->send(message.c_str());
	}
	else
	{
		LOG("Device is not connected, cannot send message '%s'", message.c_str());
	}
}

void VHEngine::eventMessage(std::string message)
{
	
}










