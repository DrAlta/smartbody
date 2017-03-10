#ifndef __VH_ANDROID_ENGINE__
#define __VH_ANDROID_ENGINE__
#include <jni.h>
#include <vhcl_log.h>
#include <vhcl_string.h>
#include <string>
#include <sr/sr_vec.h>

class Model;
namespace SmartBody {
	class  SBParserListener;
};


class VHEngine
{
public:
	VHEngine();
	~VHEngine();

	// version information
	std::string getVersion();
	
	virtual void eventInit();
	virtual void eventInitUI();
	virtual void eventStep();
	void exitApp();
	
	void setDataFolder(std::string folder);
	std::string getDataFolder();
	
	// capability query
	bool isWifiOn();
	bool isMobileOn();
	bool isLocationOn();

	// voice recognition API
	void startVoiceRecognition();
	void stopVoiceRecognition();	
	bool isVoiceRecognitionListening();

	// voice recording API
	void startVoiceRecording(std::string file);
	void stopVoiceRecording();	
	bool isVoiceRecording();
	
	// voice API events
	virtual void eventVoiceRecognition(bool success, std::string recogText);
	
	// Interaction API
	void createDialogBox(std::string dialogName, std::string dialogTitle, std::string dialogMessage, std::string positiveMessage, std::string negativeMessage, bool hasTextInput);
	void setWidgetProperty(std::string widgetName, int visible, std::string widgetText);
	// Interaction API events
	virtual void eventDialogButton(std::string dialogName, int action, std::string textMsg);
	virtual bool eventScreenTouch(int action, float x, float y);
	virtual bool eventButtonTouch(std::string buttonName, int action);
	virtual bool eventButtonClick(std::string buttonName);
	
	// Video API
	void playVideo(std::string videoViewName, std::string videoFilePath, bool looping = false);
	void stopVideo(std::string videoViewName);
	virtual bool eventVideoCompletion(std::string videoViewName);

	// Sound API
	void playSound(std::string soundFilePath, bool looping = false);
	void stopSound();
	virtual bool eventSoundCompletion(std::string videoViewName);

	void callJNIMethod(std::string className, std::string methodName, std::string methodDef, std::vector<std::string> parameters);

	// Sensor API
	void enableSensor(std::string sensorName, bool enable);
	SrVec getAccelerometerValues();
	SrVec getGyroscopValues();
	
	// Dialogue API
	virtual void initClassifier(std::string filename);
	virtual std::string classify(std::string characterName, std::string question);
	virtual std::vector<std::string> classifyWithExternalID(std::string characterName, std::string question);
	virtual std::vector<std::string> classifyMultipleWithExternalID(std::string characterName, std::string question);
	virtual void addQuestionAnswer(std::string stateName, std::string question, std::string answer);
	virtual void updateClassifier(std::string tempFilename = "temp.plist.csxml");
	
	// Network API
	virtual bool isConnected();
	virtual bool connect(std::string host);
	virtual void disconnect();
	virtual void send(std::string message);
	// Network API Events
	virtual void eventMessage(std::string message);
	
	// TTS
	void initTextToSpeechEngine(std::string ttsType);
	// Utterance API
	std::string getNonverbalBehavior(std::string utterance);
	std::string getBehavior(std::string behaviorName, double start, double end);
	std::vector<std::string> getBehaviorNames();
	virtual std::string eventWord(std::string timing, std::string word, std::string emphasis);
	virtual std::string eventPartOfSpeech(std::string timing, std::string partOfSpeech);	
	
	// Display API
	void setBackgroundImage(std::string imageName);
	
	bool beforeCallJavaMethod( const std::string& className, const std::string& methodName, const std::string& methodDef, jclass& interfaceClass, jmethodID& method);
	
	static VHEngine* getEngine() { return engine; }
	static void setEngine(VHEngine* inEngine) { engine = inEngine; }
	
	vhcl::Log::AndroidListener androidListener;
	long prevTime;
	bool engineInit;
	SrVec curAccel;
	SrVec curGyroRate;
	
	std::string _dataFolder;
	Model* _model;
	
	
	static JavaVM* jvm;
	static JNIEnv* env;
	static bool jvmIsAttached;
	static VHEngine* engine;
	static int curH, curW;
	
protected:
	jstring stringToJString(const std::string& str);
	SmartBody::SBParserListener* _parserListener;
};



#endif
