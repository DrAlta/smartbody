#include "vhcl.h"
#include "VHEngine.h"
#include "VHPython.h"
#include <vector>
#include <string>
#include "NonverbalBehavior.h"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#include <boost/python.hpp>

struct VHEngineWrap :  VHEngine, boost::python::wrapper<VHEngine>
{
	// Simulation control API
	virtual void eventInit();
	virtual void eventInitUI();
	virtual void eventStep();
	
	// UI Events
	virtual bool eventScreenTouch(int action, float x, float y);
	virtual bool eventButtonTouch(std::string buttonName, int action);
	virtual bool eventButtonClick(std::string buttonName);
	virtual bool eventVideoCompletion(std::string videoViewName);
	virtual bool eventSoundCompletion(std::string videoViewName);
	virtual void eventDialogButton(std::string dialogName, int action, std::string textMsg);
	
	// Virtual Human Events
	virtual void eventVoiceRecognition(bool success, std::string recogText);
	
	// Messaging API
	virtual void eventMessage(std::string message);
	
	// Utterance API
	virtual std::string eventWord(std::string timing, std::string word, std::string emphasis);
	virtual std::string eventPartOfSpeech(std::string timing, std::string partOfSpeech);

	void default_eventInit() { VHEngine::eventInit();}
	void default_eventInitUI() { VHEngine::eventInitUI();}
	void default_eventStep() { VHEngine::eventStep(); }
	bool default_eventScreenTouch(int action, float x, float y)  { return VHEngine::eventScreenTouch(action, x, y); }
	bool default_eventButtonTouch(std::string buttonName, int action) { return VHEngine::eventButtonTouch(buttonName, action); }
	bool default_eventButtonClick(std::string buttonName) { return VHEngine::eventButtonClick(buttonName); }
	bool default_eventVideoCompletion(std::string videoViewName) { return VHEngine::eventVideoCompletion(videoViewName); }
	bool default_eventSoundCompletion(std::string videoViewName) { return VHEngine::eventSoundCompletion(videoViewName); }
	void default_eventVoiceRecognition(std::string recogText) { VHEngine::eventVoiceRecognition(true, recogText); }
	void default_eventDialogButton(std::string dialogName, int action, std::string textMsg) { VHEngine::eventDialogButton(dialogName, action, textMsg); }
	void default_eventMessage(std::string message) { VHEngine::eventMessage(message); }
	std::string default_eventWord(std::string timing, std::string word, std::string emphasis) { return VHEngine::eventWord(timing, word, emphasis); }
	std::string default_eventPartOfSpeech(std::string timing, std::string partOfSpeech) { return VHEngine::eventPartOfSpeech(timing, partOfSpeech); }
	
};

void setVHEngine(VHEngine* inEngine) { VHEngine::setEngine(inEngine); }

/************************************************************************/
/* VHEngineWrap                                                  */
/************************************************************************/
void VHEngineWrap::eventInit()
{
	if (boost::python::override o = this->get_override("eventInit"))
	{
		try {
			o();
		} catch (...) {
			PyErr_Print();
		}
	}
	VHEngine::eventInit();
}

void VHEngineWrap::eventInitUI()
{
	if (boost::python::override o = this->get_override("eventInitUI"))
	{
		try {
			o();
		} catch (...) {
			PyErr_Print();
		}
	}
	VHEngine::eventInitUI();
}


void VHEngineWrap::eventStep()
{
	if (boost::python::override o = this->get_override("eventStep"))
	{
		try {
			o();
		} catch (...) {
			PyErr_Print();
		}
	}
	VHEngine::eventStep();
}

bool VHEngineWrap::eventScreenTouch( int action, float x, float y )
{
	if (boost::python::override o = this->get_override("eventScreenTouch"))
	{
		try {
			o(action,x,y);
		} catch (...) {
			PyErr_Print();
		}
	}
	return VHEngine::eventScreenTouch(action,x,y);
}

bool VHEngineWrap::eventButtonTouch( std::string buttonName, int action )
{
	if (boost::python::override o = this->get_override("eventButtonTouch"))
	{
		try {
			return o(buttonName, action);
		} catch (...) {
			PyErr_Print();
		}
	}
	return VHEngine::eventButtonTouch(buttonName, action);
}

bool VHEngineWrap::eventButtonClick( std::string buttonName )
{
	if (boost::python::override o = this->get_override("eventButtonClick"))
	{
		try {
			return o(buttonName);
		} catch (...) {
			PyErr_Print();
		}
	}
	return VHEngine::eventButtonClick(buttonName);
}

bool VHEngineWrap::eventVideoCompletion( std::string videoViewName )
{
	if (boost::python::override o = this->get_override("eventVideoCompletion"))
	{
		try {
			return o(videoViewName);
		} catch (...) {
			PyErr_Print();
		}
	}
	return VHEngine::eventVideoCompletion(videoViewName);
}

bool VHEngineWrap::eventSoundCompletion( std::string videoViewName )
{
	if (boost::python::override o = this->get_override("eventSoundCompletion"))
	{
		try {
			return o(videoViewName);
		} catch (...) {
			PyErr_Print();
		}
	}
	return VHEngine::eventSoundCompletion(videoViewName);
}

void VHEngineWrap::eventVoiceRecognition( bool success, std::string recogText )
{
	if (boost::python::override o = this->get_override("eventVoiceRecognition"))
	{
		try {
			o(success, recogText);
		} catch (...) {
			PyErr_Print();
		}
	}
	VHEngine::eventVoiceRecognition(success, recogText);
}

void VHEngineWrap::eventDialogButton( std::string dialogName, int action, std::string textMsg )
{
	if (boost::python::override o = this->get_override("eventDialogButton"))
	{
		try {
			o(dialogName,action, textMsg);
		} catch (...) {
			PyErr_Print();
		}
	}
}

void VHEngineWrap::eventMessage( std::string message )
{
	if (boost::python::override o = this->get_override("eventMessage"))
	{
		try {
			o(message);
		} catch (...) {
			PyErr_Print();
		}
	}
	VHEngine::eventMessage(message);
}

std::string VHEngineWrap::eventWord( std::string timing, std::string word, std::string emphasis)
{
	if (boost::python::override o = this->get_override("eventWord"))
	{
		try {
			return o(timing, word, emphasis);
		} catch (...) {
			PyErr_Print();
			return "";
		}
	}
	return VHEngine::eventWord(timing, word, emphasis);
}

std::string VHEngineWrap::eventPartOfSpeech( std::string timing, std::string partOfSpeech )
{
	if (boost::python::override o = this->get_override("eventPartOfSpeech"))
	{
		try {
			return o(timing, partOfSpeech);
		} catch (...) {
			PyErr_Print();
			return "";
		}
	}
	return VHEngine::eventPartOfSpeech(timing, partOfSpeech);
}



BOOST_PYTHON_MODULE(VH)
{	
	boost::python::def("setVHEngine", setVHEngine, "Set Engine");
	boost::python::class_<VHEngineWrap, boost::noncopyable>("VHEngine")
		//.staticmethod("setEngine", &VHEngineWrap::setEngine, "Set the VHEngine singleton")
		.def("getVersion", &VHEngine::getVersion, "Gets version information.")

		// Simulation API
		.def("eventInit", &VHEngine::eventInit, &VHEngineWrap::default_eventInit, "Init the android engine.")
		.def("eventInitUI", &VHEngine::eventInitUI, &VHEngineWrap::default_eventInitUI, "Init the widgets and user interface elements.")
		.def("eventStep", &VHEngine::eventStep, &VHEngineWrap::default_eventStep, "Init the android engine.")
		.def("exitApp", &VHEngine::exitApp, "Exit the app.")
		
		// Voice TTS, Recognition and Recording API
		.def("initTextToSpeechEngine", &VHEngine::initTextToSpeechEngine, "Init TTS engine.")
		.def("startVoiceRecognition", &VHEngine::startVoiceRecognition, "Starts the voice recognition process.")
		.def("stopVoiceRecognition", &VHEngine::stopVoiceRecognition, "Stop the voice recognition process.")
		.def("isVoiceRecognitionListening", &VHEngine::isVoiceRecognitionListening, "Checks to see if the voice recoognition is already listening.")
		.def("startVoiceRecording", &VHEngine::startVoiceRecording, "Starts the voice recording process.")
		.def("stopVoiceRecording", &VHEngine::stopVoiceRecording, "Stops the voice recording process.")
		.def("isVoiceRecording", &VHEngine::isVoiceRecording, "Checks to see if the voice recording is in progress.")

		// Voice API Events
		.def("eventVoiceRecognition", &VHEngine::eventVoiceRecognition, &VHEngineWrap::eventVoiceRecognition, "Init the android engine.")

		
		// Interaction API
		.def("createDialogBox", &VHEngine::createDialogBox, "Creates a dialog box.")
		.def("setWidgetProperty", &VHEngine::setWidgetProperty, "Sets properties of the widget such as name and activation.")
		.def("eventDialogButton", &VHEngine::eventDialogButton, &VHEngineWrap::default_eventDialogButton, "Event called when a dialog button is pressed.")
		.def("eventScreenTouch", &VHEngine::eventScreenTouch, &VHEngineWrap::default_eventScreenTouch, "Event called when the screen is touched.")
		.def("eventButtonTouch", &VHEngine::eventButtonTouch, &VHEngineWrap::default_eventButtonTouch, "Event called when the button is touched.")
		.def("eventButtonClick", &VHEngine::eventButtonClick, &VHEngineWrap::default_eventButtonClick, "Event called when the button is clicked.")
		.def("eventVideoCompletion", &VHEngine::eventVideoCompletion, &VHEngineWrap::default_eventVideoCompletion, "Event called when the video playback is completed.")
		.def("eventSoundCompletion", &VHEngine::eventSoundCompletion, &VHEngineWrap::default_eventSoundCompletion, "Event called when the sound playback is completed.")
		// Video API
		.def("playVideo", &VHEngine::playVideo, "Playback a video file from a specific video view.")
		.def("stopVideo", &VHEngine::stopVideo, "Stop the video playback.")
		
		// Sound API
		.def("playSound", &VHEngine::playSound, "Playback a sound file.")
		.def("stopSound", &VHEngine::stopSound, "Stop the sound playback.")
		
		// Sensor API
		.def("enableSensor", &VHEngine::enableSensor, "Enable a particular sensor to listen to its values.")
		.def("getAccelerometerValues", &VHEngine::getAccelerometerValues, "Enable a particular sensor to listen to its values.")
		.def("getGyroscopValues", &VHEngine::getGyroscopValues, "Enable a particular sensor to listen to its values.")

		// Dialogue API
		.def("initClassifier", &VHEngine::initClassifier, "Intializes the classifier.")
		.def("classify", &VHEngine::classify, "Returns a set of valid responses given a question.")
		.def("classifyWithExternalID", &VHEngine::classifyWithExternalID, "Returns a set of valid responses given a question.")
		.def("classifyMultipleWithExternalID", &VHEngine::classifyMultipleWithExternalID, "Returns a set of multiple valid responses given a question.")
		.def("addQuestionAnswer", &VHEngine::addQuestionAnswer, "Add a new set of question/answer into classifier.")
		.def("updateClassifier", &VHEngine::updateClassifier, "Update the classifier with new questions/answers.")
		
		// Utterance API
		.def("getNonverbalBehavior", &VHEngine::getNonverbalBehavior, boost::python::return_value_policy<boost::python::return_by_value>(), "Callback when a word is processed.")
		.def("getBehaviorNames", &VHEngine::getBehaviorNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns all behaviors that could be invoked.")
		.def("getBehavior", &VHEngine::getBehavior, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns BML for a given behavior and start/end times.")
		.def("eventWord", &VHEngine::eventWord, "Callback when a word is processed.")
		.def("eventPartOfSpeech", &VHEngine::eventPartOfSpeech, "Callback when a word is processed.")
		
		// Network API
		.def("isConnected", &VHEngine::isConnected, "Determine if the device is connected to the messaging server.")	
		.def("connect", &VHEngine::connect, "Connect the device to the messaging server.")	
		.def("disconnect", &VHEngine::disconnect, "Disonnect the device from the messaging server.")	
		.def("send", &VHEngine::send, "Send message to the messaging server.")	

		// Display API
		.def("setBackgroundImage", &VHEngine::setBackgroundImage, "Set the background image in the scene.")
		
		// JNI API
		.def("callJNIMethod", &VHEngine::callJNIMethod, "Called a JNI method based on the provided class name, method name, method definition, and a set of string function parameters")	
		;
		
		boost::python::def("defaultOnWord", defaultOnWord, "Default nonverbal behavior per-word.");
		boost::python::def("defaultOnPartOfSpeech", defaultOnPartOfSpeech, "Default nonverbal behavior per-part of speech.");

}

void initVHPythonModule()
{
	initVH();
}


